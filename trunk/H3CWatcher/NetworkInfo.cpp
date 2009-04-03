#include "stdafx.h"
#include "NetworkInfo.h"
#include "string_utils.h"

#include <IPHlpApi.h>
#include <SetupAPI.h>
#include <ObjBase.h>
#include <NetCon.h>
#include <atlbase.h>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void NetworkInfo::EnumerateAdapters( const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter )
{
	CComPtr<INetConnectionManager> pManager;
	CComPtr<INetConnection>  pConnection; 
	CComPtr<IEnumNetConnection>  pEnum; 
	ULONG celtFetched = 0; 
	HRESULT hr = 0;

	//Get  an  Instance  of  INetConnectionManager 
	hr = CoCreateInstance(CLSID_ConnectionManager,  NULL,  CLSCTX_SERVER,
		IID_INetConnectionManager,  (void**)&pManager);
	if (FAILED(hr))
		throw ComError("Can't create INetConnectionManager");
	//Enumerate  the  Network  Connections 
	hr = pManager->EnumConnections(NCME_DEFAULT,  &pEnum); 
	if (FAILED(hr))
		throw ComError("Can't EnumConnections");

	int index = 0;
	OLECHAR guid[256] = {0};
	while(  pEnum->Next(1,  &pConnection,  &celtFetched)  ==  S_OK  ) 
	{ 
		NETCON_PROPERTIES*  properties = 0;
		//Get  the  properties  of  a  Network  Connection 
		hr = pConnection->GetProperties(&properties);
		if (FAILED(hr))
			throw ComError("Can't get connection properties");

		StringFromGUID2(properties->guidId, guid, sizeof(guid));

		onGotAdapter (guid, index, properties->pszwName);

		CoTaskMemFree(properties);
		pConnection.Release();

		++index;
	}
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
			if (adapterId == mbstowcs(pAdapter->AdapterName))
				retStr = pAdapter->GatewayList.IpAddress.String;
			pAdapter = pAdapter->Next;
			++i;
		}
	} else {
		//throw InvalidIdentifier("GetAdaptersInfo failed with error: %d\n"/*, dwRetVal*/);
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return retStr;
}

void NetworkInfo::SetConnection( const AdpaterIdentifier& adapterId, bool enabled )
{
	CComPtr<INetConnectionManager> pManager;
	CComPtr<INetConnection>  pConnection; 
	CComPtr<IEnumNetConnection>  pEnum; 
	ULONG celtFetched = 0; 
	HRESULT hr = 0;

	//Get  an  Instance  of  INetConnectionManager 
	hr = CoCreateInstance(CLSID_ConnectionManager,  NULL,  CLSCTX_SERVER,
		IID_INetConnectionManager,  (void**)&pManager);
	if (FAILED(hr))
		throw ComError("Can't create INetConnectionManager");
	//Enumerate  the  Network  Connections 
	hr = pManager->EnumConnections(NCME_DEFAULT,  &pEnum); 
	if (FAILED(hr))
		throw ComError("Can't EnumConnections");

	int index = 0;
	OLECHAR guid[256] = {0};
	while(  pEnum->Next(1,  &pConnection,  &celtFetched)  ==  S_OK  ) 
	{ 
		NETCON_PROPERTIES*  properties = 0;
		//Get  the  properties  of  a  Network  Connection 
		hr = pConnection->GetProperties(&properties);
		if (FAILED(hr))
			throw ComError("Can't get connection properties");

		StringFromGUID2(properties->guidId, guid, sizeof(guid));
		if (guid == adapterId)
		{
			if (enabled == false)
				hr = pConnection->Disconnect(); 
			else
				hr = pConnection->Connect(); 

			if (FAILED(hr))
				throw ComError("Can't set connection");
		}
		CoTaskMemFree(properties);
		pConnection.Release();

		++index;
	}
}