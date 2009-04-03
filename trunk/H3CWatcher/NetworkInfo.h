#pragma once
#include "stdafx.h"
#include <exception>

//Get network infos.
class NetworkInfo
{
public:
	typedef std::string AdpaterIdentifier;		//TODO: replace int with actual adpater identifier

	class InvalidIdentifier : public runtime_error{
	public:
		InvalidIdentifier(const char* msg):runtime_error(msg){}
	};

	//adpaterPos: 传递给服务、写入pw.data中的“第几个网卡”。
	static void EnumerateAdapters(
		const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter );

	static string GetGatewayAddress( const AdpaterIdentifier& adapterId );

	static void SetConnection( const AdpaterIdentifier& adapterId, bool enabled );
};