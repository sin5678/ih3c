#pragma once

#pragma warning(push)
#pragma warning(disable:4290)

#include <string>
#include <tchar.h>

#ifndef tstring
typedef std::basic_string<TCHAR> tstring;
#endif

std::wstring mbstowcs(const std::string& str);
std::string wcstombs(const std::wstring& wstr);
std::wstring utf8towcs(const std::string& utf8str) throw(std::runtime_error);
std::string wcstoutf8(const std::wstring& wstr);

// replace characters not allowed in file name with param replace, default to '.'.
// replace \r\n\t with space ' ', and trim out the space.
std::string filenamize(const std::string& str, char replace = '.');
std::wstring filenamize(const std::wstring& str, char replace = '.');

#ifdef UNICODE
#define wcstotcs(s) (s)
#define mbstotcs(s) mbstowcs(s)
#define tcstowcs(s) (s)
#define tcstombs(s) wcstombs(s)
#else
#define wcstotcs(s) wcstombs(s)
#define mbstotcs(s) (s)
#define tcstowcs(s) mbstowcs(s)
#define tcstombs(s) (s)
#endif

#pragma warning(pop)