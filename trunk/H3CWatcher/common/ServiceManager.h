/* Part of ServiceManager
 * Author: Denis Gourjii, http://www.codeproject.com/KB/system/ServiceManager.aspx
 * MFC dependency removed and replaced with STL data structures.
 */
#pragma once

#include <list>
#include <WinSvc.h>
#include "string_utils.h"

enum Status
{
	Unknown = 0,
	Active = 1,
	Inactive = 2
};

struct Service
{
public: //interface
	inline Service() : pid(0), status(Unknown) {}
	inline ~Service() {}
	inline Service(const Service& service) { *this = service; }
	Service& operator=(const Service& service);
	inline tstring GetName() { return name; }
	inline tstring GetDisplayName() { return display_name; }
	inline int GetProcessID() { return pid; }
	inline Status GetStatus() { return status; }

protected: //data
	tstring name;
	tstring display_name;
	int pid;
	Status status;

	friend class ServiceManager;
};

typedef std::list<Service> CServiceArray;

class ServiceManager
{
public: //interface
	inline ServiceManager(void) : manager(0) {}
	inline ~ServiceManager(void) { CleanUp(); }
	bool Initialize();
	void CleanUp();
	bool GetServices(CServiceArray& services, int status_mask = Active|Inactive);
	bool StartService(const TCHAR* name);
	bool StopService(const TCHAR* name);

protected: //data
	SC_HANDLE manager;

	friend struct Service;
};
