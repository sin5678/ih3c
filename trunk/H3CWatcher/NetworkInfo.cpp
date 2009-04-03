#include "stdafx.h"
#include "NetworkInfo.h"
#include "string_utils.h"

#include <IPHlpApi.h>
#include <SetupAPI.h>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void NetworkInfo::EnumerateAdapters( const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter )
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}
	UINT i = 1;

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			onGotAdapter(pAdapter->AdapterName, i, mbstowcs(pAdapter->Description).c_str());
			pAdapter = pAdapter->Next;
			++i;
		}
	} else {
		throw InvalidIdentifier("GetAdaptersInfo failed with error: %d\n"/*, dwRetVal*/);
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);
}

std::string NetworkInfo::GetGatewayAddress( const AdpaterIdentifier& adapterId )
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	std::string retStr;

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			throw InvalidIdentifier("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}
	UINT i = 1;

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if (adapterId == pAdapter->AdapterName)
				retStr = pAdapter->GatewayList.IpAddress.String;
			pAdapter = pAdapter->Next;
			++i;
		}
	} else {
		throw InvalidIdentifier("GetAdaptersInfo failed with error: %d\n"/*, dwRetVal*/);
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return retStr;
}

void NetworkInfo::SetConnection( const AdpaterIdentifier& adapterId, bool enabled )
{
	IN LPTSTR HardwareId = L"PCI\\VEN_10EC&DEV_8139&SUBSYS_813910EC" ;
	//硬件ComponentId，注册表地址：system\currentcontrolset\class\{4D36E972-E325-11CE-BFC1-08002BE10318}\0000

	DWORD NewState ;

	if(enabled == false)
	{
		NewState = DICS_DISABLE ;
		//禁用
	}
	else
	{
		NewState = DICS_ENABLE ;
		//启用
	}

	//调用ddk函数，来禁用网卡

	DWORD i,err ;
	BOOL Found=false ;

	HDEVINFO hDevInfo ;
	SP_DEVINFO_DATA spDevInfoData ;

	//访问系统的硬件库
	hDevInfo=SetupDiGetClassDevs(NULL,L"PCI", NULL,DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if(hDevInfo==INVALID_HANDLE_VALUE)
	{
		//printf("访问系统硬件出错！");
		return;
	}

	//枚举硬件，获得需要的接口
	spDevInfoData.cbSize=sizeof(SP_DEVINFO_DATA);
	for(i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&spDevInfoData);i++)
	{
		DWORD DataT ;
		LPTSTR p,buffer=NULL ;
		DWORD buffersize=0 ;

		//获得硬件的属性值
		while(!SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&spDevInfoData,
			SPDRP_HARDWAREID,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if(GetLastError()==ERROR_INVALID_DATA)
			{
				//不存在HardwareID. Continue.
				break ;
			}
			else if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				//buffer size不对.
				if(buffer)
					LocalFree(buffer);
				buffer=(LPTSTR)LocalAlloc(LPTR, buffersize*2);
			}
			else
			{
				//未知错误
				goto cleanup_DeviceInfo ;
			}
		}

		if(GetLastError()==ERROR_INVALID_DATA)
			continue ;

		//比较，找到和网卡ID相同的项
		for(p=buffer;*p&&(p<&buffer[buffersize]);p+=lstrlen(p)+sizeof(TCHAR))
		{

			if(!_tcscmp(HardwareId,p))
			{
				//找到网卡
				Found=TRUE ;
				break ;
			}
		}

		if(buffer)
			LocalFree(buffer);

		//如果相等
		if(Found)
		{
			//禁用该设备

			SP_PROPCHANGE_PARAMS spPropChangeParams ;

			spPropChangeParams.ClassInstallHeader.cbSize=sizeof(SP_CLASSINSTALL_HEADER);
			spPropChangeParams.ClassInstallHeader.InstallFunction=DIF_PROPERTYCHANGE ;
			spPropChangeParams.Scope=DICS_FLAG_GLOBAL ;
			spPropChangeParams.StateChange=NewState ;
			//禁用：DICS_DISABLE，DICS_ENABLE启用

			//
			if(!SetupDiSetClassInstallParams(hDevInfo,&spDevInfoData,(SP_CLASSINSTALL_HEADER*)&spPropChangeParams,sizeof(spPropChangeParams)))
			{
				DWORD errorcode=GetLastError();
			}

			if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&spDevInfoData))
			{
				DWORD errorcode=GetLastError();
			}

			switch(NewState)
			{
			case DICS_DISABLE :
				//printf("成功禁用网络！");
				break ;
			case DICS_ENABLE :
				//printf("成功启用网络！");
				break ;
			}

			break ;
		}

	}

	//退出时，清理工作环境
cleanup_DeviceInfo :
	err=GetLastError();
	SetupDiDestroyDeviceInfoList(hDevInfo);
	SetLastError(err);
}