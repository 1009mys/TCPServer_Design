#pragma once
#include <thread>
#include <map>
#include <atomic>
#include <netinet/in.h>
#include <unistd.h>
#include "json.hpp"
#include "ThreadSafeQueue.h"
#include "Message.h"
#include "Dispatcher.h"
#include "RequestHandler.h"

class TcpServer
{
public:
    explicit TcpServer(int port);
    ~TcpServer();

    void start();
    void stop();

    void sendToClient(int client_id, const nlohmann::json &json);

    void addHandler(const std::string &type, Dispatcher::Handler handler)
    {
        dispatcher_.registerHandler(type, std::move(handler));
    }

private:
    void acceptLoop();
    void recvLoop(int thread_index);
    void sendLoop();
    void processLoop();

    static bool recvAll(int fd, void *buf, size_t len);
    static bool sendAll(int fd, const void *buf, size_t len);

    int server_fd_;
    int port_;
    std::atomic<bool> running_{false};

    std::thread accept_thread_;
    int recv_thread_count_ = 4;
    std::vector<std::thread> recv_threads_;
    std::thread send_thread_;
    std::thread process_thread_;

    std::mutex client_mutex_;
    std::map<int, int> clients_; // client_id -> socket_fd
    int next_client_id_ = 1;
    
    // 소켓 할당: socket_fd -> assigned_thread_index (경쟁 상태 방지)
    std::mutex socket_assignment_mutex_;
    std::map<int, int> socket_assignments_;

    Dispatcher dispatcher_;

    ThreadSafeQueue<Message> recv_queue_;
    ThreadSafeQueue<Message> send_queue_;
};
