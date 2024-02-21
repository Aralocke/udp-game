#include "Common.h"

#include <Windows.h>

namespace Common
{
Position::Position(uint32_t x, uint32_t y)
    : x(x)
    , y(y)
{ }

std::string ErrorToString(int error)
{
    constexpr size_t kBufferLen = 1024;
    wchar_t buffer[kBufferLen] = { 0 };

    DWORD result = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        kBufferLen,
        NULL);

    auto Trim = [](std::string& str)
    {
        auto it = std::find_if(str.rbegin(), str.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            });

        str.erase(it.base(), str.end());
    };

    if (buffer)
    {
        std::wstring wstr(buffer, result);
        std::string str = WideToUtf8(wstr);
        Trim(str);

        return str;
    }

    return {};
}

std::string WideToUtf8(std::wstring_view wstr)
{
    int32_t size = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        nullptr,
        0,
        nullptr,
        nullptr);

    std::string str(size, '\0');

    if (!WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        nullptr))
    {
        return {};
    }

    return str;
}
}
