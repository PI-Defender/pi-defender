/**
 * @file		Utilitaire.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

#include "Utilitaire.hpp"

//
// Print
//

VOID PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...)
{
	if (!lpszFormat || !lpszFormat[0]) return;

	LPCTSTR lpszPrefix = TEXT("");
	LPVOID lpBuf = nullptr;
	va_list list, copy;
	int nSize = 0;

	__try {

		va_start(list, lpszFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(lpszFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) __leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, lpszFormat, list);

		switch (msgType) {
		case MESSAGE_TYPE::TYPE_ERROR:
			lpszPrefix = TEXT("\n\033[31m[-] ");
			break;
		case MESSAGE_TYPE::TYPE_SUCCESS:
			lpszPrefix = TEXT("\033[32m[+] ");
			break;
		default:
			break;
		}

		_tprintf_s(TEXT("%s%s\033[0m\n"), lpszPrefix, (LPCTSTR)lpBuf);
	}

	__finally {
		va_end(list);
		if (lpBuf) LocalFree(lpBuf);
	}
}

VOID DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...)
{
	if (!lpszFormat || !lpszFormat[0]) return;
	
	int nSize = 0;
	va_list list, copy;
	LPVOID lpBuf = nullptr;
	LPCTSTR lpszPrefix = TEXT("");
	LONG lIcon = 0L;

	__try {

		va_start(list, lpszFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(lpszFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) __leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, lpszFormat, list);

		switch (msgType)
		{
		case MESSAGE_TYPE::TYPE_ERROR:
			lpszPrefix = TEXT("\033[31m[-] ");
			lIcon = MB_ICONERROR;
			break;
		case MESSAGE_TYPE::TYPE_SUCCESS:
			lpszPrefix = TEXT("\033[32m[+] ");
			lIcon = MB_ICONINFORMATION;
			break;
		default:
			break;
		}
		
		PrintMessage(msgType, (LPCTSTR)lpBuf);
		MessageBox(NULL, (LPCTSTR)lpBuf, TEXT("Error"), MB_OK | lIcon);
	}

	__finally {
		va_end(list);
		if (lpBuf) LocalFree(lpBuf);
	}
}

VOID DisplayError(_In_ LPCTSTR lpszFunction)
{
	LPVOID lpMsgBuf = nullptr;
	LPVOID lpDisplayBuf = nullptr;
	DWORD dwLastErrorCode = GetLastError();

	__try {
		if (!FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwLastErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		)) {
			PrintMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("FormatMessage failed with error 0x%X"), GetLastError());
			__leave;
		}

		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s failed with error 0x%X\n%s"), lpszFunction, dwLastErrorCode, (LPCTSTR)lpMsgBuf);
	}

	__finally {
		if (lpMsgBuf) LocalFree(lpMsgBuf);
	}
}

VOID FormatAccessMask(_In_ DWORD dwMask, _In_ SIZE_T szFormatSize, _Out_ LPTSTR lpszFormat)
{
	if (dwMask == 0) StringCchCopy(lpszFormat, szFormatSize, TEXT("<NULL>\n"));
	else *lpszFormat = L'\0';

	// Standard access rights
	if (dwMask & DELETE) StringCchCat(lpszFormat, szFormatSize, TEXT("DELETE\n"));
	if (dwMask & READ_CONTROL) StringCchCat(lpszFormat, szFormatSize, TEXT("READ_CONTROL\n"));
	if (dwMask & WRITE_DAC) StringCchCat(lpszFormat, szFormatSize, TEXT("WRITE_DAC\n"));
	if (dwMask & WRITE_OWNER) StringCchCat(lpszFormat, szFormatSize, TEXT("WRITE_OWNER\n"));
	if (dwMask & SYNCHRONIZE) StringCchCat(lpszFormat, szFormatSize, TEXT("SYNCHRONIZE\n"));

	// Process-specific access rights
	if (dwMask & PROCESS_TERMINATE) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_TERMINATE\n"));
	if (dwMask & PROCESS_CREATE_THREAD) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_CREATE_THREAD\n"));
	if (dwMask & PROCESS_SET_SESSIONID) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_SET_SESSIONID\n"));
	if (dwMask & PROCESS_VM_OPERATION) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_VM_OPERATION\n"));
	if (dwMask & PROCESS_VM_READ) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_VM_READ\n"));
	if (dwMask & PROCESS_VM_WRITE) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_VM_WRITE\n"));
	if (dwMask & PROCESS_DUP_HANDLE) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_DUP_HANDLE\n"));
	if (dwMask & PROCESS_CREATE_PROCESS) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_CREATE_PROCESS\n"));
	if (dwMask & PROCESS_SET_QUOTA) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_SET_QUOTA\n"));
	if (dwMask & PROCESS_SET_INFORMATION) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_SET_INFORMATION\n"));
	if (dwMask & PROCESS_QUERY_INFORMATION) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_QUERY_INFORMATION\n"));
	if (dwMask & PROCESS_SUSPEND_RESUME) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_SUSPEND_RESUME\n"));
	if (dwMask & PROCESS_QUERY_LIMITED_INFORMATION) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_QUERY_LIMITED_INFORMATION\n"));
	if (dwMask & PROCESS_SET_LIMITED_INFORMATION) StringCchCat(lpszFormat, szFormatSize, TEXT("PROCESS_SET_LIMITED_INFORMATION\n"));

	// Generic access rights
	if (dwMask & GENERIC_ALL) StringCchCat(lpszFormat, szFormatSize, TEXT("GENERIC_ALL\n"));
	if (dwMask & GENERIC_EXECUTE) StringCchCat(lpszFormat, szFormatSize, TEXT("GENERIC_EXECUTE\n"));
	if (dwMask & GENERIC_WRITE) StringCchCat(lpszFormat, szFormatSize, TEXT("GENERIC_WRITE\n"));
	if (dwMask & GENERIC_READ) StringCchCat(lpszFormat, szFormatSize, TEXT("GENERIC_READ\n"));

	// Remove trailing L'\n'
	lpszFormat[_tcslen(lpszFormat) - 1] = L'\0';
}


//
// Service
//

SC_HANDLE GetSCMHandle()
/*
Description:
	Get a handle to the Service Controller Database

Parameters:
	None

Return:
	SC_HANDLE - No error occurs.
	NULL - Error occurs.
*/
{
	SC_HANDLE schSCManager;

	// Get SCM database handle
	schSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, GENERIC_READ);
	if (!schSCManager) DisplayError(TEXT("OpenSCHManager"));

	return schSCManager;
}

DWORD GetServiceState(_In_ LPCTSTR lpszServiceName)
/*
Description:
	Query the service "lpszServiceName" and return its state

Parameters:
	lpszServiceName - Name of the service

Return:
	serviceStatus.dwCurrentState - "lpszServieceName" current state
	0 - Error
*/
{
	DWORD dwStatus = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS serviceStatus;

	__try {
		// Get SCM database handle
		schSCManager = GetSCMHandle();
		if (!schSCManager) {
			DisplayError(TEXT("GetSCMHandle"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
		schService = OpenService(schSCManager, lpszServiceName, SERVICE_QUERY_STATUS);
		if (!schService) {
			DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not installed."), lpszServiceName);
			__leave;
		}
		
		// Get service information
		if (!QueryServiceStatus(schService, &serviceStatus)) {
			DisplayError(TEXT("QueryServiceStatus"));
			__leave;
		}

		dwStatus = serviceStatus.dwCurrentState;
	}

	__finally {
		if (schService) CloseServiceHandle(schService);
		if (schSCManager) CloseServiceHandle(schSCManager);
	}

	return dwStatus;
}


//
// Process Handle & access rights
//

HANDLE GetProcessHandleByName(_In_ LPCTSTR lpszProcessName, _In_ DWORD dwDesiredAccess)
{
	HANDLE hProcess = NULL;
	HANDLE hSnapshot = NULL;
	BOOLEAN bResult = TRUE;

	PROCESSENTRY32 ProcessEntry = { 0 };

	__try {

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			DisplayError(TEXT("CreateToolhelp32Snapshot"));
			__leave;
		}

		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hSnapshot, &ProcessEntry)) {
			DisplayError(TEXT("Process32First"));
			__leave;
		}

		do {
			if (!_tcsicmp(ProcessEntry.szExeFile, lpszProcessName)) break;

			bResult = Process32Next(hSnapshot, &ProcessEntry);
		} while (bResult);

		if (!bResult) {
			DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not running."), lpszProcessName);
			__leave;
		}

		hProcess = OpenProcess(dwDesiredAccess, FALSE, ProcessEntry.th32ProcessID);
	}

	__finally {
		if (hSnapshot && hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
	}

	return hProcess;
}

_Success_(return) BOOLEAN GetProcessGrantedAccessRights(_In_ HANDLE hProcess, _In_ DWORD dwDesiredAccess, _Out_ DWORD* dwGrantedAccess)
/*
Description:
	Return the access mask associated with the handle of the process (hProcess)

Parameters:
	hProcess - handle to the targeted process
	dwDesiredAccess - Desired access mask of the handle
	dwGrantedAccess - (out) Pointer to DWORD, return granted access associated with the handle

Return:
	TRUE - No error occurs.
	FALSE - Error occurs.
*/
{
	PUBLIC_OBJECT_BASIC_INFORMATION objBasicInfo;
	
	if (!hProcess) return FALSE;
		
	if (!NT_SUCCESS(NtQueryObject(hProcess, ObjectBasicInformation, &objBasicInfo, sizeof(PUBLIC_OBJECT_BASIC_INFORMATION), NULL))) {
		DisplayError(TEXT("NtQueryObject"));
		return FALSE;
	}
		
	*dwGrantedAccess = objBasicInfo.GrantedAccess;

	return TRUE;
}
