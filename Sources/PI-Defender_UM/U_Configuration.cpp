/**
 * @file       U_Configuration.cpp
 * @brief      Set all the needed configuration for the driver and the user-mode service
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Configuration.hpp"


LSTATUS U_CONFIGURATION::SetServiceConfiguration()
/**
 * @brief   Create registry keys needed for the service configuration.
 *
 * @param   None.
 *
 * @return   ERROR_SUCCESS - No error occurs.
 *           Other status  - Error status.
*/
{
	LSTATUS lResult = 0;
	
	__try {
		
		// Configure logging in ETW with a registry key
		lResult = _CreateRegistryForLogs();
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Create PI-Defender_UM Parameter key
		lResult = U_REGISTRY::CreateRegistryKey(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE);
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Set Listener Threads key for the service
		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_LISTENER_THREADS_KEY, LISTENER_THREADS_REG_DEFAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Set Min/Max Worker Threads for the service
		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_MIN_WORKER_THREADS_KEY, MIN_WORKER_THREADS_REG_DEFAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;
		
		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_MAX_WORKER_THREADS_KEY, MAX_WORKER_THREADS_REG_DEFAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;

	}

	__finally {

		return lResult;

	}
}

LSTATUS U_CONFIGURATION::SetDriverConfiguration()
/**
 * @brief   Create registry keys needed for the driver configuration.
 *
 * @param   None.
 *
 * @return  ERROR_SUCCESS - No error occurs.
 *          Other status  - Error status.
*/
{
	LSTATUS lResult = 0;
	TCHAR szWhitelistDefaultValue[MAX_PATH] = { 0 };
	TCHAR szTempString[MAX_PATH] = { 0 };
	SIZE_T blobSize = 0;

	__try {

		// Create PI-Defender Parameters key
		lResult = U_REGISTRY::CreateRegistryKey(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE);
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Set Communication Port key
		lResult = U_REGISTRY::WriteStringInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_COMMUNICATION_PORT_KEY, (PWSTR)COMMUNICATIONPORT_REG_DEFEAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Set Max Clients key
		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_MAX_CLIENTS_KEY, MAXCLIENTS_REG_DEFAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;

		//
		// Set Whitelist key
		//

		if (!SHGetSpecialFolderPath(NULL, szTempString, CSIDL_WINDOWS, FALSE))
		{
			lResult = ERROR_CAN_NOT_COMPLETE;
			__leave;
		}

		blobSize += (_tcslen(szTempString) + 1) * sizeof(TCHAR);
		RtlCopyMemory(szWhitelistDefaultValue, szTempString, blobSize);
		RtlZeroMemory(szTempString, blobSize);

		if (!SHGetSpecialFolderPath(NULL, szTempString, CSIDL_PROGRAM_FILES, FALSE))
		{
			lResult = ERROR_CAN_NOT_COMPLETE;
			__leave;
		}

		if (FAILED(PathCchAppendEx(szTempString, MAX_PATH, TEXT("Windows Defender\\MsMpEng.exe"), NULL)))
		{
			lResult = ERROR_CAN_NOT_COMPLETE;
			__leave;
		}

		RtlCopyMemory(szWhitelistDefaultValue + blobSize / sizeof(TCHAR), szTempString, (_tcslen(szTempString) + 1) * sizeof(TCHAR));
		blobSize += (_tcslen(szTempString) + 1) * sizeof(TCHAR);

		blobSize += sizeof(TCHAR); // terminating null-byte (second one)

		lResult = U_REGISTRY::WriteMultiStringInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_WHITELIST_KEY, (const BYTE*)szWhitelistDefaultValue, (DWORD)blobSize);
		if (lResult != ERROR_SUCCESS)
			__leave;

		// Set Cache Size
		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_CACHE_SIZE_KEY, CACHE_SIZE_REG_DEFAULT_VALUE);
		if (lResult != ERROR_SUCCESS)
			__leave;

	}

	__finally {

		return lResult;
	
	}
}

VOID U_CONFIGURATION::PrintConfiguration()
/**
 * @brief   Print and populate service structure with the configuration read from registry keys.\n
 *             - Max number of clients
 *             - Communication Port
 *             - Whitelist
 *             - Number of threads of the service
 *
 * @param   None.
 *
 * @return  None.
*/
{
	PCWSTR szPort = NULL;
	DWORD dwCount = 0;
	PWHITELIST pWhitelist = nullptr;

	_tprintf_s(TEXT("\033[1;4m%s configuration:\033[0m\n"), DRIVER_NAME);

	// Max Clients
	dwCount = GetMaxClients();
	if (dwCount == 0) 
		_tprintf_s(TEXT("[!] Error getting \"%s\" registry value\n"), U_CONFIGURATION_MAX_CLIENTS_KEY);
	else 
		_tprintf_s(TEXT("  Max Clients: \033[32m%d\033[0m\n"), dwCount);


	// Communication Port
	szPort = GetCommunicationPort();
	if (szPort == L"") 
		_tprintf_s(TEXT("[!] Error getting \"%s\" registry value\n"), U_CONFIGURATION_COMMUNICATION_PORT_KEY);
	else 
		_tprintf_s(TEXT("  CommunicationPort: \033[32m%ls\033[0m\n"), szPort);


	// Whitelist
	pWhitelist = GetWhitelist();
	if (pWhitelist == NULL)
	{
		_tprintf_s(TEXT("[!] Error getting \"%s\" registry value\n"), U_CONFIGURATION_WHITELIST_KEY);
	}
	else {
		_tprintf_s(TEXT("  %s:\n"), U_CONFIGURATION_WHITELIST_KEY);

		if (pWhitelist->iLength == 0) 
			_tprintf_s(TEXT("    \033[33m<NULL>\033[0m\n"));
		else 
			for (int i = 0; i < pWhitelist->iLength; i++) 
				_tprintf_s(TEXT("    [%d] \033[32m%ls\033[0m\n"), i, pWhitelist->szBuffer[i]);
	}

	// Cache Size
	dwCount = GetCacheSize();
	if (dwCount == 0) 
		_tprintf_s(TEXT("  Error getting \"%s\" registry value\n"), U_CONFIGURATION_CACHE_SIZE_KEY);
	else 
		_tprintf_s(TEXT("  Cache Size: \033[32m%d\033[0m\n"), dwCount);


	_tprintf_s(TEXT("\n\033[1;4m%s configuration:\033[0m\n"), SERVICE_NAME);

	// Listening Threads
	dwCount= GetListenerThreads();
	if (dwCount == 0) 
		_tprintf_s(TEXT("  Error getting \"%s\" registry value\n"), U_CONFIGURATION_LISTENER_THREADS_KEY);
	else 
		_tprintf_s(TEXT("  Number of Listening Threads: \033[32m%d\033[0m\n"), dwCount);


	// Min / Max Worker Threads
	dwCount = GetMinWorkerThreads();
	if (dwCount == 0) 
		_tprintf_s(TEXT("  Error getting \"%s\" registry value\n"), U_CONFIGURATION_MIN_WORKER_THREADS_KEY);
	else 
		_tprintf_s(TEXT("  Number of Worker Threads (Min): \033[32m%d\033[0m\n"), dwCount);

	dwCount = GetMaxWorkerThreads();
	if (dwCount == 0) 
		_tprintf_s(TEXT("  Error getting \"%s\" registry value\n"), U_CONFIGURATION_MAX_WORKER_THREADS_KEY);
	else 
		_tprintf_s(TEXT("  Number of Worker Threads (Max): \033[32m%d\033[0m\n"), dwCount);

	return;
}

LSTATUS U_CONFIGURATION::_CreateRegistryForLogs()
/**
 * @brief   Create all the registry keys needed for ETW Logs.
 *
 * @param   None.
 *
 * @return  ERROR_SUCCESS - No error occurs.
 *          Other status  - Error status.
*/
{
	LSTATUS lResult = 0;
	PWSTR szDllPath = NULL;

	__try {

		lResult = U_REGISTRY::CreateRegistryKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("CreateRegistryKey"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error creating the key HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;
		}

		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"), TEXT("CategoryCount"), 1);
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("WriteDwordInRegistry"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error creating the key CategoryCount in HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;

		}

		lResult = U_REGISTRY::WriteDwordInRegistry(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"), TEXT("TypesSupported"), 7);
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("WriteDwordInRegistry"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error creating the key TypesSupported in HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;

		}

		szDllPath = (PWSTR)calloc(MAX_PATH, sizeof(WCHAR));
		if (szDllPath == NULL) {
			lResult = ERROR_CAN_NOT_COMPLETE;
			_tprintf_s(TEXT("Error cannot allocate memory\n"));
			__leave;
		}

		if (!U_HELPER::GetDllFullPath(szDllPath))
		{
			lResult = ERROR_CAN_NOT_COMPLETE;
			U_LOGS::SvcReportEvent(TEXT("GetDllFullPath"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error retrieving the full path of the log message dll"));
			__leave;

		}

		lResult = U_REGISTRY::WriteStringInRegistry(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"), TEXT("CategoryMessageFile"), szDllPath);
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("WriteStringInRegistry"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error creating the key CategoryMessageFile in HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;

		}

		lResult = U_REGISTRY::WriteStringInRegistry(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"), TEXT("EventMessageFile"), szDllPath);
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("WriteStringInRegistry"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error creating the key EventMessageFile in HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;
		}

		U_LOGS::SvcReportEvent(TEXT("_CreateRegistryForLogs"), EVENTLOG_INFORMATION_TYPE, TASK_OK, TEXT("All the keys needed for ETW are created"));

	}
	__finally {

		if (szDllPath != NULL)
			free(szDllPath);

		return lResult;

	}
}

LSTATUS U_CONFIGURATION::DeleteLogsRegistry()
/**
 * @brief   Delete all the registry keys needed for ETW Logs.
 *          This function is called when the service UM is deleted.
 *
 * @param   None.
 *
 * @return  ERROR_SUCCESS - No error occurs.
 *          Other status - Error status.
*/
{
	LSTATUS lResult = 0 ;

	__try {

		lResult = RegDeleteTree(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
		if (lResult != ERROR_SUCCESS)
		{
			U_LOGS::SvcReportEvent(TEXT("RegDeleteTree"), EVENTLOG_ERROR_TYPE, TASK_ERROR,
				TEXT("Error deletting the key HKLM:SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\PI-Defender_UM"));
			__leave;
		}

		U_LOGS::SvcReportEvent(TEXT("DeleteLogsRegistry"), EVENTLOG_INFORMATION_TYPE, TASK_OK, TEXT("All the keys needed for ETW are deleted"));

	}
	__finally {

		return lResult;
	}

}

DWORD U_CONFIGURATION::GetListenerThreads()
/**
 * @brief   Get the number of threads of the service by reading the values in the Registry.
 *
 * @param   None.
 *
 * @return  dwThreads - Number of threads of the service.
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_LISTENER_THREADS_KEY, REG_DWORD, &pData);

	return *(DWORD*)pData;
}

DWORD U_CONFIGURATION::GetMinWorkerThreads()
/**
 * @brief   Get the number of threads of the service by reading the values in the Registry.
 *
 * @param   None.
 *
 * @return  dwMinThreads - Number of threads of the service.
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_MIN_WORKER_THREADS_KEY, REG_DWORD, &pData);

	return *(DWORD*)pData;
}

DWORD U_CONFIGURATION::GetMaxWorkerThreads()
/**
 * @brief   Get the number of threads of the service by reading the values in the REGISTRY::
 *
 * @param   None.
 *
 * @return  dwMaxThreads - Number of threads of the service.
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_SERVICE_HIVE, U_CONFIGURATION_MAX_WORKER_THREADS_KEY, REG_DWORD, &pData);

	return *(DWORD*)pData;
}

DWORD U_CONFIGURATION::GetMaxClients()
/**
 * @brief   Get the maximum number of clients of the service by reading the values in the REGISTRY::\n
 *				[i] This value is loaded but never used by the service (only for printing purpose). The driver will also load the key to setup the communication.
 *
 * @param   None.
 *
 * @return  dwMaxClients - Maximum number of clients allowed to connect to the driver. 
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_MAX_CLIENTS_KEY, REG_DWORD, &pData);

	return *(DWORD*)pData;
}

PCWSTR U_CONFIGURATION::GetCommunicationPort()
/**
 * @brief   Get the communication port used by the driver to interact with the service.
 *
 * @param   None.
 *
 * @return  szCommunicationPort - String containing the port used by the driver to initiate the communication.
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_COMMUNICATION_PORT_KEY, REG_SZ, &pData);

	return reinterpret_cast<PCWSTR>(pData);
}

PWHITELIST U_CONFIGURATION::GetWhitelist()
/**
 * @brief   Get the whitelist used to allow user-specified apps.
 *
 * @param   None.
 *
 * @return  pWhitelist - pointer to the loaded Whitelist (Don't forget to free at unload)
*/
{
	LSTATUS lResult = 0;
	HKEY hKey = NULL;
	DWORD dwLen = 0;
	PWSTR readBuffer = NULL;
	PWHITELIST pWhitelist = NULL;

	__try {

		// Get handle for Registry key
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, 0, KEY_QUERY_VALUE, &hKey);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error opening the key : 0x%x\n"), lResult);
			__leave;
		}

		// Query data size (first read)
		lResult = RegGetValue(hKey, NULL, U_CONFIGURATION_WHITELIST_KEY, RRF_RT_REG_MULTI_SZ, NULL, NULL, &dwLen);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error querying the value : 0x%x\n"), lResult);
			__leave;
		}

		// Allocate memory
		readBuffer = (PWSTR)calloc(dwLen, 1);
		if (readBuffer == NULL)
		{
			_tprintf_s(TEXT("Error allocating the buffer that will receive the value of the key\n"));
			__leave;
		}

		// Get data (second read)
		lResult = RegGetValue(hKey, NULL, U_CONFIGURATION_WHITELIST_KEY, RRF_RT_REG_MULTI_SZ, NULL, (PVOID)readBuffer, &dwLen);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error querying the value : 0x%x\n"), lResult);
			__leave;
		}


		// Allocate memory to hold Whitelist
		// /!\ DON'T FORGET TO FREE AFTER USE (at service unload)
		pWhitelist = (PWHITELIST)calloc(1, sizeof(WHITELIST));
		if (!pWhitelist) {
			_tprintf_s(TEXT("Error cannot allocate memory\n"));
			__leave;
		}

		// Save the number of strings (paths) to be whitelisted in "Whitelist.Length" attribute
		pWhitelist->iLength = U_HELPER::CountWStringsInBuffer(readBuffer, dwLen);
		if (pWhitelist->iLength == 0) {
			pWhitelist->szBuffer = NULL;
		}
		else {
			// Allocate memory and save a pointers array
			pWhitelist->szBuffer = (PCWSTR*)calloc(pWhitelist->iLength, sizeof(PCWSTR));
			if (!pWhitelist->szBuffer) {
				_tprintf_s(TEXT("[-] Error allocating memory\n"));
				__leave;
			}

			// This block parses raw data (REG_MUTLI_SZ format) from pInfo->Data and save extracted strings in Whitelist.Buffer
			// The REG_MULTI_SZ type is a sequence of null-terminated strings, terminated by a empty string (\0). ex: String1\0String2\0String3\0LastString\0\0
			SIZE_T pBegin = 0;
			SIZE_T pEnd = 0;
			for (int i = 0; i < pWhitelist->iLength; i++) {
				while (pEnd < dwLen) {
					if (L'\0' == *(WCHAR*)&readBuffer[pEnd]) {
						pEnd++;
						break;
					}
					pEnd++;
				}

				// Allocate memory (this process is needed to have strings aligned on memory)
#pragma warning(disable : 6386)
				// Warning C6386 (Buffer overrun) is ok, because `i` goes from 0 to pWhitelist.iLength (previous calloc)
				pWhitelist->szBuffer[i] = (PCWSTR)calloc(pEnd - pBegin, sizeof(WCHAR));
#pragma warning(default : 6386)

				// Copy string
#pragma warning(disable : 6385)
				// Warning C6385 (Reading invalid data) is same as C6386 above
				memcpy_s((PVOID)pWhitelist->szBuffer[i], (pEnd - pBegin) * sizeof(WCHAR), &readBuffer[pBegin], (pEnd - pBegin) * sizeof(WCHAR));
#pragma warning(default : 6385)

				// Update pBegin
				pBegin = pEnd;

			}
		}

	}
	__finally {

		if (hKey)
			RegCloseKey(hKey);

		if (readBuffer)
			free((PVOID)readBuffer);

		if (!hKey)
			return NULL;

		if (lResult != ERROR_SUCCESS)
			return NULL;

		return pWhitelist;

	}
}

DWORD U_CONFIGURATION::GetCacheSize()
/**
 * @brief   Get the number of items to be kept in the cache by the filter.
 *
 * @param   None.
 *
 * @return  dwCacheSize - number of items to be kept in the cache by the filter.
*/
{
	PVOID pData = nullptr;

	U_REGISTRY::ReadValue(HKEY_LOCAL_MACHINE, U_CONFIGURATION_DRIVER_HIVE, U_CONFIGURATION_CACHE_SIZE_KEY, REG_DWORD, &pData);

	return *(DWORD*)pData;
}
