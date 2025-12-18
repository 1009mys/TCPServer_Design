#include <iostream>

#include "TcpServer.h"

using namespace std;

bool TcpServer::recvAll(int fd, void *buf, size_t len)
{
    char *p = static_cast<char *>(buf);
    size_t got = 0;
    while (got < len)
    {
        ssize_t r = ::recv(fd, p + got, len - got, 0);
        if (r == 0)
            return false; // peer closed
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        got += static_cast<size_t>(r);
    }
    return true;
}

bool TcpServer::sendAll(int fd, const void *buf, size_t len)
{
    const char *p = static_cast<const char *>(buf);
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t s = ::send(fd, p + sent, len - sent, 0);
        if (s <= 0)
        {
            if (s < 0 && errno == EINTR)
                continue;
            return false;
        }
        sent += static_cast<size_t>(s);
    }
    return true;
}

// TcpServer::TcpServer(int port) : port_(port) {}

TcpServer::TcpServer(int port) : port_(port)
{
}

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::start()
{
    std::cout.setf(std::ios::unitbuf);

    server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        perror("socket");
        return;
    }

    int opt = 1;
    ::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (::bind(server_fd_, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        ::close(server_fd_);
        return;
    }
    if (::listen(server_fd_, 10) < 0)
    {
        perror("listen");
        ::close(server_fd_);
        return;
    }

    running_ = true;

    accept_thread_ = std::thread(&TcpServer::acceptLoop, this);
    for (int i = 0; i < recv_thread_count_; ++i)
    {
        recv_threads_.emplace_back(&TcpServer::recvLoop, this, i);
    }
    send_thread_ = std::thread(&TcpServer::sendLoop, this);
    process_thread_ = std::thread(&TcpServer::processLoop, this);

#ifdef DEBUG_BUILD
    std::cout << "[TcpServer] Server started on port " << port_ << std::endl;
#endif
}

void TcpServer::stop()
{
    running_ = false;
    close(server_fd_);

    // 큐 종료 신호 전송
    recv_queue_.shutdown();
    send_queue_.shutdown();

    if (accept_thread_.joinable())
        accept_thread_.join();
    for (auto &t : recv_threads_)
    {
        if (t.joinable())
            t.join();
    }
    if (send_thread_.joinable())
        send_thread_.join();
    if (process_thread_.joinable())
        process_thread_.join();
}

void TcpServer::acceptLoop()
{
    while (running_)
    {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);

        int client_fd = accept(server_fd_, (sockaddr *)&client_addr, &len);
        if (client_fd < 0)
            continue;

#ifdef DEBUG_BUILD
        cout << "[TcpServer] New client connected: fd=" << client_fd << "\n";
#endif

        int client_id;
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            client_id = next_client_id_++;
            clients_[client_id] = client_fd;
        }
        
        // 라운드 로빈으로 스레드에 할당
        {
            std::lock_guard<std::mutex> lock(socket_assignment_mutex_);
            socket_assignments_[client_fd] = client_id % recv_thread_count_;
        }
    }
}

void TcpServer::recvLoop(int thread_index)
{
    while (running_)
    {
        // 1) 이 스레드에 할당된 클라이언트만 가져오기
        std::vector<std::pair<int, int>> snapshot;
        snapshot.reserve(64);

        {
            std::lock_guard<std::mutex> lock1(client_mutex_);
            std::lock_guard<std::mutex> lock2(socket_assignment_mutex_);
            for (auto &[cid, fd] : clients_)
            {
                auto it = socket_assignments_.find(fd);
                if (it != socket_assignments_.end() && it->second == thread_index)
                {
                    snapshot.push_back({cid, fd});
                }
            }
        }

        if (snapshot.empty())
        {
            // 연결된 클라이언트 없으면 살짝 쉬기
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // 2) select 준비
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;
        for (auto &[cid, fd] : snapshot)
        {
            FD_SET(fd, &rfds);
            if (fd > maxfd)
                maxfd = fd;
        }

        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000; // 100ms

        int ready = ::select(maxfd + 1, &rfds, nullptr, nullptr, &tv);
        if (ready <= 0)
            continue; // timeout or error

        // 3) 읽을 수 있는 fd만 처리
        for (auto &[client_id, fd] : snapshot)
        {
            if (!FD_ISSET(fd, &rfds))
                continue;

#ifdef DEBUG_BUILD
            cout << "[TcpServer] Data available from client_id=" << client_id << " fd=" << fd << "\n";
#endif

            uint32_t len_net = 0;
            if (!recvAll(fd, &len_net, sizeof(len_net)))
            {
#ifdef DEBUG_BUILD
                std::cout << "[TcpServer] Client disconnected (len) client_id=" << client_id << "\n";
#endif
                int fd_to_close;
                {
                    std::lock_guard<std::mutex> lock(client_mutex_);
                    auto it = clients_.find(client_id);
                    if (it != clients_.end())
                    {
                        fd_to_close = it->second;
                        ::close(it->second);
                        clients_.erase(it);
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(socket_assignment_mutex_);
                    socket_assignments_.erase(fd_to_close);
                }
                continue;
            }

            uint32_t len = ntohl(len_net);
            if (len == 0 || len > 4 * 1024 * 1024)
            { // 방어(4MB 제한 예시)
#ifdef DEBUG_BUILD
                std::cout << "[TcpServer] Invalid length=" << len << " client_id=" << client_id << "\n";
#endif
                int fd_to_close;
                {
                    std::lock_guard<std::mutex> lock(client_mutex_);
                    auto it = clients_.find(client_id);
                    if (it != clients_.end())
                    {
                        fd_to_close = it->second;
                        ::close(it->second);
                        clients_.erase(it);
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(socket_assignment_mutex_);
                    socket_assignments_.erase(fd_to_close);
                }
                continue;
            }

            std::string payload(len, '\0');
            if (!recvAll(fd, payload.data(), len))
            {
#ifdef DEBUG_BUILD
                std::cout << "[TcpServer] Client disconnected (payload) client_id=" << client_id << "\n";
#endif
                int fd_to_close;
                {
                    std::lock_guard<std::mutex> lock(client_mutex_);
                    auto it = clients_.find(client_id);
                    if (it != clients_.end())
                    {
                        fd_to_close = it->second;
                        ::close(it->second);
                        clients_.erase(it);
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(socket_assignment_mutex_);
                    socket_assignments_.erase(fd_to_close);
                }
                continue;
            }

            // 예외 없이 파싱
            auto j = nlohmann::json::parse(payload, nullptr, false);
            if (j.is_discarded())
            {
#ifdef DEBUG_BUILD
                std::cout << "[TcpServer] Invalid JSON from client_id=" << client_id << "\n";
#endif
                // 에러 응답 보내도 되고, 무시해도 됨
                nlohmann::json err =
                    {
                        {"type", "error"},
                        {"ok", false},
                        {"reason", "invalid_json"}};
                send_queue_.push({client_id, err});
                continue;
            }
#ifdef DEBUG_BUILD
            std::cout << "[TcpServer] Received message from client " << client_id << "\n";
#endif
            recv_queue_.push({client_id, j});
        }
    }
}

void TcpServer::sendLoop()
{
    while (running_)
    {
        auto msg_opt = send_queue_.pop();
        if (!msg_opt.has_value())
            break; // shutdown

        Message msg = msg_opt.value();

        std::lock_guard<std::mutex> lock(client_mutex_);
        auto it = clients_.find(msg.client_id);
        if (it == clients_.end())
            continue;

#ifdef DEBUG_BUILD
        cout << "[TcpServer] Sending message to client " << msg.client_id << "\n";
#endif
        int fd = it->second;
        std::string data = msg.json.dump();

        uint32_t len = htonl(data.size());
        if (!sendAll(fd, &len, sizeof(len)) || !sendAll(fd, data.data(), data.size()))
        {
#ifdef DEBUG_BUILD
            std::cout << "[TcpServer] Failed to send to client " << msg.client_id << ", closing\n";
#endif
            ::close(fd);
            clients_.erase(it);
        }
    }
}

void TcpServer::sendToClient(int client_id, const nlohmann::json &json)
{
    send_queue_.push({client_id, json});
}

void TcpServer::processLoop()
{
    while (running_)
    {
        auto msg_opt = recv_queue_.pop();
        if (!msg_opt.has_value())
            break; // shutdown

        Message msg = msg_opt.value();

        // 종료용
        if (msg.json.contains("type") && msg.json["type"] == "_quit")
            continue;
#ifdef DEBUG_BUILD
        cout << "[TcpServer] Processing message from client " << msg.client_id << "\n";
#endif
        // 디스패치
        nlohmann::json response = dispatcher_.dispatch(msg);

        // 응답 송신 큐로
        send_queue_.push({msg.client_id, response});
    }
}
