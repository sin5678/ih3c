#pragma once

#include "resource.h"
#include "NetworkInfo.h"

struct Settings
{
	NetworkInfo::AdpaterIdentifier aid;
	string defGatewayAddr;
	int defGatewayPort;
	time_duration chkInterval;
};