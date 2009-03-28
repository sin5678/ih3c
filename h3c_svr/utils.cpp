#include "utils.h"
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <sstream>
#include <string>
#include <tchar.h>
#include <iphlpapi.h>
#include <WinSock.h>
using namespace std;

wstring GetExecutableFilePath()
{
	const int BUFF_SIZE = 2048;
	wchar_t pathBuf[BUFF_SIZE] = {0};
	::GetModuleFileNameW(0, pathBuf, BUFF_SIZE);

	wstring fullPath = pathBuf;
	fullPath.erase(fullPath.find_last_of(L'\\') + 1);
	return fullPath;
}

wstring logFileName = GetExecutableFilePath() + L"log.txt";

std::wstring utils::GetLogFileName()
{
	return logFileName;
}

//void utils::SetLogFileName(const wchar_t* filename)
//{
//	logFileName = filename;
//}

void utils::MyH3CError(const std::wstring& errMsg)
{
	static bool firstRun = true;
	std::ofstream fstrm(GetLogFileName().c_str(), std::ios::out|std::ios::binary|(firstRun?0:std::ios::app), 0x40);
	if(!(!fstrm)){
		if(firstRun)
		{
			fstrm.put((char)0xFF);
			fstrm.put((char)0xFE);
		}
		fstrm.write((const char*)errMsg.c_str(), errMsg.size()*2);
		fstrm.write((const char*)L"\r\n", 4);
		fstrm.close();
	}
	firstRun = false;
}

bool DoReleaseOrRenew(bool isRelease, int adapterID)
{
	PIP_INTERFACE_INFO pInfo = NULL;

	ULONG size = 0;

	if( ::GetInterfaceInfo(NULL, &size) == ERROR_INSUFFICIENT_BUFFER )
	{
		string buff;
		buff.resize(size);
		pInfo = (PIP_INTERFACE_INFO)buff.c_str();
		if( ::GetInterfaceInfo( pInfo, &size) == NO_ERROR )
		{
			if(adapterID>=0)
			{
				if(adapterID>=pInfo->NumAdapters)
				{
					std::wostringstream os;
					os<<adapterID;
					utils::MyH3CError(L"error: “" + os.str() + L"”不是有效的网卡ID。");
					return false;
				}
				if(isRelease)
					return ( ::IpReleaseAddress( &pInfo->Adapter[adapterID] ) == NO_ERROR );
				else return ( ::IpRenewAddress( &pInfo->Adapter[adapterID] ) == NO_ERROR );
			}
			else
			{
				bool anyoneOK = false;
				for(int i=0; i<pInfo->NumAdapters; i++)
				{
					if(isRelease)
					{
						if ( ::IpReleaseAddress( &pInfo->Adapter[i] ) == NO_ERROR )
							anyoneOK = true;
					}
					else
					{
						if ( ::IpRenewAddress( &pInfo->Adapter[i] ) == NO_ERROR )
							anyoneOK = true;
					}
				}
				return anyoneOK;
			}
		}
	}
	return false;
}

bool utils::ReleaseDHCPAddr(int adapterID)
{
	return DoReleaseOrRenew(true, adapterID);
}

bool utils::RenewDHCPAddr(int adapterID)
{
	return DoReleaseOrRenew(false, adapterID);
}

bool utils::ReleaseAllDHCPAddr()
{
	return DoReleaseOrRenew(true, -1);
}

bool utils::RenewAllDHCPAddr()
{
	return DoReleaseOrRenew(false, -1);
}


class WSAStarterAndCleaner{
public:
	::WSADATA WSAData;
	WSAStarterAndCleaner()
	{
		::WSAStartup(MAKEWORD(2,2), &WSAData);
	}
	~WSAStarterAndCleaner()
	{
		::WSACleanup();
	}
};

WSAStarterAndCleaner wsc;

bool CheckOnline(const std::string& adapterName)
{
	//获得网关IP地址.
	ULONG len = 0;
    if (GetAdaptersInfo(NULL, &len) == ERROR_BUFFER_OVERFLOW)
	{
		std::string buff;
		buff.resize(len);
		PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)buff.c_str();
		if ( GetAdaptersInfo(pAdapterInfo, &len) == NO_ERROR )
		{
			PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
			bool found = false;
			std::string gatewayAddr;
			while(pAdapter)
			{
				if(adapterName.compare(std::string(pAdapter->AdapterName))==0)
				{
					gatewayAddr = pAdapter->GatewayList.IpAddress.String;
					found = true;
					break;
				}
				pAdapter = pAdapter->Next;
			}
			if(found)
			{
				//尝试连接网关的80端口.
				SOCKET s;
				s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (s != INVALID_SOCKET) {
					sockaddr_in addr; 
					addr.sin_family = AF_INET;
					addr.sin_addr.s_addr = inet_addr( gatewayAddr.c_str() );
					addr.sin_port = htons( 27015 );

					if ( connect( s, (SOCKADDR*) &addr, sizeof(addr) ) != SOCKET_ERROR) {
						closesocket(s);
						return true;
					}
				}
			}
		}
	}
	return false;
}