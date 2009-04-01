#pragma once
#include "stdafx.h"
#include <vector>

//≤Àµ•°£ —Ó‰Ï£¨2009.1.1

class MenuItem : public MENUITEMINFO{
public:
	typedef function<void()> CmdFunc;
	auto_ptr<CmdFunc> onCmd;
public:
	MenuItem(LPTSTR text, const CmdFunc& onCmd_, HMENU subMenu = NULL) : onCmd(new CmdFunc(onCmd_))
	{
		cbSize = (sizeof(MENUITEMINFO));
		fMask = (
#if(WINVER > 0x0400)
			MIIM_STRING
#else
			MIIM_TYPE
#endif /* WINVER = 0x0400 */
			|MIIM_SUBMENU);
		fType = (MFT_STRING);
		fState = (0);
		wID = (0); hSubMenu = (subMenu); hbmpChecked = (NULL); hbmpUnchecked = (NULL);
		dwTypeData = (text); cch = (0);
#if(WINVER >= 0x0500)
		hbmpItem = (NULL);
#endif /* WINVER >= 0x0500 */
	}
};

class MenuSaperator : public MenuItem{
public:
	MenuSaperator() : MenuItem(NULL, CmdFunc())
	{
		fMask = 
#if(WINVER > 0x0400)
			MIIM_FTYPE;
#else
			MIIM_TYPE;
#endif /* WINVER = 0x0400 */
		fType = MFT_SEPARATOR;
	}
};

class Menu{
protected:
	vector<shared_ptr<MenuItem::CmdFunc>> cmdFuncs;
protected:
	MenuItem::CmdFunc* GetCmdFunc(UINT pos);
public:
	HMENU hMenu;
public:
	Menu(bool isPopup);
	~Menu();
	BOOL Append(MenuItem* pInfo);
	BOOL Insert(UINT pos, MenuItem* pInfo);
	BOOL Remove(UINT pos);
	BOOL Modify(UINT pos, MenuItem* pInfo);
	static Menu* FromHMENU(HMENU hMenu);
public:
	operator HMENU() const{return hMenu;}
public:
	static void onWM_MENUCOMMAND(WPARAM wParam, LPARAM lParam);
};