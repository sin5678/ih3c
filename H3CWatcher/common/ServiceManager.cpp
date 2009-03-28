#include "StdAfx.h"
#include "ServiceManager.h"
#pragma warning(disable:4482)
#pragma warning(disable:4018)
#pragma warning(disable:4800)

Service& Service::operator=(const Service& service)
{
	this->display_name = service.display_name;
	this->name = service.name;
	this->pid = service.pid;
	this->status = service.status;

	return *this;
}

bool ServiceManager::Initialize()
{
	manager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
	return (manager!=0);
}

void ServiceManager::CleanUp()
{
	if(manager)
	{
		CloseServiceHandle(manager);
		manager = 0;
	}
}

bool ServiceManager::GetServices(CServiceArray& services, int status_mask)
{
	services.clear();

	//form the mask
	int mask = 0;
	if((status_mask&Status::Active) && (status_mask&Status::Inactive))
		mask = SERVICE_STATE_ALL;
	else if(status_mask&Status::Active)
		mask = SERVICE_ACTIVE;
	else if(status_mask&Status::Inactive)
		mask = SERVICE_INACTIVE;
	else
		return false;

	int maxnum = 64000/sizeof(ENUM_SERVICE_STATUS_PROCESS);
	ENUM_SERVICE_STATUS_PROCESS *pData = new ENUM_SERVICE_STATUS_PROCESS[maxnum];
	int size = maxnum*sizeof(ENUM_SERVICE_STATUS_PROCESS); //the maximum
	DWORD needed = 0;
	DWORD num = 0;
	DWORD pos = 0;

	if(!EnumServicesStatusEx(manager,SC_ENUM_PROCESS_INFO,SERVICE_DRIVER|SERVICE_WIN32,
		mask,(BYTE*)pData,size,&needed,&num,&pos,NULL))
	{
		delete [] pData;
		return false;
	}

	//fill the array
	for(int i=0; i<num; ++i)
	{
		Service s;
		s.name = pData[i].lpServiceName;
		s.display_name = pData[i].lpDisplayName;
		s.pid = pData[i].ServiceStatusProcess.dwProcessId;
		s.status = (pData[i].ServiceStatusProcess.dwCurrentState==SERVICE_STOPPED)?Inactive:Active;
		services.push_back(s);
	}

	delete [] pData;
	return true;
}

bool ServiceManager::StartService(const TCHAR* name)
{
	SC_HANDLE service = OpenService(manager,name,SERVICE_START);
	if(!service)
		return false;

	bool ret = ::StartService(service,0,NULL);

	CloseServiceHandle(service);
	return ret;
}

bool ServiceManager::StopService(const TCHAR* name)
{
	SC_HANDLE service = OpenService(manager,name,SERVICE_STOP);
	if(!service)
		return false;

	SERVICE_STATUS status;
	bool ret = ControlService(service,SERVICE_CONTROL_STOP,&status);

	CloseServiceHandle(service);
	return ret;
}