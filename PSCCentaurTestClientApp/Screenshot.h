#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <string>
#include <windows.h> // ��� ������ � GDI

class Screenshot {
public:
    static std::string capture();
};

#endif // SCREENSHOT_H
