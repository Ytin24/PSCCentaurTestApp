#include "Screenshot.h"
#include <windows.h>
#include <atlimage.h>
#include "Utils.h"

std::string Screenshot::capture() {
    HWND desktop = GetDesktopWindow();
    HDC desktopDC = GetDC(desktop);
    HDC memoryDC = CreateCompatibleDC(desktopDC);

    RECT desktopRect;
    GetWindowRect(desktop, &desktopRect);

    int width = desktopRect.right;
    int height = desktopRect.bottom;

    HBITMAP bitmap = CreateCompatibleBitmap(desktopDC, width, height);
    SelectObject(memoryDC, bitmap);
    BitBlt(memoryDC, 0, 0, width, height, desktopDC, 0, 0, SRCCOPY);

    CImage image;
    image.Attach(bitmap);

    IStream* stream = nullptr;
    CreateStreamOnHGlobal(nullptr, TRUE, &stream);
    image.Save(stream, Gdiplus::ImageFormatJPEG);

    STATSTG stats;
    stream->Stat(&stats, STATFLAG_NONAME);
    ULONG size = stats.cbSize.LowPart;

    std::vector<BYTE> buffer(size);
    LARGE_INTEGER liZero = {};
    stream->Seek(liZero, STREAM_SEEK_SET, nullptr);
    stream->Read(buffer.data(), size, nullptr);

    stream->Release();
    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    ReleaseDC(desktop, desktopDC);

    return Utils::base64Encode(buffer);
}
