#include "stdafx.h"
#include "Menu.h"
#include <exception>

Menu::Menu(bool isPopup)
{
	if(isPopup){
		hMenu = ::CreatePopupMenu();
	}else{
		hMenu = ::CreateMenu();
	}
	MENUINFO minfo = {sizeof(minfo), MIM_MENUDATA|MIM_STYLE, MNS_AUTODISMISS|MNS_NOTIFYBYPOS, 0, NULL, 0, (ULONG_PTR)this };
	if(!::SetMenuInfo(hMenu, &minfo))
		throw std::exception("Menu::SET_MENU_INFO_FAILED, SetMenuInfo failed in construction.");
}

Menu::~Menu()
{
	::DestroyMenu(hMenu);
}

BOOL Menu::CheckRadioItem( UINT pos )
{
	return ::CheckMenuRadioItem( hMenu, 0, ::GetMenuItemCount(hMenu)-1, pos, MF_BYPOSITION );
}

BOOL Menu::Insert(UINT pos, MenuItem* pInfo)
{
	cmdFuncs.insert(cmdFuncs.begin()+pos, pInfo->onCmd);
	return ::InsertMenuItem(hMenu, pos, TRUE, pInfo);
}

MenuItem::CmdFunc* Menu::GetCmdFunc(UINT pos)
{
	return cmdFuncs[pos].get();
}

BOOL Menu::Remove(UINT pos)
{
	if(::RemoveMenu(hMenu, pos, MF_BYPOSITION))
	{
		cmdFuncs.erase(cmdFuncs.begin()+pos);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL Menu::Modify(UINT pos, MenuItem* pInfo)
{
	return Remove(pos) && Insert(pos, pInfo);
}

Menu* Menu::FromHMENU(HMENU hMenu)
{
	MENUINFO minfo = {sizeof(minfo), MIM_MENUDATA, 0, 0, NULL, 0, NULL };
	if(::GetMenuInfo(hMenu, &minfo)){
		return (Menu*)(minfo.dwMenuData);
	}else{
		return NULL;
	}
}

BOOL Menu::Append( MenuItem* pInfo )
{
	return Insert(::GetMenuItemCount(hMenu), pInfo);
}

void Menu::onWM_MENUCOMMAND(WPARAM wParam, LPARAM lParam)
{
	Menu *p = FromHMENU((HMENU)lParam);
	if(p)
	{
		MenuItem::CmdFunc* func = p->GetCmdFunc(wParam);
		if(func) (*func)();
	}
}