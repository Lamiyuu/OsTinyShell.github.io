#include "Conversion.h"
#include <windows.h>
wchar_t* convertToWideChar(const char* charStr) {
    size_t len = strlen(charStr) + 1;
    wchar_t* wideStr = new wchar_t[len];
    mbstowcs(wideStr, charStr, len);
    return wideStr;
}
char* convertToChar(const wchar_t* wstr) {
    int wstr_length = wcslen(wstr);
    int required_length = WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_length, NULL, 0, NULL, NULL);
    char* str = new char[required_length + 1];
    WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_length, str, required_length, NULL, NULL);
    str[required_length] = '\0';
    return str;
}