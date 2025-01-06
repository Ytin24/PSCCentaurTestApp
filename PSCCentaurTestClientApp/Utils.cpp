#include "Utils.h"
#include <windows.h>

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
