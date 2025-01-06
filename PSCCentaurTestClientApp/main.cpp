#include "Client.h"
#include <iostream>
#include <fstream>
#include <windows.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // Скрываем консольное окно
        FreeConsole();

        Client client(SERVER_IP, SERVER_PORT);
        client.connectToServer();
        client.run();
    }
    catch (const std::exception& e) {
    }

    return 0;
}
