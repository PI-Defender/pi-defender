/**
 * @file       U_Controller.cpp
 * @brief      Controll the service/driver (Install|Start|Stop|Delete|Query)
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Controller.hpp"


extern U_COMMUNICATION Communication;
extern sSERVICE strctService;


SC_HANDLE U_CONTROLLER::_GetSCMHandle()
/**
 * @brief   Get a handle to the Service Controller Database.
 * 
 * @param   None
 * 
 * @return  SC_HANDLE (no error occurs)
 *          NULL (error occurs) 
*/
{
	SC_HANDLE hSCManager = NULL;

	// Get SCM database handle
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL)
		_tprintf_s(TEXT("[-] Cannot get a handle to the SCM database\n"));

	return hSCManager;
}

SC_HANDLE U_CONTROLLER::_GetServiceHandle(_In_ SC_HANDLE hSCManager, _In_ PCWSTR szServiceName)
/**
 * @brief       Get a handle to the service
 * 
 * @param[in]   "hSCManager"    - Handle to the service Manager
 * @param[in]   "szServiceName" - Name of the service
 * 
 * @return      SC_HANDLE (no error occurs)\n
 *              NULL (error occurs)
*/
{
	SC_HANDLE hService = NULL;

	// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
	hService = OpenService(hSCManager, szServiceName, SC_MANAGER_ALL_ACCESS);
	if (hService == NULL)
		_tprintf_s(TEXT("[-] Cannot get a handle to the service %s\n"), szServiceName);

	return hService;
}

BOOLEAN U_CONTROLLER::IsServiceInstalled(_In_ PCWSTR szServiceName)
/**
 * @brief       Check if the service is already installed
 * 
 * @param[in]   "szServiceName" - Name of the service
 * 
 * @return      TRUE (no error occurs) - FALSE (error occurs)
*/
{
	BOOLEAN bResult = FALSE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;

	__try {

		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL)
		{
			_tprintf_s(TEXT("[-] Error unable to GetSCMHandle\n"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we got an error.
		hService = OpenService(hSCManager, szServiceName, SC_MANAGER_ALL_ACCESS);
		if (hService == NULL)		// Error if service not installed
			__leave;		// Return TRUE to the caller -> He has to install the service

		bResult = TRUE;			// Service already installed

	}
	__finally {

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return bResult;

	}
}

BOOLEAN U_CONTROLLER::InstallService(_In_ PCWSTR szServiceName, _In_ BOOLEAN bDriver)
/**
 * @brief       Install the service "szServiceName".
 * 
 * @param[in]   "szServiceName" - Name of the service.
 * 
 * @return      TRUE (no error occurs)
 *              FALSE (error occurs)
*/
{
	BOOLEAN bResult = FALSE;
	HRESULT hResult;
	LSTATUS lResult = 0;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;
	PWSTR szPath = NULL;
	PWSTR szFinalDriverPath = NULL;
	DWORD dwSizePath = 0;
	DWORD dwTag = 2;

	__try {

		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL)
		{
			_tprintf_s(TEXT("[-] Error GetSCMHandle\n"));
			__leave;
		}

		szPath = (PWSTR)calloc(MAX_PATH, sizeof(WCHAR));
		if (szPath == NULL) {
			_tprintf_s(TEXT("[-] Error cannot allocate memory %d\n"), GetLastError());
			__leave;
		}

		//
		// Get the current folder where the executable is store
		// The .sys must be in the same folder as the executable
		// If bDriver = True, we append the current folder with the name of the .sys
		// If bDriver = False, we append the current folder with the name of the executable
		//

		if (bDriver) {
			//
			// DRIVER KERNEL
			//

			bResult = U_HELPER::GetCurrentFolder(szPath);
			if (!bResult)
			{
				_tprintf_s(TEXT("[-] Error unable to GetCurrentFolder for %s\n"), szServiceName);
				__leave;
			}

			hResult = PathCchAppend(szPath, MAX_PATH, DRIVER_SYS);
			if (FAILED(hResult))
			{
				_tprintf_s(TEXT("[-] Error trying to append \"PI-Defender.sys\" to the path %s\n"), szPath);
				__leave;
			}

			// Install service (Driver kernel)
			hService = CreateService(
				hSCManager,
				szServiceName,
				szServiceName,
				SERVICE_ALL_ACCESS,
				SERVICE_FILE_SYSTEM_DRIVER,
				SERVICE_DEMAND_START,
				SERVICE_ERROR_NORMAL,
				szPath,
				TEXT(GROUP),
				&dwTag,
				TEXT(DEPEND_ON_SERVICE),
				NULL,
				NULL
			);
		}

		else {
			//
			// SERVICE UM
			//

			dwSizePath = GetModuleFileName(NULL, szPath, MAX_PATH);
			if (dwSizePath == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				_tprintf_s(TEXT("[-] Error unable to GetModuleFileName for %s\n"), szServiceName);
				__leave;
			}

			// Append " service" parameter to path
			wcscat_s(szPath, MAX_PATH, TEXT(" service"));

			// Install service (Service UM)
			hService = CreateService(
				hSCManager,
				szServiceName,
				szServiceName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START,
				SERVICE_ERROR_NORMAL,
				szPath,
				NULL, NULL, NULL, NULL, NULL
			);
		}

		if (hService == NULL)
		{
			_tprintf_s(TEXT("[-] Error CreateService\n"));
			__leave;
		}

		if (bDriver)
		{
			// Now, we need to create the registry key "Instances" & "PI-Defender Instance"
			lResult = U_REGISTRY::CreateRegistryKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\PI-Defender\\Instances\\PI-Defender Instance"));
			if (lResult != ERROR_SUCCESS)
				__leave;

			// Create "DefaultInstance" child key to "Instances"
			lResult = U_REGISTRY::WriteStringInRegistry(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PI-Defender\\Instances", TEXT("DefaultInstance"), (PWSTR)TEXT("PI-Defender Instance"));
			if (lResult != ERROR_SUCCESS)
				__leave;

			// Create "Altitude" & "Flags" child key to "PI-Defender Instance"
			lResult = U_REGISTRY::WriteStringInRegistry(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PI-Defender\\Instances\\PI-Defender Instance", TEXT("Altitude"), (PWSTR)DRIVER_ALTITUDE);
			if (lResult != ERROR_SUCCESS)
				__leave;

			lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PI-Defender\\Instances\\PI-Defender Instance", TEXT("Flags"), 0);
			if (lResult != ERROR_SUCCESS)
				__leave;
		}

		bResult = TRUE;

	}
	__finally {

		if (hService)
			CloseServiceHandle(hService);

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		return bResult;

	}
}

DWORD U_CONTROLLER::GetServiceState(_In_ PCWSTR szServiceName)
/**
 * @brief       Query the service "szServiceName" and return its state.
 *
 * @param[in]   "szServiceName" - Name of the service
 *
 * @return      dwStatus - current state of the service, can be one of the following values:
 *              SERVICE_CONTINUE_PENDIND, SERVICE_PAUSE_PENDING, SERVICE_PAUSED, SERVICE_RUNNING,
 *              SERVICE_START_PENDING, SERVICE_STOP_PENDING, SERVICE_STOPPED
*/
{
	DWORD dwStatus = 0;
	BOOLEAN bResult = TRUE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;
	DWORD dwBytesNeeded = 0;
	SERVICE_STATUS_PROCESS* pBuffer = NULL;

	__try {

		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL) {
			_tprintf_s(TEXT("[-] Error GetSCMHandle\n"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
		hService = OpenService(hSCManager, szServiceName, SERVICE_QUERY_STATUS);
		if (hService == NULL) {
			_tprintf_s(TEXT("[-] Error OpenService\n"));
			__leave;
		}

		//
		// Get configuration information
		//
		
		// Query data size (first read)
		bResult = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, NULL, 0, &dwBytesNeeded);
		if (bResult == NULL && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			_tprintf_s(TEXT("[-] Error QueryServiceStatusEx\n"));
			__leave;
		}

		// Allocate memory
		pBuffer = (SERVICE_STATUS_PROCESS*)calloc(dwBytesNeeded, 1);
		if (pBuffer == NULL) {
			_tprintf_s(TEXT("[-] Error cannot allocate memory\n"));
			__leave;
		}

		// Get data (second read)
		bResult = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)pBuffer, dwBytesNeeded, &dwBytesNeeded);
		if (!bResult) {
			_tprintf_s(TEXT("[-] Error QueryServiceStatusEx\n"));
			__leave;
		}

		dwStatus = pBuffer->dwCurrentState;

	}
	__finally {

		if (pBuffer)
			free(pBuffer);

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return dwStatus;
	}
}

BOOLEAN U_CONTROLLER::LaunchService(_In_ PCWSTR szServiceName)
/**
 * @brief       Start the service "szServiceName".
 *
 * @param[in]   "szServiceName" - Name of the service
 *
 * @return      TRUE - No error occurs.
 *              FALSE - Error occurs.
*/
{
	BOOLEAN bResult = FALSE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;

	__try {
		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL)
		{
			_tprintf_s(TEXT("[-] Cannot get a handle to the SC Manager\n"));
			__leave;
		}

		//
		// Get Service handle, if the service is installed, we get the handle. 
		// Else we raise an error.
		//

		hService = _GetServiceHandle(hSCManager, szServiceName);
		if (hService == NULL)
		{
			_tprintf_s(TEXT("[-] Cannot get a handle to the service %s\n"), szServiceName);
			__leave;
		}

		// Start the service
		bResult = StartService(hService, 0, NULL);
		if (!bResult)
			U_HELPER::DisplayError(L"StartService");

	}
	__finally {

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return bResult;

	}
}

BOOLEAN U_CONTROLLER::StopService(_In_ PCWSTR szServiceName)
/**
 * @brief       Stop the service "szServiceName".
 *
 * @param[in]   "szServiceName" - Name of the service
 *
 * @return      TRUE - No error occurs.
 *              FALSE - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;
	SERVICE_STATUS scStatus = { 0 };

	__try {

		// Check the service is not already stopped
		DWORD scState = GetServiceState(szServiceName);
		if (scState == 0) {
			_tprintf_s(TEXT("[-] Error GetServiceState\n"));
			bResult = FALSE;
			__leave;
		}

		if (scState == SERVICE_STOPPED) {
			_tprintf_s(TEXT("[!] %s already stopped\n"), szServiceName);
			__leave;
		}
		else if (scState == SERVICE_STOP_PENDING) {
			_tprintf_s(TEXT("[!] %s stop pending...\n"), szServiceName);
			__leave;
		}


		//
		// Notify the driver that it should stop
		//

		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL) {
			_tprintf_s(TEXT("[-] Error GetSCMHandle\n"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
		hService = OpenService(hSCManager, szServiceName, SERVICE_STOP);
		if (hService == NULL) {
			_tprintf_s(TEXT("[-] Error OpenService\n"));
			__leave;
		}

		bResult = ControlService(hService, SERVICE_CONTROL_STOP, &scStatus);
		if (!bResult) {
			_tprintf_s(TEXT("[-] Error ControlService %d\n"), GetLastError());
			__leave;
		}

		while (scStatus.dwCurrentState != SERVICE_STOPPED && scStatus.dwCurrentState != SERVICE_STOP_PENDING) {
			Sleep(scStatus.dwWaitHint);

			scState = GetServiceState(szServiceName);
			if (scState == 0) {
				_tprintf_s(TEXT("[-] Error GetServiceState\n"));
				__leave;
			}
		}

		_tprintf_s(TEXT("[i] Service %s stopped successfully\n"), szServiceName);

	}
	__finally {

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return bResult;
	}
}

BOOLEAN U_CONTROLLER::QueryService(_In_ PCWSTR szServiceName)
/**
 * @brief       Query the service "szServiceName" configuration.
 *
 * @param[in]   "szServiceName" - Name of the service
 *
 * @return      TRUE - No error occurs.
 *              FALSE - Error occurs.
*/
{
	DWORD dwStatus = 0;
	BOOLEAN bResult = TRUE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;
	DWORD dwBytesNeeded = 0;
	SERVICE_STATUS_PROCESS* pBuffer = NULL;

	__try {
		_tprintf_s(TEXT("\033[1;4m%s status:\033[0m\n"), szServiceName);

		// Get SCM database handle
		hSCManager = _GetSCMHandle();
		if (hSCManager == NULL) {
			_tprintf_s(TEXT("[-] Error GetSCMHandle\n"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else, it is not
		hService = OpenService(hSCManager, szServiceName, SERVICE_QUERY_STATUS);
		if (hService == NULL) {
			_tprintf_s(TEXT("  Uninstalled\n"));
			__leave;
		}

		// Get configuration information

		// Query data size (first read)
		bResult = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, NULL, 0, &dwBytesNeeded);
		if (!bResult && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			_tprintf_s(TEXT("[-] Error QueryServiceStatusEx\n"));
			__leave;
		}

		// Allocate memory
		pBuffer = (SERVICE_STATUS_PROCESS*)calloc(dwBytesNeeded, 1);

		// Get data (second read)
		bResult = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)pBuffer, dwBytesNeeded, &dwBytesNeeded);
		if (!bResult || !pBuffer) {
			_tprintf_s(TEXT("[-] Error QueryServiceStatusEx\n"));
			__leave;
		}

		// Print service type
		_tprintf_s(TEXT("  Service Type       : %X"), pBuffer->dwServiceType);
		if (pBuffer->dwServiceType & SERVICE_FILE_SYSTEM_DRIVER) 
			_tprintf_s(TEXT(" FILE_SYSTEM_DRIVER"));
		if (pBuffer->dwServiceType & SERVICE_KERNEL_DRIVER) 
			_tprintf_s(TEXT(" KERNEL_DRIVER"));
		if (pBuffer->dwServiceType & SERVICE_WIN32_OWN_PROCESS) 
			_tprintf_s(TEXT(" WIN32_OWN_PROCESS"));
		if (pBuffer->dwServiceType & SERVICE_WIN32_SHARE_PROCESS) 
			_tprintf_s(TEXT(" WIN32_SHARE_PROCESS"));
		if (pBuffer->dwServiceType & SERVICE_INTERACTIVE_PROCESS) 
			_tprintf_s(TEXT(" INTERACTIVE_PROCESS"));
		_tprintf_s(TEXT("\n"));

		// Print current state
		_tprintf_s(TEXT("  Current State      : %d %s\n"), pBuffer->dwCurrentState,
			pBuffer->dwCurrentState == SERVICE_CONTINUE_PENDING ? L"CONTINUE_PENDING" :
			pBuffer->dwCurrentState == SERVICE_PAUSE_PENDING ? L"PAUSE_PENDING" :
			pBuffer->dwCurrentState == SERVICE_PAUSED ? L"PAUSED" :
			pBuffer->dwCurrentState == SERVICE_RUNNING ? L"RUNNING" :
			pBuffer->dwCurrentState == SERVICE_START_PENDING ? L"START_PENDING" :
			pBuffer->dwCurrentState == SERVICE_STOP_PENDING ? L"STOP_PENDING" :
			pBuffer->dwCurrentState == SERVICE_STOPPED ? L"STOPPED" :
			L"<UNKOWN>"
		);

		// Print accepted controls
		_tprintf_s(TEXT("  Controls Accepted  :"));
		// Non-exaustive list!
		if (pBuffer->dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) 
			_tprintf_s(TEXT(" PAUSABLE")); else _tprintf_s(TEXT(" NOT_PAUSABLE"));
		if (pBuffer->dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) 
			_tprintf_s(TEXT(" ACCEPTS_SHUTDOWN"));
		if (pBuffer->dwControlsAccepted & SERVICE_ACCEPT_STOP) 
			_tprintf_s(TEXT(" STOPABLE")); else _tprintf_s(TEXT(" NOT_STOPPABLE"));
		_tprintf_s(TEXT("\n"));

		// Print process ID
		_tprintf_s(TEXT("  PID                : "));
		if (pBuffer->dwCurrentState == SERVICE_STOPPED) 
			_tprintf_s(TEXT("-\n\n")); else _tprintf_s(TEXT("%d\n\n"), pBuffer->dwProcessId);

	}
	__finally {

		if (pBuffer)
			free(pBuffer);

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return bResult;
	}
}

BOOLEAN U_CONTROLLER::DelService(_In_ PCWSTR szServiceName)
/**
 * @brief       Delete the service "szServiceName".
 *
 * @param[in]   "szServiceName" - Name of the service
 *
 * @return      TRUE - No error occurs.
 *              FALSE - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;

	__try {
		// Get SCM database handle
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (hSCManager == INVALID_HANDLE_VALUE)
		{
			bResult = FALSE;
			_tprintf_s(TEXT("[-] Cannot get a handle to the SCM database\n"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
		hService = OpenService(
			hSCManager,	// Handle SC Manager
			szServiceName,	// Svc Name
			DELETE		// Delete Access
		);
		if (hService == INVALID_HANDLE_VALUE)
		{
			bResult = FALSE;
			_tprintf_s(TEXT("[-] Cannot OpenService %s\n"), szServiceName);
			__leave;
		}

		// Delete the service
		bResult = DeleteService(hService);
		if (!bResult)
		{
			_tprintf_s(TEXT("[-] Cannot delete %s\n"), szServiceName);
			__leave;
		}

		_tprintf_s(TEXT("[i] Service %s deleted succesfully\n"), szServiceName);

	}
	__finally {

		if (hSCManager)
			CloseServiceHandle(hSCManager);

		if (hService)
			CloseServiceHandle(hService);

		return bResult;

	}
}

VOID WINAPI U_CONTROLLER::SvcCtrlHandler(_In_ DWORD dwCtrl)
/**
 * @brief       Receive signal send by the User / SC Manager, handle it, update service status.
 *
 * @param[in]   "dwCtrl" - Service signal.
 *
 * @return      None.
*/
{
	if (dwCtrl == SERVICE_CONTROL_STOP)
	{
		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Stop event received"));

		// Close communication
		Communication.Disconnect();

		// Stop Driver:
		StopService(DRIVER_NAME);

		// Stop Service
		SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, SERVICE_STOP_PENDING, NO_ERROR, 0);
		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("STOP PENDING"));

		SetEvent(strctService.hSvcStopEvent);
		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("SET EVENT"));

		SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, strctService.SvcStatus.dwCurrentState, NO_ERROR, 0);
		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("stop"));
	}
}

VOID U_CONTROLLER::SvcReportStatus(_In_ SERVICE_STATUS SvcStatus, _In_ SERVICE_STATUS_HANDLE SvcStatusHandle, _In_ DWORD dwCurrentState, _In_ DWORD dwWin32ExitCode, _In_ DWORD dwWaitHint)
/**
 * @brief       Depending on the state of the service, it can receive order from the user space.
 *
 * @param[in]   "dwCurrentState"  - State of the service.
 * @param[in]   "dwWin32ExitCode" - Exit code.
 * @param[in]   "dwCurrentState"  - State of the service.
 * @param[in]   "dwWaitHint"      - Max Waiting time before action occur.
 *
 * @return      None.
*/
{
	static DWORD dwCheckPoint = 1;

	__try {

		SvcStatus.dwCurrentState = dwCurrentState;
		SvcStatus.dwWin32ExitCode = dwWin32ExitCode;
		SvcStatus.dwWaitHint = dwWaitHint;


		if (dwCurrentState == SERVICE_START_PENDING)
			SvcStatus.dwControlsAccepted = 0;			// Init the "dwControlsAccepted"
		else
			SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;	// If the service isn't START_PENDING it can retrieve a STOP from the user

		if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
			SvcStatus.dwCheckPoint = 0;				// Init the "dwCheckPoint"
		else
			SvcStatus.dwCheckPoint = dwCheckPoint++;		// Increments periodically the progress during a lengthy start, stop, pause or continue operation

		SetServiceStatus(SvcStatusHandle, &SvcStatus);

	}
	__finally {

	}
}
