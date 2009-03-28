#pragma once
#include "stdafx.h"

//Get network infos.
class NetworkInfo
{
public:
	typedef int AdpaterIdentifier;		//TODO: replace int with actual adpater identifier

	static void EnumerateAdapters(
		function<void(AdpaterIdentifier/*id*/, const wstring&/*name*/)>& onGotAdapter )
	{
		//TODO: implement this
	}

	static string GetGatewayAddress( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
		return "";
	}

	static void DisableConnection( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
	}
	
	static void EnableConnection( const AdpaterIdentifier& adapterId )
	{
		//TODO: implement this
	}
}