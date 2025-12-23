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
#include "ExampleMessageHandler.h"
#include "Logger.h"

 namespace msgnet 
 { 

struct TcpServerConfig
{
    int server_fd;
    int port;
    int accept_thread_count;
    int recv_thread_count;
    int process_thread_count;
    int send_thread_count;
    LogLevel log_level;

};

enum class ExceptionType
    {
        SOCKET_CREATION_FAILED,
        BIND_FAILED,
        LISTEN_FAILED,
        DISCONNECT,
        INVALID_LENGTH,
    };

class TcpServer
{
public:
    
    
    explicit TcpServer(int port);
    ~TcpServer();

    void start();
    void stop();

    void sendToClient(int client_id, const nlohmann::json &json);

    void addMessageHandler(const std::string &type, Dispatcher::MessageHandler handler)
    {
        dispatcher_.registerMessageHandler(type, std::move(handler));
    }

    void setAcceptThreadCount(int count)
    {
        accept_thread_count_ = count;
    }

    void setProcessThreadCount(int count)
    {
        process_thread_count_ = count;
    }

    void setRecvThreadCount(int count)
    {
        recv_thread_count_ = count;
    }

    void setLogLevel(LogLevel level)
    {
        Logger::instance().setLevel(level);
    }

    void setStartProbe(std::function<void(const TcpServerConfig &)> probe)
    {
        start_probe_ = std::move(probe);
    }

    void setStopProbe(std::function<void(const TcpServerConfig &)> probe)
    {
        stop_probe_ = std::move(probe);
    }

    void setExceptionProbe(std::function<void(const int client_id, const int socket_fd, ExceptionType et)> probe)
    {
        exception_probe_ = std::move(probe);
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

    int accept_thread_count_ = 1;
    int recv_thread_count_ = 4;
    int process_thread_count_ = 4;
    int send_thread_count_ = 1;

    std::vector<std::thread> accept_threads_;
    std::vector<std::thread> recv_threads_;
    std::vector<std::thread> process_threads_;
    std::vector<std::thread> send_threads_;

    std::mutex client_mutex_;
    std::map<int, int> clients_; // client_id -> socket_fd
    int next_client_id_ = 1;

    // 소켓 할당: socket_fd -> assigned_thread_index (경쟁 상태 방지)
    std::mutex socket_assignment_mutex_;
    std::map<int, int> socket_assignments_;

    Dispatcher dispatcher_;

    ThreadSafeQueue<Message> recv_queue_;
    ThreadSafeQueue<Message> send_queue_;

    std::function<void(const TcpServerConfig &)> start_probe_ = nullptr;
    std::function<void(const TcpServerConfig &)> stop_probe_ = nullptr;
    std::function<void(const int client_id, const int socket_fd, ExceptionType et)> exception_probe_ = nullptr;
};

} // namespace msgnet