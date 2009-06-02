#include "supplicant.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "utils.h"
#include "Version.h"

using namespace std;

////////////////////////////////////////////////////////////
// Declare several global variables to share 
// their values across multiple functions of your program.
////////////////////////////////////////////////////////////
SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus;
extern USERDATA stUserData;

////////////////////////////////////////////////////////////
// Make the forward definitions of functions prototypes.
////////////////////////////////////////////////////////////
void ServiceMain(int argc, char** argv); 
void ControlHandler(DWORD request); 
void ReportSvcStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

// Control Handler
#define SVCNAME "MyH3c"

void ControlHandler(DWORD request) 
{ 
	switch(request) 
	{ 
	case SERVICE_CONTROL_STOP: 
	case SERVICE_CONTROL_SHUTDOWN: 
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 3000 );
		//::ExitProcess (0);
		return;

	default:
		break;
	} 

	// Report current status
	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 3000 );
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
void ReportSvcStatus( DWORD dwCurrentState,
										 DWORD dwWin32ExitCode,
										 DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		ServiceStatus.dwControlsAccepted = 0;
	else ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ( (dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED) )
		ServiceStatus.dwCheckPoint = 0;
	else ServiceStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus( hStatus, &ServiceStatus );
}

extern USERDATA stUserData;

void ServiceMain(int /*argc*/, char** /*argv*/) 
{ 

#ifdef _DEBUG
	::Sleep( 10000 );
#endif

	ServiceStatus.dwServiceType = 
		SERVICE_WIN32; 
	ServiceStatus.dwCurrentState = 
		SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted   =  
		SERVICE_ACCEPT_STOP | 
		SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	ServiceStatus.dwCheckPoint = 0; 
	ServiceStatus.dwWaitHint = 0; 

	hStatus = RegisterServiceCtrlHandler(
		SVCNAME, 
		(LPHANDLER_FUNCTION)ControlHandler); 
	if (hStatus == 0) 
	{ 
		utils::MyH3CError(L"error: Registering Control Handler failed");
		return; 
	}  

	// We report the running status to SCM. 
	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 3000 );

	std::string cfgFileName = "pw.data";

	char pathBuf[MAX_PATH] = {0};
	::GetModuleFileNameA(0, pathBuf, sizeof(pathBuf));

	std::string fullPath = pathBuf;
	fullPath.erase(fullPath.find_last_of('\\') + 1);
	fullPath += cfgFileName;

	std::ifstream ifs(fullPath.c_str());
	if (ifs.is_open() == false)
		ExitProcess(0);

	std::string userName,pw;
	ifs>>userName>>pw;
	int ncIndex = 0;
	ifs>>ncIndex;
	ifs.close();

	memset(&stUserData,0,sizeof(stUserData));
	memcpy(stUserData.username, userName.c_str(), 
		min(userName.size(), sizeof(stUserData.username)));
	memcpy(stUserData.password, pw.c_str(), 
		min(pw.size(), sizeof(stUserData.password)));

	if (GetNIC(ncIndex) == FALSE)		//指定得到第几个网卡的信息
	{
		ReportSvcStatus( SERVICE_STOPPED, ERROR_BAD_ENVIRONMENT, 3000 );
		utils::MyH3CError( L"Error: Unable to get adapter's information." );
	}

	StartSupplicant ();

	ReportSvcStatus( SERVICE_STOPPED, ERROR_INVALID_DATA, 3000 );
}

void SvcInstall()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
	{
		std::ostringstream os;
		os<<GetLastError();
		std::cout<<"Cannot install service ("<<os.str()<<")\n";
		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager) 
	{
		std::ostringstream os;
		os<<GetLastError();
		std::cout<<"OpenSCManager failed ("<<os.str()<<")\n";
		return;
	}

	// Create the service.

	schService = CreateService( 
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_AUTO_START,		   // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL) 
	{
		std::ostringstream os;
		os<<GetLastError();
		std::cout<<"CreateService failed ("<<os.str()<<")\n";
		CloseServiceHandle(schSCManager);
		return;
	}
	else utils::MyH3CError(L"message: Service installed successfully"); 

	SERVICE_DESCRIPTION dpt = {0};
	dpt.lpDescription = "iH3C 客户端核心认证服务,编写人:胡杨,杨潇 (05CSB)";
	ChangeServiceConfig2 (schService, SERVICE_CONFIG_DESCRIPTION, &dpt);

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
}

bool InstallService ()
{
	std::cout<<"Installing iH3C Svr.\n";

	SvcInstall();

	return true;
}

bool UnInstallService ()
{
	std::cout<<"Uninstalling iH3C Svr.\n";

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager) 
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return false;
	}

	// Get a handle to the service.

	schService = OpenService( 
		schSCManager,       // SCM database 
		SVCNAME,          // name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{ 
		printf("OpenService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Delete the service.

	if (! DeleteService(schService) ) 
	{
		printf("DeleteService failed (%d)\n", GetLastError()); 
	}
	else printf("Service deleted successfully\n"); 

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	return true;
}

bool StartMyService ()
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = SVCNAME;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	return StartServiceCtrlDispatcher(ServiceTable) != FALSE;
}

bool ListDevice ()
{
	pcap_if_t *alldevs = 0;
	char errbuf[PCAP_ERRBUF_SIZE];

	//获取设备列表
	if (pcap_findalldevs(&alldevs, errbuf) == -1) {
		return false;
	}
	//得到每个网卡的描述信息
	int index = 0;
	for(pcap_if_t* dev = alldevs; dev; dev = dev->next, ++index)
	{
		cout<<"ID: "<<index<<endl;
		cout<<"Name: "<<dev->name<<endl;
		cout<<"Description: "<<dev->description<<endl<<endl;
	}

	pcap_freealldevs(alldevs);

	return true;
}

void Usage()
{
	std::cout<<"Wrong argument!\n";
	std::cout<<"Usage: \n";
	std::cout<<"-i install me\n";
	std::cout<<"-u uninstall me\n";
	std::cout<<"-l list devices\n";
	std::cout<<"-v show version\n";
}

void ShowVersion()
{
	std::cout<< "Version: \n";
	std::cout<< VERSION;
}

int main( int argc, char* argv[])
{
	switch (argc)
	{
	case 1:
		StartMyService();
		break;

	case 2:
		{
			std::string argu(argv[1]);
			if (argu == "-i")
				InstallService();
			else if (argu == "-u")
				UnInstallService();
			else if (argu == "-l")
				ListDevice();
			else if (argu == "-v")
				ShowVersion();
			else
				Usage();
		}
		break;

	default:
		Usage();
		break;
	}

	return EXIT_SUCCESS;
}

