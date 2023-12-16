//СЕРВЕР
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Лінкування з бібліотекою Winsock

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(serverAddr);
    int clientAddrLen = sizeof(clientAddr);
     
    // Ініціалізація Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock." << std::endl;
        return 1;
    }
    
    // Створення сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Налаштування адреси сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12000); // Встановлення порту сервера

    // Прив'язка сокета до адреси та порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed." << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    getsockname(serverSocket, (struct sockaddr*)&serverAddr, &addrLen);
    std::cout << "\nServer address: " << inet_ntoa(serverAddr.sin_addr) << std::endl;
    std::cout << "Server port: " << ntohs(serverAddr.sin_port) << std::endl;

    // Очікування підключення від клієнта
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "\nServer is waiting for incoming connections..." << std::endl;

    // Прийняття підключення від клієнта
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Accept failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
