/* Copyright (c) 2009-2009 H3C Watcher
 * Subject to the GPLv3 Software License. 
 * (See accompanying file GPLv3.txt or http://www.gnu.org/licenses/gpl.txt)
 * Author: Xiao, Yang
 */

#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <memory>
#include "common/ServiceManager.h"

class Restarter
{
public:
	typedef function<void(const wstring&)> MsgFunc;
protected:
	auto_ptr<thread> prestartthrd, pcheckthrd;
	MsgFunc onMessage;
	ptime lastOKTime;
	time_duration maxNoConnTime, checkInterval;
	string host, port;
	bool lastTimeConnectable;
	volatile bool needRestart;
public:
	Restarter(const time_duration& maxNoConnTime_, const time_duration& checkInterval_, const string& host_,
		const string& port_, const MsgFunc& onMessage_)
		:maxNoConnTime(maxNoConnTime_), host(host_), port(port_), lastTimeConnectable(false),
		onMessage(onMessage_), needRestart(false), checkInterval(checkInterval_){}
	  
	~Restarter()
	{
		prestartthrd.reset();
		pcheckthrd.reset();
	}

	void Start()
	{
		pcheckthrd.reset(new thread(bind(&Restarter::Check, this)));
	}

protected:
	void Check()
	{
		while(true)
		{
			using namespace boost::asio;
			using namespace boost::asio::ip;
			using namespace boost;

			io_service io_service_;

			tcp::resolver resolver(io_service_);
			tcp::resolver::query query(ip::tcp::v4(), host, lexical_cast<std::string>(port));
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::resolver::iterator end;
			tcp::socket socket_(io_service_);

			boost::system::error_code error = boost::asio::error::host_not_found;
			while (error && endpoint_iterator != end)
			{
				socket_.close();
				socket_.connect(*endpoint_iterator++, error);
			}

			ptime now = microsec_clock::local_time();
			if (error)
			{
			}
			else
			{
				socket_.close();
				lastOKTime = now;
				if(!lastTimeConnectable)
				{
					if(onMessage) onMessage(L"网络连接正常。");
					lastTimeConnectable = true;
				}
			}

			if ( lastOKTime.is_not_a_date_time() || now - lastOKTime > maxNoConnTime )
			{
				lastTimeConnectable = false;
				if(onMessage)
					onMessage(L"超出指定时间未能连上网关，H3C连接可能已断开。\r\nH3C正在重新连接.....");
				if(!prestartthrd.get())
				{
					prestartthrd.reset(new thread(bind(&Restarter::Restart, this)));
				}
				needRestart = true;
			}

			this_thread::sleep(checkInterval);
		}
	}

	void Restart()
	{
		while(true)
		{
			if(needRestart)
			{
				if(onMessage)
					onMessage(L"H3C正在重新连接.....");
				ServiceManager sm;
				if(!sm.Initialize())
				{
					if(onMessage) onMessage(L"无法取得对服务的控制权。请尝试以管理员的身份运行此程序。");
				}

				sm.StopService(L"MyH3C");
				sm.StartService(L"MyH3C");

				needRestart = false;
			}
			else
			{
				this_thread::sleep(milliseconds(500));
			}
		}
	}
};
