// main.cpp
#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

#include "TcpServer.h" 
#include "json.hpp"
#include "Logger.h"

using namespace std;
using namespace std::chrono;
using namespace msgnet;

static std::atomic<bool> g_run{true};

static void onSignal(int)
{
    g_run = false;
}

int initServer(int argc, char **argv)
{
    // 포트 입력 (기본 55000)
    int port = 55000;
    if (argc >= 2)
    {
        port = std::stoi(argv[1]);
    }

    // SIGINT(Ctrl+C), SIGTERM 처리
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    try
    {
        // 서버 객체 생성
        msgnet::TcpServer server(port);

        // 로그 레벨 설정 (DEBUG, INFO, WARN, ERROR)
        server.setLogLevel(msgnet::LogLevel::DEBUG);

        server.setRecvThreadCount(1);

        msgnet::ExampleMessageHandler handler;
        server.addMessageHandler("ping", msgnet::ExampleMessageHandler::handlePing);
        server.addMessageHandler("echo", msgnet::ExampleMessageHandler::handleEcho);
        server.addMessageHandler("add", msgnet::ExampleMessageHandler::handleAdd);

        // 프로브 설정
        server.setStartProbe([](const msgnet::TcpServerConfig &config)
        {
            LOG_INFO("[Probe] Server started on port ", config.port,
                     " with ", config.accept_thread_count, " accept threads, ",
                     config.recv_thread_count, " recv threads, ",
                     config.process_thread_count, " process threads, ",
                     config.send_thread_count, " send threads. Log level: ", static_cast<int>(config.log_level));
        });

        server.setStopProbe([](const msgnet::TcpServerConfig &config)
        {
            LOG_INFO("[Probe] Server stopping on port ", config.port);
        });

        server.setExceptionProbe([](const int client_id, const int socket_fd, msgnet::ExceptionType et)
        {
            LOG_WARN("[Probe] Exception on client_id=", client_id, ", socket_fd=", socket_fd, ", type=", static_cast<int>(et));
        });

        // 서버 시작
        server.start();

        LOG_INFO("[Server] Listening on port ", port);
        LOG_INFO("[Server] Press Ctrl+C to stop.");

        // 메인 루프: 신호 대기
        while (g_run.load())
        {
            this_thread::sleep_for(milliseconds(200));
        }

        LOG_INFO("[Server] Stopping...");
        server.stop();
        LOG_INFO("[Server] Stopped.");
        return 0;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("[Fatal] ", e.what());
        return 1;
    }
}

int main(int argc, char **argv)
{
    return initServer(argc, argv);
}