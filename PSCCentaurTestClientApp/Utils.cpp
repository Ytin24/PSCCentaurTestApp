#include "Utils.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <shlwapi.h> // Для PathFindFileNameA

#pragma comment(lib, "shlwapi.lib")

#define REGISTRY_KEY_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

std::string Utils::getComputerName() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    GetComputerNameA(buffer, &size);
    return std::string(buffer);
}

std::string Utils::getUserName() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    GetUserNameA(buffer, &size);
    return std::string(buffer);
}

std::string Utils::base64Encode(const std::vector<BYTE>& data) {
    static const char* base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    int val = 0, valb = -6;

    for (BYTE c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    return encoded;
}

std::string Utils::getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
}

std::string Utils::getExecutableName() {
    std::string fullPath = getExecutablePath();
    return PathFindFileNameA(fullPath.c_str());
}

void Utils::addToStartup() {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        return;
    }

    std::string exePath = getExecutablePath();
    std::string appName = getExecutableName();

    result = RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(exePath.c_str()), exePath.size() + 1);
    if (result != ERROR_SUCCESS) {
    }
    else {
    }

    RegCloseKey(hKey);
}

// Проверка, находится ли в автозагрузке
bool Utils::isInStartup() {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::string appName = getExecutableName(); // Используем имя файла как ключ

    char buffer[MAX_PATH];
    DWORD bufferSize = sizeof(buffer);
    result = RegQueryValueExA(hKey, appName.c_str(), 0, NULL, reinterpret_cast<BYTE*>(buffer), &bufferSize);
    RegCloseKey(hKey);

    return result == ERROR_SUCCESS;
}

void Utils::removeFromStartup() {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, 0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        std::string appName = getExecutableName();
        RegDeleteValueA(hKey, appName.c_str());
        RegCloseKey(hKey);
    }
    else {
    }
}
