/////////////////////////
// 包括了以下几个功能：
// 1.输出错误信息: utils::MyH3CError(const T& errArg)
// 2.释放DHCP地址: utils::ReleaseDHCPAddr(int adapterID)
// 3.更新DHCP地址: utils::RenewDHCPAddr(int adapterID)

#pragma once
#include <string>
#include <sstream>
#include <fstream>
//#include <Windows.h>

namespace utils{

	//void SetLogFileName(const wchar_t* filename);
	std::wstring GetLogFileName();

	/**
	 * 输出错误信息。
	 * @param errMsg 错误。
	 */
	void MyH3CError(const std::wstring& errMsg);

	/**
	 * 释放当前从DHCP服务器获得的IP地址。
	 * @param adapterID 网卡ID。
	 * @return 操作是否成功。
	 */
	bool ReleaseDHCPAddr(int adapterID);

	/**
	 * 向DHCP服务器请求更新IP地址。
	 * @param adapterID 网卡ID。
	 * @return 操作是否成功。
	 */
	bool RenewDHCPAddr(int adapterID);
	
	/**
	 * 向所有网卡对应网络的DHCP服务器请求更新IP地址。
	 * @return 只要有某一网卡操作成功即返回true；无一成功则返回false。
	 */
	bool ReleaseAllDHCPAddr();

	/**
	 * 向所有网卡对应网络的DHCP服务器请求更新IP地址。
	 * @return 只要有某一网卡操作成功即返回true；无一成功则返回false。
	 */
	bool RenewAllDHCPAddr();

	/**
	 * 通过尝试连接网关的80端口，检查本机是否在线。
	 * @return 若连接成或，返回回true；否则返回false。
	 */
	bool CheckOnline(const std::string& adapterName);

};
