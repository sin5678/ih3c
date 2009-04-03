#pragma once

#include <exception>

//Get network infos.
class NetworkInfo
{
public:
	typedef std::wstring AdpaterIdentifier;

	class InvalidIdentifier : public runtime_error{
	public:
		InvalidIdentifier(const char* msg):runtime_error(msg){}
	};

	class ComError : public runtime_error{
	public:
		ComError(const char* msg):runtime_error(msg){}
	};

	//adpaterPos: 传递给服务、写入pw.data中的“第几个网卡”。
	static void EnumerateAdapters(
		const function<void(const AdpaterIdentifier&/*id*/, int /*adapterPos*/, const wstring&/*name*/)>& onGotAdapter );

	static string GetGatewayAddress( const AdpaterIdentifier& adapterId );

	static void SetConnection( const AdpaterIdentifier& adapterId, bool enabled );
};