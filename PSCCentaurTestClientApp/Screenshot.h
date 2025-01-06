#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <string>
#include <windows.h> // Для работы с GDI

class Screenshot {
public:
    static std::string capture();
};

#endif // SCREENSHOT_H
