/**
 * @file		Main.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

#include "Main.hpp"

int _tmain()
{
	BOOLEAN bStatus = TRUE;
	HANDLE hCon = INVALID_HANDLE_VALUE;
	DWORD dwDefaultMode;
	DWORD dwMode;
	BOOLEAN bConsoleModeChanged = FALSE;

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
		bConsoleModeChanged = TRUE;

		// Test Title
		_tprintf_s(TEXT("\nTEST_COMMUNICATION\n------------------\n"));

		// Core work
		bStatus = RunAllTests();
	}

	__finally {
		if (hCon != INVALID_HANDLE_VALUE && bConsoleModeChanged) {
			if (!SetConsoleMode(hCon, dwDefaultMode)) {
				DisplayError(TEXT("SetConsoleMode"));
				bStatus = FALSE;
			}
		}
	}

	return bStatus;
}

BOOLEAN RunAllTests()
{
	BOOLEAN bStatus = FALSE;
	DWORD dwServiceState = 0;
	HANDLE hCompletion = INVALID_HANDLE_VALUE;
	HANDLE hPort = INVALID_HANDLE_VALUE;

	__try {
		// Check if the driver is running
		dwServiceState = GetServiceState(DRV_NAME);
		if (dwServiceState == 0)
			__leave; // Errors handled in GetServiceState
		if (dwServiceState != SERVICE_RUNNING) {
			DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not running."), DRV_NAME);
			bStatus = TRUE;
			__leave;
		}

		bStatus = ConnectToDriver((PWSTR)COMMUNICATION_PORT, 1, &hCompletion, &hPort);
		if (!bStatus)
			__leave;

		CreateListenerThread(hPort, hCompletion);

	}
	__finally {

		return bStatus;

	}

}