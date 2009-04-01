#pragma once
#include "Menu.h"
#include "H3CWatcher.h"
#include "NetworkInfo.h"

class FunctionMenu : public Menu
{
	Menu selAdptMenu;
	Settings* setting;
	HWND hMainWnd;
	MenuItem::CmdFunc onAbout;
	Restarter* restarter;
public:
	FunctionMenu(HWND hMainWnd_, Restarter* restarter_, Settings* setting_,
		const MenuItem::CmdFunc& onAbout_)
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

		//TODO:枚举网卡并动态生成菜单。
	}

	void Restart()
	{
		restarter->TryRestart();
	}

	void DisableCurConn()
	{
		//TODO:
	}
	
	void EnableCurConn()
	{
		//TODO:
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
};