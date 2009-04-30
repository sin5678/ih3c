/* Copyright (c) 2009
 * Subject to the GPLv3 Software License. 
 * (See accompanying file GPLv3.txt or http://www.gnu.org/licenses/gpl.txt)
 * Author: Xiao, Yang
 */

// H3CWatcher.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "H3CWatcher.h"
#include "Restarter.h"
#include "NetworkInfo.h"
#include "string_utils.h"
#include "version.h"
#include "FunctionMenu.h"

#include <sstream>
#include <fstream>
#include <shellapi.h>
#include <ObjBase.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

NOTIFYICONDATA nid = {0};
const UINT WM_NOTIFYICON = WM_USER+111;
const DWORD MY_TRAY_ICON_ID = 1;
HICON appIcon = NULL;
HMENU popupMenu = NULL;
HWND hMainWnd = NULL;

mutex showMsgMtx;

void ShowBubbleMessage(const wstring& message, const wstring& title);
auto_ptr<Restarter> restarter;
Settings curSetting;

auto_ptr<FunctionMenu> menu;

AdapterMap adapterInfos;

void OnGotAdapter(const NetworkInfo::AdpaterIdentifier& id, int adapterPos, const wstring& name)
{
	adapterInfos[id] = AdapterInfo(adapterPos, name);
}

//Not includes terminating '\\' or '/'.
wstring GetMainDirPath()
{
	wstring str;
	size_t size = 10240;
	str.resize(size);
	::GetModuleFileName(NULL, &(str[0]), size);
	wstring result = &(str[0]);
	return result.substr( 0, result.find_last_of( L"\\/" ) );
}

const wstring PWDATA_PATHNAME = GetMainDirPath()+L"\\pw.data";
const wstring WATCHER_SETTING_PATHNAME = GetMainDirPath()+L"\\setting.data";
const wstring WATCHER_PATHNAME = GetMainDirPath()+L"\\H3CWatcher.exe";

void SaveSettings()
{
	const wchar_t * crlf = L"\r\n";
	wofstream wostrm( WATCHER_SETTING_PATHNAME.c_str(), ios::out|ios::binary );
	if ( wostrm )
	{
		wostrm << curSetting.adapterId << crlf 
			<< mbstowcs( curSetting.defGatewayAddr ) << crlf
			<< curSetting.defGatewayPort << crlf;
		wostrm.close();
	}
	else
	{
		MessageBox( NULL, L"无法保存设置到setting.data。", L"提示", MB_OK|MB_ICONINFORMATION );
	}
}

//读入设置。
void LoadSettings()
{
	// default values
	curSetting.defGatewayAddr = "172.18.59.254";
	curSetting.defGatewayPort = 80;
	curSetting.chkInterval = seconds(5);
	//curSetting.adapterPos = 1;

	{
		wstring gateway;
		wifstream strm( WATCHER_SETTING_PATHNAME.c_str() );
		if ( strm )
		{
			strm >> curSetting.adapterId >> gateway >> curSetting.defGatewayPort;
			if ( strm.rdstate() & (ios::failbit|ios::badbit) )
			{
				SaveSettings();
			}
			else
			{
				curSetting.defGatewayAddr = wcstombs(gateway);
			}
			strm.close();
		}
	}

	//获得网卡列表。
	adapterInfos.clear();
	NetworkInfo::EnumerateAdapters(&OnGotAdapter);
	if ( !adapterInfos.empty() && adapterInfos.find( curSetting.adapterId ) == adapterInfos.end() )
	{
		curSetting.adapterId = adapterInfos.begin()->first;
		SaveSettings();
	}
}

//根据设置重启restarter。在程序开始时、以及设置变更时调用。
void LaunchRestarter()
{
	string addr;
	try
	{
		addr = NetworkInfo::GetGatewayAddress( curSetting.adapterId );
	}
	catch ( const runtime_error& )
	{
	}
	if ( !addr.empty() && curSetting.defGatewayAddr!=addr )
	{
		curSetting.defGatewayAddr = addr;
		SaveSettings();
	}
	restarter.reset(new Restarter(curSetting.chkInterval, curSetting.defGatewayAddr,
		curSetting.defGatewayPort, &ShowBubbleMessage));
	restarter->Start();
}

void ChangeSettingsAndRestart(const Settings& s)
{
	curSetting = s;
	SaveSettings();
	LaunchRestarter();
}

const wchar_t* IH3C_REG_NAME = L"iH3C";
const wchar_t* REG_STARTUP_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

bool RegisterStartUp()
{
	HKEY hkey;
	wstring path = WATCHER_PATHNAME;

	if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		REG_STARTUP_KEY, 0, KEY_ALL_ACCESS, &hkey ) )
	{
		LONG ret = RegSetValueEx( hkey, IH3C_REG_NAME, 0, REG_SZ, (BYTE*)path.c_str(),
			(path.size()+1)*sizeof(path[0]) );
		RegCloseKey(hkey);
		return ERROR_SUCCESS == ERROR_SUCCESS;
	}
	return false;
}

bool UnregisterStartUp()
{
	HKEY hkey;

	if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		REG_STARTUP_KEY, 0, KEY_ALL_ACCESS, &hkey ) )
	{
		LONG ret = RegDeleteValue( hkey, IH3C_REG_NAME );
		RegCloseKey(hkey);
		return ERROR_SUCCESS == ERROR_SUCCESS;
	}
	return false;
}

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	if ( lpCmdLine[0]!=0 )
	{
		wstring cmdLine = lpCmdLine;
		if ( L"-i"==cmdLine )
		{
			return RegisterStartUp()?0:1;
		}
		else if( L"-u"==cmdLine )
		{
			return UnregisterStartUp()?0:2;
		}
		else
		{
			MessageBox(NULL, L"Usage: H3CWatcher [ -i | -u ]\r\n"
				L"  -i : Register as start-up application\r\n"
				L"  -u : Start-up application unregister\r\n"
				, L"CmdLine", MB_OK);
			return 3;
		}
	}

	//Initialize  COM 
	CoInitialize(NULL); 

	appIcon = (HICON)LoadImage( hInstance,
            MAKEINTRESOURCE(IDI_H3CWATCHER),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR);

	popupMenu = GetSubMenu(LoadMenu(hInstance, MAKEINTRESOURCE(IDR_POPUPMENU)), 0);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_H3CWATCHER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, /*nCmdShow*/SW_HIDE))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_H3CWATCHER));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//uninit
	CoUninitialize(); 

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= appIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_H3CWATCHER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= appIcon;

	return RegisterClassEx(&wcex);
}

template<typename ArrayT>
inline size_t ArrayElementCount(const ArrayT& t)
{
	return sizeof(t)/sizeof(t[0]);
}

template<typename T, typename ArrayT>
void CopyStringIntoArray(const basic_string<T>& s, ArrayT& buff)
{
	memcpy(buff, s.c_str(), std::min(ArrayElementCount(buff), s.size()+1)*sizeof(T));
}

void ShowAbout()
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hMainWnd, About);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hMainWnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

	//OK，在系统托盘显示图标。
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
	nid.uID = MY_TRAY_ICON_ID;
	nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = appIcon;
    CopyStringIntoArray<wchar_t>(L"H3CWatcher", nid.szTip);

	nid.hWnd = hWnd;

	Shell_NotifyIcon(NIM_ADD, &nid);

	LoadSettings();
	LaunchRestarter();

	//初始化菜单。
	menu.reset( new FunctionMenu(hWnd, restarter.get(), &curSetting, &ShowAbout, adapterInfos ) );

   return TRUE;
}

void ShowBubbleMessage(const wstring& message, const wstring& title)
{
	mutex::scoped_lock lck(showMsgMtx);
	nid.uFlags = NIF_INFO;
	CopyStringIntoArray(message, nid.szInfo);
    CopyStringIntoArray(title, nid.szInfoTitle);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void ShowCurSetting()
{
	wostringstream strm;
	wstring tab = L"  ";
	ptime restartTime = restarter->GetLastRestartTime();
	strm<<L"IPv4连接测试地址: \r\n"
		<<tab<<mbstowcs(curSetting.defGatewayAddr)
		<<L", 端口 = "<<curSetting.defGatewayPort<<L"\r\n"
		//<<L"H3C User: \r\n"
		<<L"上一次H3C服务重启时间: ";
	if ( restartTime.is_not_a_date_time() )
	{
		strm<<L"无";
	}
	else
	{
		strm<<restartTime;
	}
	AdapterInfo &ai = adapterInfos[curSetting.adapterId];
	strm<<L"\r\n"
		<<L"网卡: ("<<ai.adapterPos
		<<L")"<<ai.name<<L"\r\n"
		<<L"网络状况检查间隔: "<<curSetting.chkInterval<<L"\r\n";
	ShowBubbleMessage(strm.str(), L"iH3C信息");
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_MENUCOMMAND:
		Menu::onWM_MENUCOMMAND(wParam, lParam);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		Shell_NotifyIcon(NIM_DELETE, &nid);
		break;
	case WM_NOTIFYICON:
		{
			if(lParam==WM_RBUTTONUP)
			{
				if(menu.get())
				{
					SetForegroundWindow(hWnd);
					POINT pt = {0};
					GetCursorPos(&pt);
					TrackPopupMenu(*menu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
				}
			}
			else if(lParam==WM_LBUTTONUP)
			{
				ShowCurSetting();
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			wstring s = L"iH3CWatcher, version " VERSION;
			::SetDlgItemTextW( hDlg, IDC_PRODUCT_NAME, s.c_str());
			return (INT_PTR)TRUE;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
