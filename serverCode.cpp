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

    // Перевірка стану таймера
    if (timerExpired) {
        std::cout << "\nTimeout reached. Idea submission phase terminated." << std::endl;
    }
    else {
        std::cout << "\nIdea submission phase completed within the time limit." << std::endl;
    }

    std::cout << "\nIdea submission phase completed." << std::endl;
    std::stringstream ss;

    //голосування!
    std::string votingText = "Time to vote! \n";

    for (int i = 0; i < ideasWithVotes.size(); i++) {
        if (ideasWithVotes[i].first == "") break;
        ss << (i + 1) << " - " << ideasWithVotes[i].first << "\n";
    }

    votingText += ss.str();
    std::cout << "\n" << votingText;
    for (int i = 0; i < clientSockets.size(); i++) {
        send(clientSockets[i], votingText.c_str(), votingText.length(), 0);
    }

    std::cout << "Voting phase is now open. Clients can vote for the three best ideas." << std::endl;
    char num[4];

    for (int i = 0; i < clientSockets.size(); i++) {
        recv(clientSockets[i], num, sizeof(num), 0);
        int receivedNum = *reinterpret_cast<int*>(num);
        ss.str("");
        ss << "\nthe idea no." << receivedNum << " got a vote!";
        ideasWithVotes[receivedNum - 1].second++;
        std::cout << ss.str();
        num[0] = '\0';
    }

    int max[3] = { -1, -1, -1 };
    std::vector<int> votes(ideasWithVotes.size());
    for (int i = 0; i < ideasWithVotes.size(); i++) {
        votes[i] = ideasWithVotes[i].second;
    }

    auto number1 = std::max_element(votes.begin(), votes.end());
    int maxEl = *number1;
    max[0] = maxEl;

    int a = 0;
    if (clientSockets.size() > 1) {
        votes.clear();
        votes.resize(ideasWithVotes.size() - 1);

        for (int i = 0; i < ideasWithVotes.size(); i++) {
            if (ideasWithVotes[i].second == max[0]) continue;

            votes[a] = ideasWithVotes[i].second;
            a++;
        }
        auto number2 = std::max_element(votes.begin(), votes.end());
        maxEl = *number2;
        if (maxEl != 0) max[1] = maxEl;
    }

    if (clientSockets.size() > 2) {
        votes.clear();
        if (max[1] == -1) votes.resize(ideasWithVotes.size() - 1);
        else votes.resize(ideasWithVotes.size() - 2);
        a = 0;
        for (int i = 0; i < ideasWithVotes.size(); i++) {
            if (ideasWithVotes[i].second == max[0] || ideasWithVotes[i].second == max[1]) continue;

            votes[a] = ideasWithVotes[i].second;
            a++;
        }
        auto number3 = std::max_element(votes.begin(), votes.end());
        maxEl = *number3;
        if (maxEl != 0) max[2] = maxEl;
    }


    for (int i = 0; i < ideasWithVotes.size(); i++) {
        if (ideasWithVotes[i].second == max[0] || ideasWithVotes[i].second == max[1] || ideasWithVotes[i].second == max[2]) {
            std::cout << "\n\n" << ideasWithVotes[i].first;
        }
    }

    HANDLE hFile;
    DWORD dwBytesWritten;
    ss.str("");
    // Створення або відкриття файлу для запису
    hFile = CreateFileA("C:\\Users\\Тарас\\source\\repos\\Lab9OC\\lab9OS_Output.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Unable to create or open file." << std::endl;
        return 1;
    }

    // Запис даних у файл
    std::string text;

    for (int i = 0; i < ideasWithVotes.size(); i++) {

        std::string str1 = "Idea n.";
        std::string str3 = " : ";

        ss << "\n" << str1 << (i + 1) << str3 << ideasWithVotes[i].first;

        text = ss.str();

        if (!WriteFile(hFile, text.c_str(), strlen(text.c_str()), &dwBytesWritten, NULL)) {
            std::cerr << "Error writing to file." << std::endl;
            CloseHandle(hFile);
            return 1;
        }

        ss.str("");  //очищення stringstream

    }
    ss << "\ntop ideas: ";
    for (int i = 0; i < ideasWithVotes.size(); i++) {
        if (ideasWithVotes[i].second == max[0] || ideasWithVotes[i].second == max[1] || ideasWithVotes[i].second == max[2]) {
            ss << "\n" << ideasWithVotes[i].first << " has " << ideasWithVotes[i].second << " votes.";

        }
    }
    dwBytesWritten = 0;
    if (!WriteFile(hFile, ss.str().c_str(), strlen(ss.str().c_str()), &dwBytesWritten, NULL)) {
        std::cerr << "Error writing to file." << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    std::cout << "\nData written to file successfully!" << std::endl;

    // Закриття файлу
    CloseHandle(hFile);

    for (SOCKET clientSocket : clientSockets) {
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
