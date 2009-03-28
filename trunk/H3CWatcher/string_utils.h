/* Copyright (c) 2009-2009 string_utils
 * Subject to the GPLv3 Software License. 
 * (See accompanying file GPLv3.txt or http://www.gnu.org/licenses/gpl.txt)
 * Author: Xiao, Yang
 */
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