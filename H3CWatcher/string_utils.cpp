#include "stdafx.h"
#include "string_utils.h"
#include <sstream>
#include <iomanip>
using namespace std;
#pragma warning(disable:4290)
#pragma warning(disable:4996)

class global_locale_setter{
public:
    global_locale_setter(){
        std::locale::global(std::locale(""));
    }
};
global_locale_setter setter;

std::wstring mbstowcs(const string& r)
{
    wstring result;
    size_t result_len = mbstowcs(NULL, r.c_str(), r.size());
	if(result_len!=-1)
	{
		result.resize(result_len);
		wchar_t *result_buf = &result[0];
		//result_buf[result_len-1] = 0;
		mbstowcs(result_buf,  r.c_str(), result_len);
	}
    return result;
}

std::string wcstombs(const wstring& r)
{
    string result;
    size_t result_len = wcstombs(NULL, r.c_str(), r.size());
    result.resize(result_len);
    char *result_buf = const_cast<char*>(result.data());
    //result_buf[result_len-1] = 0;
    wcstombs(result_buf,  r.c_str(), result_len);
    return result;
}

std::wstring utf8towcs(const std::string& utf8str) throw(std::runtime_error)
{
    std::wstring result;
    result.reserve(utf8str.size());

    std::string::const_iterator utf8Itr = utf8str.begin();
    std::string::const_iterator utf8ItrEnd = utf8str.end();
    if(utf8Itr==utf8ItrEnd)
        return result;
    for (;;)
    {
        wchar_t t = 0;
        if (*utf8Itr&0x80 && *utf8Itr&0x40 && *utf8Itr&0x20 && !(*utf8Itr&0x10))
        {
            t |= ((wchar_t)(*utf8Itr&0xF))<<12;
            utf8Itr++;
            if(utf8Itr==utf8ItrEnd) break;
            t |= ((wchar_t)(*utf8Itr&0x3F))<<6;
            utf8Itr++;
            if(utf8Itr==utf8ItrEnd) break;
            t |= ((wchar_t)(*utf8Itr&0x3F));
        }else if(!(*utf8Itr&0x80)){
            t = *utf8Itr;
        }else{
            ostringstream o;
            o<<"The format of UTF-8 character "<<setbase(16)<<*utf8Itr<<" is not supported now.";
            throw runtime_error(o.str());
        }
        utf8Itr++;
        if(utf8Itr==utf8ItrEnd) break;
        result.push_back(t);
    }
    return result;
}

std::string wcstoutf8(const std::wstring& wstr)
{
    std::string result;
    result.reserve(wstr.size()*3+3);

    for (std::wstring::const_iterator wstrItr = wstr.begin(); wstrItr!=wstr.end(); wstrItr++)
    {
        if (!(*wstrItr&0xFF80))
        {
            result.push_back(char(*wstrItr));
        }else{
            result.push_back((((char)((*wstrItr)>>12))&0xF)|0xE0);
            result.push_back((((char)((*wstrItr)>>6))&0x3F)|0x80);
            result.push_back((((char)((*wstrItr)))&0x3F)|0x80);
        }
    }
    return result;
}
