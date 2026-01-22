#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::map<char, SOCKET> clients_map; //מזהה→ SOCKET
std::mutex clients_mutex;

void handle_client(char id, SOCKET client_socket) {
    char buffer[1024];

    while (true) {
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        std::string msg(buffer, bytes);

        //פורמט: recipient:message
        if (msg.size() < 3 || msg[1] != ':') continue;

        char recipient = msg[0];
        std::string text = msg.substr(2);

        std::lock_guard<std::mutex> lock(clients_mutex);
        if (clients_map.count(recipient)) {
            std::string final_msg = "From " + std::string(1, id) + ": " + text;
            send(clients_map[recipient], final_msg.c_str(), final_msg.size(), 0);
        }
    }

    closesocket(client_socket);
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients_map.erase(id);
    std::cout << "Client " << id << " disconnected." << std::endl;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server running on port 5000..." << std::endl;

    char next_id = 'A';
    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        char id = next_id++;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients_map[id] = client_socket;
        }
        std::cout << "Client " << id << " connected." << std::endl;

        //שולח ללקוח את המזהה שלו
        std::string welcome_msg = "Your ID is: ";
        welcome_msg += id;
        send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);

        std::thread(handle_client, id, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

