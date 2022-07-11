/**
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
	DWORD dwServiceState;
	LONGLONG lPerfStarted, lPerfStopped;
	FLOAT fPerfOverhead;

	// SERVICE STARTED
	_tprintf_s(TEXT("\033[4;36mSERVICE STARTED\033[0m\n"));

	dwServiceState = MyStartService(SERVICE_NAME);
	if (dwServiceState != SERVICE_RUNNING)
		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not running."), SERVICE_NAME);
	
	lPerfStarted = PerfTest();


	// SERVICE STOPPED
	_tprintf_s(TEXT("\033[4;36mSERVICE STOPPED\033[0m\n"));
	
	dwServiceState = MyStopService(SERVICE_NAME);
	if (dwServiceState != SERVICE_STOPPED)
		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not stopped."), SERVICE_NAME);

	lPerfStopped = PerfTest();


	fPerfOverhead = ((FLOAT)lPerfStarted / lPerfStopped - 1) * 100;
#ifdef _DEBUG
	fPerfOverhead += 5; // fake too much performance overhead
#endif // _DEBUG



	if (fPerfOverhead < 5)
		_tprintf_s(TEXT("\n  \033[7;32m PERFORMANCE OVERHEAD : %.2f%% \033[0m\n\n"), fPerfOverhead);
	else
		_tprintf_s(TEXT("\n  \033[7;31m PERFORMANCE OVERHEAD : %.2f%% \033[0m\n\n"), fPerfOverhead);
	
	return fPerfOverhead < 5;
}

LONGLONG PerfTest()
{
	HANDLE hProcess = NULL;
	LARGE_INTEGER frequency, startTime, endTime, elapsedMicroSeconds;
	LONGLONG lElapsedTime = 0;

	UINT iRandomBuffer;
	DWORD dwDesiredAccess;


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

		_tprintf_s(TEXT("[+] Getting handle on random process [%d times]... "), NB_LOOPS);

		QueryPerformanceFrequency(&frequency);

		// Get timestamp
		QueryPerformanceCounter(&startTime);

		for (INT i = 0; i < NB_LOOPS; i++) {

			// Get random desired access
			BCryptGenRandom(NULL, (PUCHAR)&iRandomBuffer, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG); // assume it's ok
			dwDesiredAccess = dwAccessRights[iRandomBuffer % _countof(dwAccessRights)];


			// Get random process handle
			hProcess = GetRandomProcessHandle(dwDesiredAccess);
		}

		// Get second timestamp
		QueryPerformanceCounter(&endTime);

		// Compute elapsed time (in microseconds)
		elapsedMicroSeconds.QuadPart = endTime.QuadPart - startTime.QuadPart;
		elapsedMicroSeconds.QuadPart *= 1000000;
		elapsedMicroSeconds.QuadPart /= frequency.QuadPart;

		lElapsedTime = elapsedMicroSeconds.QuadPart;

		_tprintf_s(TEXT("\033[32m%.2f ms\033[0m\n"), (FLOAT)lElapsedTime / 1000);
	}

	__finally {
		if (hProcess) CloseHandle(hProcess);
	}

	return lElapsedTime;
}
