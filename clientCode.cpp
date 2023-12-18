#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Лінкування з бібліотекою Winsock

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;

    // Ініціалізація Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Створення сокета
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Налаштування адреси сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("10.50.175.151"); // IP-адреса сервера "185.204.69.10" 
    serverAddr.sin_port = htons(12000); // Порт сервера

    // Підключення до сервера
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Connection failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
   
      std::cout << "Connected to server." << std::endl;
    //std::cout << "\nEnter ideas: (if you're out of ideas, enter - ";
    // Операції з сервером (обмін даними) тут...
    for (int i = 0; i < 5; i++) {

        bool timerExpired = false;
        recv(clientSocket, (char*)&timerExpired, sizeof(bool), 0);
        if (timerExpired) break;

        std::cout << "Enter idea no." << (i + 1) << ": ";
        std::string messageStr;
        std::getline(std::cin, messageStr);

        const char* message = messageStr.c_str();
        send(clientSocket, message, strlen(message), 0);
        if (messageStr == "-") break;
    }

    // Отримання стрічки
    std::string receivedText;
    char recvBuffer[1024];
    int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesReceived <= 0) {
        std::cout << "Error receiving data." << std::endl;
        //break;
    }
    receivedText.append(recvBuffer, bytesReceived);
    // Отримана стрічка
    std::cout << receivedText << std::endl;

    int choice = 0;
    std::cout << "\nPick your favourite idea: ";
    std::cin >> choice;

    send(clientSocket, reinterpret_cast<char*>(&choice), sizeof(choice), 0);

    // Закриття сокета
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
