#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <winsock2.h>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
#define TIMEOUT_DURATION std::chrono::minutes(3)
#define MAX_BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(serverAddr);
    int clientAddrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12000);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    getsockname(serverSocket, (struct sockaddr*)&serverAddr, &addrLen);
    std::cout << "\nServer address: " << inet_ntoa(serverAddr.sin_addr) << std::endl;
    std::cout << "Server port: " << ntohs(serverAddr.sin_port) << std::endl;

    int users = 0;
    while (users < 1) {
        if (std::cin.fail()) {
            std::cin.clear(); // Clear the fail state
            std::cin.ignore(256, '\n'); // Clear the input buffer
        }
        std::cout << "\nHow many users are we waiting for? : ";
        std::cin >> users;
        if (users < 1) std::cout << "no.";
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "\nServer is waiting for incoming connections..." << std::endl;

    std::vector<SOCKET> clientSockets;
    std::vector<std::pair<std::string, int>> ideasWithVotes;

    while (clientSockets.size() < users) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed. Error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected." << std::endl;
        clientSockets.push_back(clientSocket);
    }


    std::cout << "All clients connected. Idea submission phase started." << std::endl;

    // Ініціалізація таймера
    bool timerExpired = false;
    auto startTime = std::chrono::steady_clock::now();

    std::mutex ideasMutex; // Мютекс для синхронізації доступу до ideasWithVotes

    // Код, що був вище, без змін

    while (!timerExpired && ideasWithVotes.size() < clientSockets.size() * 5) {
        for (int j = 0; j < clientSockets.size(); j++) {
            for (size_t i = 0; i < 5; ++i) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - startTime);
                if (elapsedTime >= TIMEOUT_DURATION) {
                    timerExpired = true;
                    send(clientSockets[j], (char*)&timerExpired, sizeof(bool), 0);
                    break;
                }
                send(clientSockets[j], (char*)&timerExpired, sizeof(bool), 0);

                char recvBuffer[MAX_BUFFER_SIZE];
                int bytesReceived = recv(clientSockets[j], recvBuffer, sizeof(recvBuffer) - 1, 0);
                recvBuffer[bytesReceived] = '\0';

                std::string idea = recvBuffer;

                {
                    std::lock_guard<std::mutex> lock(ideasMutex); // Захоплення мютексу перед доступом до спільних даних
                    ideasWithVotes.push_back({ idea, 0 });
                } // Мютекс автоматично вивільняється при виході з області видимості lock_guard
                recvBuffer[0] = '\0';
            }
            if (timerExpired) break;
        }
    }


