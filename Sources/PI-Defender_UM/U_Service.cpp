/**
 * @file       U_Service.cpp
 * @brief      Entrypoint of the process & the service
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Service.hpp"


U_COMMUNICATION Communication;	// Used by controller to disconnect the client.
sSERVICE strctService = { 0 };	// Used by controller to send event to Service Controller


BOOLEAN Install()
/**
 * @brief   Wrap the installation of the service
 *
 * @param   None.
 *
 * @return  TRUE  - No error occurs.
 *          FALSE - Error occurs.
*/
{
	LSTATUS lResult = 0;
	BOOLEAN bResult = TRUE;
	BOOLEAN bTotalResult = TRUE;

	//
	// Driver Installation
	//

	__try {

		// Check if service already installed
		bResult = U_CONTROLLER::IsServiceInstalled(DRIVER_NAME);
		if (bResult)
		{
			_tprintf_s(TEXT("[!] Service %s already installed\n"), DRIVER_NAME);
			__leave;
		}

		// If not, install it

		// Create Registry keys
		lResult = U_CONFIGURATION::SetDriverConfiguration();
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to create keys for HKLM\\SYSTEM\\CurrentControlSet\\Services\\PI-Defender"));
			__leave;
		}
		else {
			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Registry Keys created for HKLM\\SYSTEM\\CurrentControlSet\\Services\\PI-Defender"));
		}

		// Install service
		bResult = U_CONTROLLER::InstallService(DRIVER_NAME, TRUE);
		if (!bResult)
		{
			_tprintf_s(TEXT("[-] Error installing %s\n"), DRIVER_NAME);
			__leave;
		}

		_tprintf_s(TEXT("[i] Service %s installed succesffully\n"), DRIVER_NAME);

	}
	__finally{

		bTotalResult &= bResult; // Boolean AND operation

	}


	//
	// User-mode Service Installation
	//

	__try{

		// Check if service already installed
		bResult = U_CONTROLLER::IsServiceInstalled(SERVICE_NAME);
		if (bResult)
		{
			_tprintf_s(TEXT("[!] Service %s already installed\n"), SERVICE_NAME);
			__leave;
		}

		// If not, install it
		
		// Create Registry keys
		lResult = U_CONFIGURATION::SetServiceConfiguration();
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to create keys for HKLM\\SYSTEM\\CurrentControlSet\\Services\\PI-Defender_UM"));
			__leave;
		}
		else {
			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Registry Keys created for HKLM\\SYSTEM\\CurrentControlSet\\Services\\PI-Defender_UM"));
		}

		// Install service
		bResult = U_CONTROLLER::InstallService(SERVICE_NAME, FALSE);
		if (!bResult)
		{
			_tprintf_s(TEXT("[-] Error installing %s\n"), SERVICE_NAME);
			__leave;
		}

		_tprintf_s(TEXT("[i] Service %s installed succesffully\n"), SERVICE_NAME);

	}
	__finally {

		bTotalResult &= bResult; // Boolean AND operation
		return bTotalResult;

	}
}


BOOLEAN Query()
/**
 * @brief   Wrap the query information about the service
 *
 * @param   None.
 *
 * @return  TRUE - No error occurs.
 *          FALSE - Error occurs.
*/
{
	if (U_CONTROLLER::IsServiceInstalled(DRIVER_NAME))
	{
		U_CONFIGURATION::PrintConfiguration();
		_tprintf_s(TEXT("\n\n"));
	}
	
	return U_CONTROLLER::QueryService(DRIVER_NAME) & U_CONTROLLER::QueryService(SERVICE_NAME);
}


BOOLEAN Delete()
/**
 * @brief   Wrap the deletion of the driver and the service.
 *          [i] If an error occur when trying to delete the driver, the service will not be deleted.
 * 
 * @param   None.
 *
 * @return  TRUE - No error occurs.
 *          FALSE - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;

	if (U_CONFIGURATION::DeleteLogsRegistry() != ERROR_SUCCESS) {
		_tprintf_s(TEXT("[-] Error deletting registry keys\n"));
		bResult = FALSE;
	}
	
	if (!U_CONTROLLER::DelService(SERVICE_NAME))
		bResult = FALSE;

	if (!U_CONTROLLER::DelService(DRIVER_NAME))
		bResult = FALSE;

	return bResult;
}


BOOLEAN Start()
/**
 * @brief   Wrap the start of the service
 *
 * @param   one.
 *
 * @return  TRUE  - No error occurs.
 *          FALSE - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;

	__try {

		bResult = U_CONTROLLER::LaunchService(SERVICE_NAME);
		if (!bResult)
		{
			_tprintf_s(TEXT("[-] Couldn't start %s\n"), SERVICE_NAME);
			__leave;
		}

		_tprintf_s(TEXT("[i] Starting %s\n\n"), SERVICE_NAME);

		U_CONFIGURATION::PrintConfiguration();

	}
	__finally {

		return bResult;

	}
}


BOOLEAN Stop()
/**
 * @brief   Wrap the stop of the driver and the service.\n
 *          [i] If the driver failed to stop, the service isn't stopped.
 * 
 * @param   None.
 *
 * @return  TRUE  - No error occurs.
 *          FALSE - Error occurs.
*/
{
	return U_CONTROLLER::StopService(DRIVER_NAME) & U_CONTROLLER::StopService(SERVICE_NAME);
}


VOID Service()
/**
 * @brief   Wrap the call to the service entrypoint.
 *
 * @param   None.
 *
 * @return  None.
*/
{
	BOOLEAN bResult;
	
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ (PWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
		{ NULL, NULL }
	};

	// Dispatcher used to start the service
	bResult = StartServiceCtrlDispatcher(DispatchTable);
	if (!bResult)
		U_LOGS::SvcReportEvent(TEXT("StartServiceCtrlDispatcher"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("The service didn't start"));

}


int _tmain(_In_ int argc, _In_ TCHAR* argv[])
/**
 * @brief      Entrypoint of the user space executable.
 *
 * @param[in]  "argc" - Number of arguments.
 * @param[in]  "argv" - Array that contains arguments values.
 *
 * @return     EXIT_SUCCESS (0) - No error occurs.
 *             EXIT_FAILURE (1) - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;
	HANDLE hCon = INVALID_HANDLE_VALUE;
	DWORD dwDefaultMode = 0;

	__try {

		// Initialize logs
		U_LOGS::Initialize(SERVICE_NAME);
		
		// Install the driver and the service
		if (argc == 2 && !lstrcmpi(argv[1], TEXT("install")))
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			Install();
		}

		// Query information about the service
		else if (argc == 2 && !lstrcmpi(argv[1], TEXT("query")))
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			Query();
		}

		// Delete the service
		else if (argc == 2 && !lstrcmpi(argv[1], TEXT("delete")))
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			Delete();
		}

		// Start the service
		else if (argc == 2 && !lstrcmpi(argv[1], TEXT("start")))
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			Start();
		}

		// Stop the service
		else if (argc == 2 && !lstrcmpi(argv[1], TEXT("stop")))
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			Stop();
		}

		// Service
		else if (argc == 2 && !lstrcmpi(argv[1], TEXT("service")))
		{
			// No need for virtual terminal and fancy colors in service
			Service();
		}

		// Print usage
		else
		{
			U_HELPER::EnableVirtualTerminal(&hCon, &dwDefaultMode);
			U_HELPER::WelcomeMessage();
			U_HELPER::PrintUsage();
		}

	}
	__finally {

		if (hCon != INVALID_HANDLE_VALUE && bResult) {
			if (!SetConsoleMode(hCon, dwDefaultMode)) {
				U_HELPER::DisplayError(TEXT("SetConsoleMode"));
				bResult = FALSE;
			}
		}

		// Unload logs
		U_LOGS::Unload();

		return bResult ? EXIT_SUCCESS : EXIT_FAILURE;

	}
}

VOID WINAPI SvcMain(_In_ DWORD dwArgc, _In_ PWSTR* pszArgv)
/**
 * @brief       Entrypoint and core of the user space service.
 *
 * @param[in]   "dwArgc"   - Number of arguments
 * @param[out]  "lpszArgv" - Array that contains arguments values
 *
 * @return      None.
*/
{
	BOOLEAN bResult = FALSE;
	HRESULT hResult = S_OK;

	HANDLE* hThreads = NULL;
	HANDLE* hWorkerThreads = NULL;
	U_COMMUNICATION::THREAD_PARAMETERS Parameters = { 0 };
	HANDLE hPort = INVALID_HANDLE_VALUE;
	HANDLE hCompletion = INVALID_HANDLE_VALUE;
	DWORD i = 0;

	__try {
		// Register service status handle
		strctService.hSvcStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)U_CONTROLLER::SvcCtrlHandler);
		if (strctService.hSvcStatus == NULL)
		{
			U_LOGS::SvcReportEvent(TEXT("RegisterServiceCtrlHandler"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to register an handler to control the service"));
			__leave;
		}

		// Report Initial status to the SCM
		strctService.SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		strctService.SvcStatus.dwServiceSpecificExitCode = 0;
		U_CONTROLLER::SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, SERVICE_START_PENDING, NO_ERROR, 3000);

		// Create a stop event for whenever the user decide to send a stop signal
		strctService.hSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // Stop Event
		if (strctService.hSvcStopEvent == NULL)
		{
			U_CONTROLLER::SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, SERVICE_STOP, NO_ERROR, 0);
			__leave;
		}

		// Report running status to the SCM
		U_CONTROLLER::SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, SERVICE_RUNNING, NO_ERROR, 0);
		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Running"));

		// Start the driver
		U_CONTROLLER::LaunchService(DRIVER_NAME);
		U_LOGS::SvcReportEvent(DRIVER_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Running"));

		// Init Communication
		hResult = Communication.Connect((PWSTR)U_CONFIGURATION::GetCommunicationPort(), U_CONFIGURATION::GetListenerThreads(), &hCompletion, &hPort);
		if (IS_ERROR(hResult)) {
			U_LOGS::SvcReportEvent(TEXT("InitializeCommunicationWithDriver"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to initialize the connection to the server"));
			bResult = FALSE;
			__leave;
		}
		U_LOGS::SvcReportEvent(TEXT("Connect"), EVENTLOG_INFORMATION_TYPE, TASK_OK, TEXT("Connection to the driver"));

		// Create Listening thread
		hThreads = (HANDLE*)calloc(U_CONFIGURATION::GetListenerThreads(), sizeof(HANDLE));
		if (hThreads == NULL)
		{
			U_LOGS::SvcReportEvent(TEXT("calloc"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to create the dynamic handle array"));
			__leave;
		}

		Parameters.hCompletion = hCompletion;
		Parameters.hPort = hPort;
		Parameters.dwMinWorkerThreads = U_CONFIGURATION::GetMinWorkerThreads();
		Parameters.dwMaxWorkerThreads = U_CONFIGURATION::GetMaxWorkerThreads();

		// Listening threads
		for (i = 0; i < U_CONFIGURATION::GetListenerThreads(); i++)
		{
			*(hThreads + i) = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)Communication.ListenerThread,
				&Parameters,
				0,
				NULL
			);

			if (*(hThreads + i) == NULL)
			{
				U_LOGS::SvcReportEvent(TEXT("CreateThread"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to create listening thread"));
				__leave;
			}

		}

		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Wait for threads to end..."));

		while (TRUE)
		{
			WaitForSingleObject(strctService.hSvcStopEvent, INFINITE);

			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Ending !"));

			U_CONTROLLER::SvcReportStatus(strctService.SvcStatus, strctService.hSvcStatus, SERVICE_STOPPED, NO_ERROR, 0);
		}

	}
	__finally {

		return;
	}
}
