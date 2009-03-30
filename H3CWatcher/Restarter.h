/* Copyright (c) 2009
 * Subject to the GPLv3 Software License. 
 * (See accompanying file GPLv3.txt or http://www.gnu.org/licenses/gpl.txt)
 * Author: Xiao, Yang
 */

#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <memory>
#include "ServiceController.h"

class Restarter
{
public:
	typedef function<void(const wstring&, const wstring&)> MsgFunc;
protected:
	auto_ptr<thread> pcheckthrd;
	MsgFunc onMessage;
	time_duration checkInterval, restartInterval;
	string host;
	int port;
	bool lastTimeConnectable;
public:
	Restarter(const time_duration& checkInterval_, const string& host_,
		int port_, const MsgFunc& onMessage_)
		:host(host_), port(port_), lastTimeConnectable(false),
		onMessage(onMessage_), checkInterval(checkInterval_){}
	  
	~Restarter()
	{
		pcheckthrd.reset();
	}

	void Start()
	{
		pcheckthrd.reset(new thread(bind(&Restarter::Check, this)));
	}

	void TryRestart(const wstring& msg = L"H3C正在重新连接.....")
	{
		if(onMessage)
			onMessage(msg, L"H3C正在重连");
		ServiceController sc;
		if(!sc.RestartService(L"MyH3C"))
		{
			if(onMessage)
				onMessage(L"在重新启动MyH3C服务的过程中出现错误。可能是：\r\n"
				L"  > 无法取得对服务的控制权。请尝试以管理员的身份运行此程序。\r\n"
				L"  > 还未成功安装MyH3C服务。请以管理员的身份运行一次h3c_svr.exe。\r\n",
					L"无法重启MyH3C服务");
		}
	}

protected:
	void RestartMyH3C(const wstring& msg)
	{
		lastTimeConnectable = false;

		TryRestart(msg);
		this_thread::sleep(seconds(3));
	}

	class ConnectWithTimeout
	{
	public:
		ConnectWithTimeout()
			: timer_(io_service_), socket_(io_service_), succeeded(false)
		{
		}

		void run(const string& addr, int port, const time_duration& timeout)
		{
			socket_.async_connect(
				ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(addr), port),
				boost::bind(&ConnectWithTimeout::handle_connect, this,
				boost::asio::placeholders::error));

			timer_.expires_from_now(timeout);
			timer_.async_wait(boost::bind(&ConnectWithTimeout::close, this));
			io_service_.run();
		}

		void handle_connect(const boost::system::error_code& err)
		{
			succeeded = !(err);
			close();
		}

		void close()
		{
			socket_.close();
		}

		bool connection_succeeded() const
		{
			return succeeded;
		}
	
	private:  
		io_service io_service_;  
		deadline_timer timer_;  
		ip::tcp::socket socket_;
		bool succeeded;
	};

	void Check()
	{
		while(true)
		{
			ConnectWithTimeout cwt;
			cwt.run(host, port, seconds(2));

			if (cwt.connection_succeeded())
			{
				if(!lastTimeConnectable)
				{
					if(onMessage)
						onMessage(L"已经成功通过H3C连接到网络。", L"网络已正常连接");
					lastTimeConnectable = true;
				}
			}

			if ( !cwt.connection_succeeded() )
			{
				RestartMyH3C(L"超出指定时间未能连上网关，H3C连接可能已断开。\r\nH3C正在重新连接.....");
			}
			else
			{
				this_thread::sleep(checkInterval);
			}
		}
	}

};
