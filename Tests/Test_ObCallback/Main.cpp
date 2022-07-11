#include "main.hpp"


int _tmain()
{
	BOOLEAN bStatus = TRUE;
	HANDLE hCon = INVALID_HANDLE_VALUE;
	DWORD dwDefaultMode;
	DWORD dwMode;
	BOOLEAN bConsoleMdoeChanged = FALSE;
	
	__try {
		// Change console output to display Unicode characters
		if (_setmode(_fileno(stdout), _O_U16TEXT) == -1) {
			bStatus = FALSE;
			__leave;
		}

		// Backup default console output mode
		hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hCon == INVALID_HANDLE_VALUE) {
			DisplayError(TEXT("GetStdHandle"));
			bStatus = FALSE;
			__leave;
		}

		if (!GetConsoleMode(hCon, &dwDefaultMode)) {
			DisplayError(TEXT("GetConsoleMode"));
			bStatus = FALSE;
			__leave;
		}

		// Enable virtual terminal
		dwMode = dwDefaultMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		if (!SetConsoleMode(hCon, dwMode)) {
			DisplayError(TEXT("SetConsoleMode"));
			bStatus = FALSE;
			__leave;
		}
		bConsoleMdoeChanged = TRUE;

		// Core work
		bStatus = RunAllTests();
	}

	__finally {
		if (hCon != INVALID_HANDLE_VALUE && bConsoleMdoeChanged) {
			if (!SetConsoleMode(hCon, dwDefaultMode)) {
				DisplayError(TEXT("SetConsoleMode"));
				bStatus = FALSE;
			}
		}
	}
	
	return bStatus ? EXIT_SUCCESS : EXIT_FAILURE;
}

BOOLEAN RunAllTests()
{
	INT nbFails = 0;
#ifndef _DEBUG
	DWORD dwServiceState;
#endif
	GENERIC_MAPPING genericMap;


#ifndef _DEBUG
	//
	// Check if the driver is running
	// (only in Release mdoe)
	//

	dwServiceState = GetServiceState(DRV_NAME);
	if (dwServiceState == 0) return FALSE; // Errors handled in GetServiceState
	if (dwServiceState != SERVICE_RUNNING) {
		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not running."), DRV_NAME);
		return 1;
	}
#endif

	// *** /!\ WARNING ***
	// Mapping dumped from Windows 10 21H2
	// Others versions of Windows may not have the same mapping for generic access rights to process-specific access rights
	genericMap.GenericRead = READ_CONTROL | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION;
	genericMap.GenericWrite = READ_CONTROL | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_DUP_HANDLE | PROCESS_CREATE_PROCESS | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION | PROCESS_SUSPEND_RESUME | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_SET_LIMITED_INFORMATION;
	genericMap.GenericExecute = READ_CONTROL | SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION;
	genericMap.GenericAll = PROCESS_ALL_ACCESS;

	// LOCAL
	_tprintf_s(TEXT("\033[4;36mLOCAL\033[0m\n"));
	nbFails += ObCallbackTest(NULL, &genericMap);

	// REMOTE
	_tprintf_s(TEXT("\033[4;36mREMOTE\033[24m (%s)\033[0m\n"), TEST_PROC);
	nbFails += ObCallbackTest(TEST_PROC, &genericMap);

	
	if (nbFails == 0) _tprintf_s(TEXT("  \033[7;32m ALL TESTS PASSED SUCCESSFULLY \033[0m\n\n"));
	else _tprintf_s(TEXT("  \033[7;31m %d TESTS FAILED \033[0m\n\n"), nbFails);


	return nbFails == 0;
}

INT ObCallbackTest(_In_opt_ LPCTSTR lpszProcessName, _In_ PGENERIC_MAPPING pGenericMap)
{
	INT nbFails = 0;
	DWORD dwMappedDesiredAccess;
	DWORD dwGrantedAccess;
	HANDLE hProcess = NULL;
	LPTSTR lpszFormat;
	SIZE_T szFormatLength, padAmount;


	const DWORD dwAccessRights[] = {
		// Standard access rights
		DELETE,        // 0x00010000
		READ_CONTROL,  // 0x00020000
		WRITE_DAC,     // 0x00040000
		WRITE_OWNER,   // 0x00080000
		SYNCHRONIZE,   // 0x00100000

		// Process-specific access rights
		PROCESS_TERMINATE,                  // 0x0001
		PROCESS_CREATE_THREAD,              // 0x0002
		PROCESS_SET_SESSIONID,              // 0x0004
		PROCESS_VM_OPERATION,               // 0x0008
		PROCESS_VM_READ,                    // 0x0010
		PROCESS_VM_WRITE,                   // 0x0020
		PROCESS_DUP_HANDLE,                 // 0x0040
		PROCESS_CREATE_PROCESS,             // 0x0080
		PROCESS_SET_QUOTA,                  // 0x0100
		PROCESS_SET_INFORMATION,            // 0x0200
		PROCESS_QUERY_INFORMATION,          // 0x0400
		PROCESS_SUSPEND_RESUME,             // 0x0800
		PROCESS_QUERY_LIMITED_INFORMATION,  // 0x1000
		PROCESS_SET_LIMITED_INFORMATION,    // 0x2000

		// Generic access rights
		GENERIC_ALL,      // 0x10000000
		GENERIC_EXECUTE,  // 0x20000000
		GENERIC_WRITE,    // 0x40000000
		GENERIC_READ      // 0x80000000
	};

	__try {
		// Allocate memory for formatted access mask (string)
		lpszFormat = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, BUFFER_SIZE);
		if (!lpszFormat) {
			DisplayError(TEXT("LocalAlloc"));
			nbFails = 1;
			__leave;
		}

		// Browse all access rights and test them
		for (DWORD dwDesiredAccess : dwAccessRights) {
			// Print
			FormatAccessMask(dwDesiredAccess, BUFFER_SIZE, lpszFormat);
			szFormatLength = _tcslen(lpszFormat);
			_tprintf_s(TEXT("[+] %s"), lpszFormat);

			// Get Process handle
			if (lpszProcessName) {
				hProcess = GetProcessHandleByName(lpszProcessName, dwDesiredAccess);
				if (!hProcess) {
					DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("Failed to get handle on %s (desired access: 0x%8.8X)"), lpszProcessName, dwDesiredAccess);
					nbFails++;
					__leave;
				}
			}
			else {
				hProcess = GetCurrentProcess();
				if (!hProcess) {
					DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("Failed to get handle on current process (desired access: 0x%8.8X)"), dwDesiredAccess);
					nbFails++;
					__leave;
				}
			}

			if (!GetProcessGrantedAccessRights(hProcess, dwDesiredAccess, &dwGrantedAccess)) {
				DisplayError(TEXT("GetProcessGrantedAccessRights"));
				nbFails++;
				__leave;
			}

#ifdef _DEBUG
			// Fake ObCallback filter effect (Debug only)
			if (hProcess != GetCurrentProcess()) {
				dwGrantedAccess &= ~FILTER_ACCESS_MASK;
			}
#endif

			CloseHandle(hProcess);
			hProcess = NULL;

			// Map generic rights bits to process-specific and standard rights following the mapping table.
			dwMappedDesiredAccess = dwDesiredAccess;
			MapGenericMask(&dwMappedDesiredAccess, pGenericMap);


			// print padding
			padAmount = _tcslen(PADDER) - szFormatLength;
			if (padAmount > 0) _tprintf_s(PADDER + szFormatLength);

			// Desired access rights does not appear in granted access rights (DesiredAccess AND NOT(GrantedAccess)) --> blocked
			if (dwMappedDesiredAccess & ~dwGrantedAccess) {
				if (
					// remote process
					lpszProcessName &&
					// ObCallback filter effect
					(dwMappedDesiredAccess & ~FILTER_ACCESS_MASK) & ~dwGrantedAccess
					)
				{
					_tprintf_s(TEXT("\033[31mBLOCKED\033[0m\n")); // red (unexpected)
					nbFails++;
				}
				else {
					_tprintf_s(TEXT("\033[32mBLOCKED\033[0m\n")); // green (expected)
				}
			}
			else {
				if (
					// remote process
					lpszProcessName &&
					// ObCallback filter effect
					FILTER_ACCESS_MASK & dwGrantedAccess
					)
				{
					_tprintf_s(TEXT("\033[31mPASS\033[0m\n")); // red (unexpected)
					nbFails++;
				}
				else {
					_tprintf_s(TEXT("\033[32mPASS\033[0m\n")); // green (expected)
				}
			}
		}

		_tprintf_s(TEXT("\n"));
	}

	__finally {
		if (hProcess) CloseHandle(hProcess);
	}

	return nbFails;
}
