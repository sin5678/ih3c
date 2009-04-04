#include "stdafx.h"
#include "UrlOpen.h"

int APIENTRY _tWinMain(HINSTANCE /*hInstance*/,
											 HINSTANCE /*hPrevInstance*/,
											 LPTSTR    lpCmdLine,
											 int       /*nCmdShow*/)
{
	ShellExecute (0, _T("open"), _T(URL), 0, 0, SW_SHOW);
	return EXIT_SUCCESS;
}