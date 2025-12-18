// main.cpp
#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

#include "TcpServer.h" // 네가 만든 서버 클래스
#include "json.hpp"

using namespace std;
using namespace std::chrono;

static std::atomic<bool> g_run{true};

static void onSignal(int)
{
    g_run = false;
}

int main(int argc, char **argv)
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
        TcpServer server(port);

        server.setRecvThreadCount(1);

        ExampleHandler handler;
        server.addHandler("ping", ExampleHandler::handlePing);
        server.addHandler("echo", ExampleHandler::handleEcho);
        server.addHandler("add", ExampleHandler::handleAdd);

        // 서버 시작
        server.start();

        cout << "[Server] Listening on port " << port << "\n";
        cout << "[Server] Press Ctrl+C to stop.\n";

        // 메인 루프: 신호 대기
        while (g_run.load())
        {
            this_thread::sleep_for(milliseconds(200));
        }

        cout << "\n[Server] Stopping...\n";
        server.stop();
        cout << "[Server] Stopped.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        cerr << "[Fatal] " << e.what() << "\n";
        return 1;
    }
}
