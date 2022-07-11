#pragma once

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>	// for _fileno
#include <io.h>			// for _setmode
#include <fcntl.h>		// for _O_U16TEXT

#include "Utilitaire.hpp"


#define DRV_NAME		TEXT("PI-Defender")
#define TEST_PROC		TEXT("explorer.exe")
#define BUFFER_SIZE		(_tcslen(TEXT("PROCESS_QUERY_LIMITED_INFORMATION")) + 1) * sizeof(TCHAR) // Longest access right string
#define PADDER			         TEXT("....................................")

#define FILTER_ACCESS_MASK (PROCESS_VM_WRITE | PROCESS_VM_OPERATION)


int _tmain();
BOOLEAN RunAllTests();
INT SpeedTest(_In_opt_ LPCTSTR lpszProcessName);
