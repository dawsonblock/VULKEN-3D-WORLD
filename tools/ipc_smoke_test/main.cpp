#include <iostream>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <cstring>

int main() {
#if defined(_WIN32)
    WSADATA wsa; if (WSAStartup(MAKEWORD(2,2), &wsa)!=0) { std::cerr << "WSAStartup failed\n"; return 1; }
#endif
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { std::cerr << "socket failed\n"; return 1; }
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); addr.sin_port = 0;
    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) { perror("bind"); return 1; }
    socklen_t len = sizeof(addr);
    if (getsockname(server, reinterpret_cast<sockaddr*>(&addr), &len) < 0) { perror("getsockname"); return 1; }
    if (listen(server, 1) < 0) { perror("listen"); return 1; }
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) { perror("connect"); return 1; }
    int conn = accept(server, nullptr, nullptr);
    const char* msg = "ping";
    send(client, msg, std::strlen(msg), 0);
    char buf[16] = {0};
    recv(conn, buf, sizeof(buf), 0);
    std::cout << buf << std::endl;
#if defined(_WIN32)
    closesocket(client); closesocket(conn); closesocket(server); WSACleanup();
#else
    close(client); close(conn); close(server);
#endif
    return std::strcmp(buf, msg) == 0 ? 0 : 2;
}
