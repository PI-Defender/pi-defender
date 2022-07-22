/**
 * @file       U_Helper.cpp
 * @brief      Help function used by several .cpp files
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Helper.hpp"



_Success_(return != FALSE) BOOLEAN U_HELPER::EnableVirtualTerminal(_Out_ HANDLE* hCon, _Out_ DWORD* dwOldMode)
/**
 * @brief        Enable the virtual terminal to be able to print text in color.
 *
 * @param[out]   "hCon"      - Virtual Terminal Handle
 * @param[out]   "dwOldMode" - Previous used terminal mode (backup)
 *
 * @return       TRUE - No error occurs.
 *               FALSE - An error occurs.
*/
{
	BOOLEAN bResult = FALSE;
	HANDLE hMyCon = INVALID_HANDLE_VALUE;
	DWORD dwMyDefaultMode;
	DWORD dwMode;

	__try {

		// Change console output to display Unicode characters
		if (_setmode(_fileno(stdout), _O_U16TEXT) == -1)
			__leave;

		// Backup default console output mode
		hMyCon = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hMyCon == INVALID_HANDLE_VALUE)
		{
			DisplayError(TEXT("GetStdHandle"));
			__leave;
		}

		if (!GetConsoleMode(hMyCon, &dwMyDefaultMode))
		{
			DisplayError(TEXT("GetConsoleMode"));
			__leave;
		}

		// Enable virtual terminal
		dwMode = dwMyDefaultMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(hMyCon, dwMode)) {
			DisplayError(TEXT("SetConsoleMode"));
			__leave;
		}

		*dwOldMode = dwMyDefaultMode;
		*hCon = hMyCon;
		bResult = TRUE;

	}
	__finally {

		return bResult;

	}
}

VOID U_HELPER::WelcomeMessage()
/**
 * @brief   Welcome banner message.
 *
 * @param   None.
 *
 * @return  None.
*/
{
	_tprintf_s(TEXT("\n\
\033[22;36m___  _    ___  ____ ____ ____ _  _ ___  ____ ____\n\
|__] | __ |  \\ |___ |___ |___ |\\ | |  \\ |___ |__/\n\
|    |    |__/ |___ |    |___ | \\| |__/ |___ | \\\033[0m\n\
\n\
\033[1;30mPI-Defender Tool - Copyright (C) NAVAL-Group\n\
Authors Nicolas JALLET - Bérenger BRAULT\n\
Version 1.0\033[0m\n\n"));
}

VOID U_HELPER::PrintUsage()
/**
 * @brief   Message showing the possible arguments that the program can handle.
 *
 * @param   None.
 *
 * @return  None.

*/
{
    _tprintf_s(TEXT("\
\033[1;4mDescription:\033[0m\n\
	PI-Defender service purpose is to start the PI-Defender driver\n\tand check if the file provided by the driver is signed.\n\
\033[1;4mUsage:\033[0m\n\
	.\\PI-Defender_UM.exe \033[4minstall\033[24m|\033[4mstart\033[24m|\033[4mquery\033[24m|\033[4mstop\033[24m|\033[4mdelete\033[0m\n"));
}

_Success_(return != FALSE) BOOLEAN U_HELPER::GetCurrentFolder(_Inout_ PWSTR szFolder)
/**
 * @brief         Get the current folder of the binary responsible for the launch of the service.
 *
 * @param[inout]  "szFolder" - DOS Path of the folder containing the binary responsible for the launch of the service.
 *
 * @return        TRUE - No error occurs.
 *                FALSE - An error occurs.
*/
{
	DWORD dwStatus = 0;
	BOOLEAN bResult = FALSE;

	__try {

		dwStatus = GetModuleFileName(NULL, szFolder, MAX_PATH);
		if (dwStatus == 0)
		{
			U_LOGS::SvcReportEvent(TEXT("GetModuleFileName"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Error retrieving the full executable path"));
			__leave;
		}

		dwStatus = PathRemoveFileSpec(szFolder);
		if (dwStatus == 0)
		{
			U_LOGS::SvcReportEvent(TEXT("PathRemoveFileSpec"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Error trying to remove the name from the full executable path"));
			__leave;
		}

		U_LOGS::SvcReportEvent(TEXT("GetExecutablePathFolder"), EVENTLOG_INFORMATION_TYPE, TASK_OK, TEXT("Retrieve folder of the executable"));
		bResult = TRUE;

	}
	__finally {

		return bResult;

	}
}

_Success_(return != FALSE) BOOLEAN U_HELPER::GetDllFullPath(_Out_ PWSTR szPath)
/**
 * @brief       Get the full DOS Path of the message DLL associated with the binary.
 *
 * @param[out]  "szPath" - DOS Path of the dll
 *
 * @return      TRUE - No error occurs.
 *              FALSE - An error occurs.
*/
{
	HRESULT hResult;
	BOOLEAN bResult = FALSE;

	__try {

		if (szPath == NULL) 
			__leave;

		bResult = GetCurrentFolder(szPath);
		if (!bResult)
		{
			U_LOGS::SvcReportEvent(TEXT("GetCurrentFolder"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Error trying to get the current folder of the executable"));
			__leave;
		}

		hResult = PathCchAppend(szPath, MAX_PATH, DLL_NAME);
		if (FAILED(hResult))
		{
			U_LOGS::SvcReportEvent(TEXT("PathAppend"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Error trying to append \"PI-Defender_UM_MsgFile.dll\" to the path"));
			__leave;
		}

		bResult = TRUE;

	}
	__finally {

		return bResult;

	}
}

_Success_(return != FALSE) BOOLEAN U_HELPER::GetHandleFromFileName(_Out_ HANDLE* hFileToCheck, _In_ PCWSTR szFileName)
/**
 * @brief       Get the handle to a file by providing it's name.
 *
 * @param[out]  "hFileToCheck" - Handle file return to the caller
 * @param[in]   "szFileName" - Name of the file which we need a handle to it.
 *
 * @return      TRUE - No error occurs.
 *              FALSE - An error occurs.
*/
{
	BOOLEAN bStatus = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	__try {

		// Open file (DOS path) (Read-only)
		hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			_tprintf_s(TEXT("\t[-]Erreur CreateFile : %d\n"), GetLastError());
			__leave;
		}

		// Return valid handle
		*hFileToCheck = hFile;

		bStatus = TRUE;

	}
	__finally {
		
		// No free(Path)         --> returned to caller (out parameter)
		// No CloseHandle(hFile) --> returned to caller (out parameter)
		
		return bStatus;

	}
}

VOID U_HELPER::DisplayError(_In_ PCWSTR szFunction)
/**
 * @brief      This routine will display an error message based off of the Win32 error
 *             code that is passed in. This allows the user to see an understandable
 *             error message instead of just the code.
 *
 * @param[in]  "szFunction" - Name of function which failed.
 *
 * @return     None.
*/
{
	LPVOID lpMsgBuf = NULL;
	LPVOID lpDisplayBuf = NULL;
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

		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s failed with error 0x%X\n%s"), szFunction, dwLastErrorCode, (LPCTSTR)lpMsgBuf);

	}
	__finally {

		if (lpMsgBuf) 
			LocalFree(lpMsgBuf);

	}
}

VOID U_HELPER::PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR szFormat, ...)
/**
 * @brief      Print error message in the terminal.
 *
 * @param[in]  "msgType"  - Type of the associated message (warning, error, etc.)
 * @param[in]  "szFormat" - Formatted string litteral
 *
 * @return     None.
*/
{
	if (!szFormat || !szFormat[0]) 
		return;

	LPCTSTR lpszPrefix = TEXT("");
	LPVOID lpBuf = NULL;
	va_list list, copy;
	int nSize = 0;

	__try {

		va_start(list, szFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(szFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) 
			__leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, szFormat, list);

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
		if (lpBuf) 
			LocalFree(lpBuf);

	}
}

VOID U_HELPER::DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR szFormat, ...)
/**
 * @brief      Popup error message displaying the function that failed associated with the error code.
 *
 * @param[in]  "msgType"  - Type of the associated message (warning, error, etc.)
 * @param[in]  "szFormat" - Formatted string litteral
 *
 * @return None.
*/
{
	if (!szFormat || !szFormat[0]) 
		return;

	int nSize = 0;
	va_list list, copy;
	LPVOID lpBuf = NULL;
	LPCTSTR lpszPrefix = TEXT("");
	LONG lIcon = 0L;

	__try {

		va_start(list, szFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(szFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) __leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, szFormat, list);

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
		if (lpBuf) 
			LocalFree(lpBuf);

	}
}

ULONG U_HELPER::CountWStringsInBuffer(_In_ PCWSTR szBuffer, _In_ const ULONG ulSize)
/**
 * @brief     Returns the number of Unicode strings found in a buffer (raw data from REG_MULTI_SZ)
 *
 * @param[in]  "szBuffer" - pointer to a buffer that contains raw data from a REG_MULTI_SZ
 * @param[in]  "ulSize"   - szBuffer size in bytes
 *
 * @return     None.
*/
{
	int count = 0;
	BOOLEAN hasAtLeastOneChar = FALSE;

	for (ULONG i = 0; i < ulSize / sizeof(WCHAR); i++) {
		if (L'\0' == *(WCHAR*)&szBuffer[i]) {
			if (hasAtLeastOneChar) {
				hasAtLeastOneChar = FALSE;
				count++;
			}
			else if (i > 0) {  // and !hasAtLeastOneChar
				break;     // stop here (two '\0' in a row)
			}
		}
		else {
			hasAtLeastOneChar = TRUE;
		}
	}

	return count;
}

_Success_(return != INVALID_HANDLE_VALUE && return != NULL) HANDLE U_HELPER::GetProcessHandleByName(_In_ PWSTR szProcessName)
/**
 * @brief      Return the handle of the process name provided in parameter ('szProcessName')
 *
 * @param[in]  "szProcessName" - Name of the process we need the handle.
 *
 * @return     Valid Handle   - Handle of the process
 *             Invalid Handle - Error occurs.
*/
{
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	HANDLE hSnapshot = INVALID_HANDLE_VALUE;
	BOOLEAN bResult = TRUE;

	PROCESSENTRY32 ProcessEntry = { 0 };

	PathStripPath(szProcessName);

	__try {

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
		{
			_tprintf_s(TEXT("Snapshot erreur\n"));
			__leave;
		}

		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		bResult = Process32First(hSnapshot, &ProcessEntry);
		if (!bResult)
		{
			_tprintf_s(TEXT("Process32First erreur\n"));
			__leave;
		}

		while (Process32Next(hSnapshot, &ProcessEntry))
		{
			if (_tcsicmp(ProcessEntry.szExeFile, szProcessName) == 0)
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessEntry.th32ProcessID);
				break;
			}
		}

	}
	__finally {

		return hProcess;

	}
}
