#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <windows.h> // Для определения BYTE

class Utils {
public:
    static std::string getComputerName();
    static std::string getUserName();
    static std::string base64Encode(const std::vector<BYTE>& data);
};

#endif // UTILS_H
