﻿/**
 * @file		Main.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

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
	LONGLONG time = 0;
	LONGLONG maxTime = time;
#ifndef _DEBUG
	DWORD dwServiceState;
#endif


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

	// LOCAL
	_tprintf_s(TEXT("\033[4;36mLOCAL\033[0m\n"));
	time = SpeedTest(NULL);
	maxTime = max(time, maxTime);

	// REMOTE
	_tprintf_s(TEXT("\033[4;36mREMOTE\033[24m (%s)\033[0m\n"), TEST_PROC);
	time = SpeedTest(TEST_PROC);
	maxTime = max(time, maxTime);


	if (maxTime < 1000000)
		_tprintf_s(TEXT("  \033[7;32m MAX ELAPSED TIME : %.2f ms \033[0m\n\n"), (FLOAT)maxTime / 1000);
	else
		_tprintf_s(TEXT("  \033[7;31m MAX ELAPSED TIME : %.2f ms \033[0m\n\n"), (FLOAT)maxTime / 1000);


	return maxTime < 1000000;
}

INT SpeedTest(_In_opt_ LPCTSTR lpszProcessName)
{
	HANDLE hProcess = NULL;
	LPTSTR lpszFormat;
	SIZE_T szFormatLength, padAmount;
	LARGE_INTEGER frequency, startTime, endTime, elapsedMicroSeconds;
	LONGLONG maxElapsedTime = 0;


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
			__leave;
		}

		// Browse all access rights and test them
		for (DWORD dwDesiredAccess : dwAccessRights) {
			// Print
			FormatAccessMask(dwDesiredAccess, BUFFER_SIZE, lpszFormat);
			szFormatLength = _tcslen(lpszFormat);
			_tprintf_s(TEXT("[+] %s"), lpszFormat);


			QueryPerformanceFrequency(&frequency);
			

			// Get Process handle
			if (lpszProcessName) {

				// Get timestamp
				QueryPerformanceCounter(&startTime);

				// Timed activity
				hProcess = GetProcessHandleByName(lpszProcessName, dwDesiredAccess);

				// Get second timestamp
				QueryPerformanceCounter(&endTime);

				if (!hProcess) {
					DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("Failed to get handle on %s (desired access: 0x%8.8X)"), lpszProcessName, dwDesiredAccess);
					__leave;
				}
			}
			else {

				// Get timestamp
				QueryPerformanceCounter(&startTime);

				// Timed activity
				hProcess = GetCurrentProcess();

				// Get second timestamp
				QueryPerformanceCounter(&endTime);

				if (!hProcess) {
					DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("Failed to get handle on current process (desired access: 0x%8.8X)"), dwDesiredAccess);
					__leave;
				}
			}

			CloseHandle(hProcess);
			hProcess = NULL;


			// print padding
			padAmount = _tcslen(PADDER) - szFormatLength;
			if (padAmount > 0) _tprintf_s(PADDER + szFormatLength);

			// Compute elapsed time (in microseconds)
			elapsedMicroSeconds.QuadPart = endTime.QuadPart - startTime.QuadPart;
#ifdef _DEBUG
			if (dwDesiredAccess & (PROCESS_VM_OPERATION | PROCESS_VM_WRITE)) elapsedMicroSeconds.QuadPart += 10000000; // fake long time (debug only)
#endif // _DEBUG
			elapsedMicroSeconds.QuadPart *= 1000000;
			elapsedMicroSeconds.QuadPart /= frequency.QuadPart;

			if (elapsedMicroSeconds.QuadPart < 1000000)
				_tprintf_s(TEXT(" \033[32m%.2f ms\033[0m\n"), (FLOAT)elapsedMicroSeconds.QuadPart / 1000); // green (ok)
			else
				_tprintf_s(TEXT(" \033[31m%.2f ms\033[0m\n"), (FLOAT)elapsedMicroSeconds.QuadPart / 1000); // red (too long)

			maxElapsedTime = max(maxElapsedTime, elapsedMicroSeconds.QuadPart);
		}

		_tprintf_s(TEXT("\n"));
	}

	__finally {
		if (hProcess) CloseHandle(hProcess);
	}

	return maxElapsedTime;
}
