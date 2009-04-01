#pragma once
#include "Menu.h"
#include "H3CWatcher.h"
#include "NetworkInfo.h"
#include <sstream>

class FunctionMenu : public Menu
{
	Menu selAdptMenu;
	Settings* setting;
	HWND hMainWnd;
	MenuItem::CmdFunc onAbout;
	Restarter* restarter;
public:
	FunctionMenu(HWND hMainWnd_, Restarter* restarter_, Settings* setting_,
		const MenuItem::CmdFunc& onAbout_, const AdapterMap& adapters)
		:Menu(true), selAdptMenu(true), setting(setting_), hMainWnd(hMainWnd_),
		onAbout(onAbout_), restarter(restarter_)
	{
		Append( &MenuItem( L"重新启动MyH3C服务(&R)", bind( &FunctionMenu::Restart, this ) ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"选择网卡(&S)", MenuItem::CmdFunc(), selAdptMenu ) );
		Append( &MenuItem( L"禁用当前连接(&D)", bind( &FunctionMenu::DisableCurConn, this ) ) );
		Append( &MenuItem( L"启用当前连接(&E)", bind( &FunctionMenu::EnableCurConn, this ) ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"帮助(&H)", bind( &FunctionMenu::Help, this ) ) );
		Append( &MenuItem( L"关于(&A)...", onAbout ) );
		Append( &MenuSaperator() );
		Append( &MenuItem( L"退出(&X)", bind( &FunctionMenu::Exit, this ) ) );

		//枚举网卡并动态生成菜单。
		UINT pos = 0;
		for(AdapterMap::const_iterator iter = adapters.begin(), end=adapters.end();
			iter!=end; iter++, pos++)
		{
			wostringstream strm;
			strm<<L"("<<iter->second.adapterPos<<L") "<<iter->second.name;
			selAdptMenu.Append( &MenuItem( const_cast<wchar_t*>( strm.str().c_str() ),
				bind( &FunctionMenu::OnSelectAdapter, this, iter->first, pos ) ) );
			//Check the menu item.
			if( iter->first==setting->adapterId )
				selAdptMenu.CheckRadioItem( pos );
		}
	}

	void OnSelectAdapter( const NetworkInfo::AdpaterIdentifier& id, UINT pos )
	{
		setting->adapterId = id;
		ChangeSettingsAndRestart( *setting );
		selAdptMenu.CheckRadioItem( pos );
	}

	void Restart()
	{
		restarter->TryRestart();
	}

	void DisableCurConn()
	{
		try
		{
			NetworkInfo::EnableConnection( setting->adapterId, false );
		}
		catch(const NetworkInfo::InvalidIdentifier& err)
		{
			ShowErrorMessage(err.what());
		}
	}
	
	void EnableCurConn()
	{
		try
		{
			NetworkInfo::EnableConnection( setting->adapterId, true );
		}
		catch(const NetworkInfo::InvalidIdentifier& err)
		{
			ShowErrorMessage(err.what());
		}
	}

	void Help()
	{
		//TODO:
	}

	void Exit()
	{
		if ( MessageBox( hMainWnd, L"确定要退出吗？退出之后将失去断线重连功能。", L"iH3C退出确认",
			MB_ICONQUESTION|MB_OKCANCEL ) == IDOK )
		{
			SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		}
	}

	void ShowErrorMessage(const char* msg)
	{
		MessageBoxA(hMainWnd, msg, "Error", MB_OK|MB_ICONERROR);
	}
};