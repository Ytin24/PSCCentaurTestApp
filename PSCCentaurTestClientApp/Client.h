#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <winsock2.h>
#include <vector>

class Client {
public:
    Client(const std::string& serverIp, int serverPort);
    ~Client();

    void connectToServer();
    void run();

private:
    bool sendMessage(const std::string& message);
    bool sendMessageWithLength(const std::vector<char>& data);
    void handleServerMessages();
    void registerToServer();
    void handleReconnection();

    SOCKET clientSocket;
    sockaddr_in serverAddress;
    bool running;
};

#endif // CLIENT_H
