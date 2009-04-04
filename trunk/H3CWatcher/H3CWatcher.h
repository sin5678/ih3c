#pragma once

#include "resource.h"
#include "NetworkInfo.h"
#include <map>
#include <string>

struct Settings
{
	NetworkInfo::AdpaterIdentifier adapterId;
	string defGatewayAddr;
	int defGatewayPort;
	//int adapterPos;
	time_duration chkInterval;
};

struct AdapterInfo
{
	AdapterInfo(){}
	AdapterInfo(int adapterPos_, const wstring& name_)
		:adapterPos(adapterPos_), name(name_){}
	int adapterPos;
	wstring name;
};

void ChangeSettingsAndRestart(const Settings& s);

typedef map<NetworkInfo::AdpaterIdentifier, AdapterInfo> AdapterMap;
