#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include "json.hpp"

using namespace std;
using namespace std::chrono;

static bool recvAll(int fd, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    size_t got = 0;
    while (got < len) {
        ssize_t r = ::recv(fd, p + got, len - got, 0);
        if (r == 0) return false; // peer closed
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        got += static_cast<size_t>(r);
    }
    return true;
}

static bool sendAll(int fd, const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    size_t sent = 0;
    while (sent < len) {
        ssize_t s = ::send(fd, p + sent, len - sent, 0);
        if (s <= 0) {
            if (s < 0 && errno == EINTR) continue;
            return false;
        }
        sent += static_cast<size_t>(s);
    }
    return true;
}

int main(int argc, char** argv) {
    char* SERVER_IP = "127.0.0.1";
    int   SERVER_PORT = 55000;

    if (argc >= 2) {
        if (strcmp(argv[1], "d") != 0) 
            SERVER_IP = argv[1];
    }
    if (argc >= 3) {
        SERVER_PORT = std::stoi(argv[2]);
    }

    // 소켓 생성
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    // 연결
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    cout << "Connected to server\n";
    cout << "Enter JSON (one line). Ctrl+D to quit.\n\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line))
            break;

        // JSON 파싱 검사
        auto j = nlohmann::json::parse(line, nullptr, false);
        if (j.is_discarded()) {
            std::cerr << "Invalid JSON\n";
            continue;
        }

        string payload = j.dump();

        // length prefix
        uint32_t len = htonl(payload.size());

        if (!sendAll(sock, &len, sizeof(len)) ||
            !sendAll(sock, payload.data(), payload.size())) {
            cerr << "Send failed\n";
            break;
        }

        // 응답 수신
        uint32_t resp_len_net;
        if (!recvAll(sock, &resp_len_net, sizeof(resp_len_net))) {
            cerr << "Server closed connection\n";
            break;
        }

        uint32_t resp_len = ntohl(resp_len_net);
        string resp(resp_len, '\0');

        if (!recvAll(sock, resp.data(), resp_len)) {
            cerr << "Failed to receive response\n";
            break;
        }

        // 출력
        auto resp_json = nlohmann::json::parse(resp, nullptr, false);
        if (resp_json.is_discarded()) {
            cout << "[RAW RESPONSE]\n" << resp << "\n";
        } else {
            cout << "[RESPONSE]\n"
                      << resp_json.dump(2) << "\n";
        }
    }

    close(sock);
    cout << "Disconnected\n";
    return 0;
}
