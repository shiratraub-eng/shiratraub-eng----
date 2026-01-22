#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

char my_id = '?'; // המזהה של הלקוח

void receive_messages(SOCKET sock) {
    char buffer[1024];
    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        std::string msg(buffer, bytes);

        // אם זו הודעה שמכילה את המזהה שלנו
        if (msg.find("Your ID is: ") == 0) {
            my_id = msg.back(); // מקבל את התו האחרון כ־ID
            std::cout << "Connected! Your ID is " << my_id << std::endl;
        }
        else {
            std::cout << "\n" << msg << std::endl;
        }

        std::cout << "To send: ";
        std::cout.flush();
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::thread(receive_messages, sock).detach();

    std::string msg;
    while (true) {
        std::cout << "To send: ";
        std::getline(std::cin, msg);
        if (msg.empty()) continue;

        // פורמט: targetID:message, לדוגמה B:שלום
        send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
