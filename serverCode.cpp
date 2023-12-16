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
// Приймання даних від клієнта

    std::cout << "Client connected." << std::endl;
    char recvBuffer[1024];
    std::string ideas[5];
    for (int i = 0; i < 5; i++) {
        int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0';
            std::cout << "Idea no." << (i+1) << " from the client: " << recvBuffer << std::endl;
            
        }
        ideas[i] = recvBuffer;
        recvBuffer[0] = '\0';
    }

    HANDLE hFile;
    DWORD dwBytesWritten;

    hFile = CreateFileA("C:\\Users\\Taras\\lab9OS_Output.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Unable to create or open file." << std::endl;
        return 1;
    }

    // Запис даних у файл
    std::stringstream ss;
    std::string text;

    for (int i = 0; i < 5; i++) {
        
        std::string str1 = "Idea no.";
        //std::string str2 = (i+1);
        std::string str3 = " : ";

        ss << "\n" << str1 << (i + 1) << str3 << ideas[i];

        text = ss.str();

        if (!WriteFile(hFile, text.c_str(), strlen(text.c_str()), &dwBytesWritten, NULL)) {
            std::cerr << "Error writing to file." << std::endl;
            CloseHandle(hFile);
            return 1;
        }

        ss.str("");
    }
    

    std::cout << "Data written to file successfully!" << std::endl;

    // Закриття файлу
    CloseHandle(hFile);

    // Закриття сокетів
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
}
