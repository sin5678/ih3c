#pragma once
#include <windows.h>
#pragma comment(lib, "advapi32.lib")

class ServiceController
{
	SC_HANDLE schSCManager;

	bool StopService(LPCTSTR szSvcName)
	{
		//¹Ø±Õ·þÎñ¡£
		SERVICE_STATUS_PROCESS ssp;
		DWORD dwStartTime = GetTickCount();
		DWORD dwBytesNeeded;
		DWORD dwTimeout = 30000; // 30-second time-out

		SC_HANDLE schService = OpenService( 
			schSCManager,         // SCM database 
			szSvcName,            // name of service 
			SERVICE_STOP | 
			SERVICE_QUERY_STATUS | 
			SERVICE_ENUMERATE_DEPENDENTS);  
	 
		if (schService == NULL)
		{ 
			return false;
		}

		if ( !QueryServiceStatusEx( 
				schService, 
				SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssp, 
				sizeof(SERVICE_STATUS_PROCESS),
				&dwBytesNeeded ) )
		{
			CloseServiceHandle(schService);
			return false;
		}


		if ( !ControlService( 
				schService, 
				SERVICE_CONTROL_STOP, 
				(LPSERVICE_STATUS) &ssp ) )
		{
			//( "ControlService failed (%d)\n", GetLastError() );
			//CloseServiceHandle(schService);
			//return false;
		}

		// Wait for the service to stop.

		while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
		{
			Sleep( ssp.dwWaitHint );
			if ( !QueryServiceStatusEx( 
					schService, 
					SC_STATUS_PROCESS_INFO,
					(LPBYTE)&ssp, 
					sizeof(SERVICE_STATUS_PROCESS),
					&dwBytesNeeded ) )
			{
				//printf( "QueryServiceStatusEx failed (%d)\n", GetLastError() );
				CloseServiceHandle(schService);
				return false;
			}

			if ( ssp.dwCurrentState == SERVICE_STOPPED )
				break;

			if ( GetTickCount() - dwStartTime > dwTimeout )
			{
				//printf( "Wait timed out\n" );
				CloseServiceHandle(schService);
				return false;
			}
		}
		//printf("Service stopped successfully\n");
		return true;
	}

public:
	~ServiceController()
	{
		if(schSCManager)
			CloseServiceHandle(schSCManager);
	}

	//@return false indicates the lack of privileges.
	bool Initialize()
	{
		schSCManager = OpenSCManager( 
			NULL,                    // local computer
			NULL,                    // servicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 
	 
		return (NULL != schSCManager) ;
	}

	bool RestartService(LPCTSTR szSvcName)
	{
		SERVICE_STATUS_PROCESS ssStatus; 
		DWORD dwOldCheckPoint; 
		DWORD dwStartTickCount;
		DWORD dwWaitTime;
		DWORD dwBytesNeeded;
		SC_HANDLE schService;

		// Get a handle to the service.

		schService = OpenService( 
			schSCManager,         // SCM database 
			szSvcName,            // name of service 
			SERVICE_ALL_ACCESS);  // full access 
	 
		if (schService == NULL)
		{ 
			return false;
		}    

		// Check the status in case the service is not stopped. 

		if (!QueryServiceStatusEx( 
				schService,                     // handle to service 
				SC_STATUS_PROCESS_INFO,         // information level
				(LPBYTE) &ssStatus,             // address of structure
				sizeof(SERVICE_STATUS_PROCESS), // size of structure
				&dwBytesNeeded ) )              // size needed if buffer is too small
		{
			CloseServiceHandle(schService); 
			return false; 
		}

		// Check if the service is already running.

		if(ssStatus.dwCurrentState != SERVICE_STOPPED)
		{
			if(ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				// Wait for the service to stop before attempting to start it.

				while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
				{
					// Save the tick count and initial checkpoint.

					dwStartTickCount = GetTickCount();
					dwOldCheckPoint = ssStatus.dwCheckPoint;

					// Do not wait longer than the wait hint. A good interval is 
					// one-tenth of the wait hint but not less than 1 second  
					// and not more than 10 seconds. 
			 
					dwWaitTime = ssStatus.dwWaitHint / 10;

					if( dwWaitTime < 1000 )
						dwWaitTime = 1000;
					else if ( dwWaitTime > 10000 )
						dwWaitTime = 10000;

					Sleep( dwWaitTime );

					// Check the status until the service is no longer stop pending. 
			 
					if (!QueryServiceStatusEx( 
							schService,                     // handle to service 
							SC_STATUS_PROCESS_INFO,         // information level
							(LPBYTE) &ssStatus,             // address of structure
							sizeof(SERVICE_STATUS_PROCESS), // size of structure
							&dwBytesNeeded ) )              // size needed if buffer is too small
					{
						//("QueryServiceStatusEx failed (%d)\n", GetLastError());
						CloseServiceHandle(schService); 
						return false; 
					}

					if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
					{
						// Continue to wait and check.

						dwStartTickCount = GetTickCount();
						dwOldCheckPoint = ssStatus.dwCheckPoint;
					}
					else
					{
						if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
						{
							//("Timeout waiting for service to stop\n");
							CloseServiceHandle(schService); 
							return false; 
						}
					}
				}
			}
			else
			{
				StopService(szSvcName);
			}
		}


		// Attempt to start the service.

		if (!StartService(
				schService,  // handle to service 
				0,           // number of arguments 
				NULL) )      // no arguments 
		{
			//("StartService failed (%d)\n", GetLastError());
			CloseServiceHandle(schService); 
			return false; 
		}
		//else printf("Service start pending...\n"); 

		// Check the status until the service is no longer start pending. 
	 
		if (!QueryServiceStatusEx( 
				schService,                     // handle to service 
				SC_STATUS_PROCESS_INFO,         // info level
				(LPBYTE) &ssStatus,             // address of structure
				sizeof(SERVICE_STATUS_PROCESS), // size of structure
				&dwBytesNeeded ) )              // if buffer too small
		{
			//("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CloseServiceHandle(schService); 
			return false; 
		}
	 
		// Save the tick count and initial checkpoint.

		dwStartTickCount = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
		{ 
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth the wait hint, but no less than 1 second and no 
			// more than 10 seconds. 
	 
			dwWaitTime = ssStatus.dwWaitHint / 10;

			if( dwWaitTime < 1000 )
				dwWaitTime = 1000;
			else if ( dwWaitTime > 10000 )
				dwWaitTime = 10000;

			Sleep( dwWaitTime );

			// Check the status again. 
	 
			if (!QueryServiceStatusEx( 
				schService,             // handle to service 
				SC_STATUS_PROCESS_INFO, // info level
				(LPBYTE) &ssStatus,             // address of structure
				sizeof(SERVICE_STATUS_PROCESS), // size of structure
				&dwBytesNeeded ) )              // if buffer too small
			{
				//("QueryServiceStatusEx failed (%d)\n", GetLastError());
				break; 
			}
	 
			if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
			{
				// Continue to wait and check.

				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ssStatus.dwCheckPoint;
			}
			else
			{
				if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
				{
					// No progress made within the wait hint.
					break;
				}
			}
		} 

		// Determine whether the service is running.

		//if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
		//{
		//	printf("Service started successfully.\n"); 
		//}
		//else 
		//{ 
		//	printf("Service not started. \n");
		//	printf("  Current State: %d\n", ssStatus.dwCurrentState); 
		//	printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
		//	printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
		//	printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 
		//} 
		bool result = (ssStatus.dwCurrentState == SERVICE_RUNNING) ;

		CloseServiceHandle(schService); 

		return result;
	}
};