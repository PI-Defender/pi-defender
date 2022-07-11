/**
 * @file		main.hpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

#pragma once

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>	// for _fileno
#include <io.h>			// for _setmode
#include <fcntl.h>		// for _O_U16TEXT

#include "Utilitaire.hpp"


#define DRIVER_NAME		TEXT("PI-Defender")
#define SERVICE_NAME	TEXT("PI-Defender_UM")
#define BUFFER_SIZE		(_tcslen(TEXT("PROCESS_QUERY_LIMITED_INFORMATION")) + 1) * sizeof(TCHAR) // Longest access right string
#define PADDER			         TEXT("....................................")
#define NB_LOOPS			2000

#define FILTER_ACCESS_MASK (PROCESS_VM_WRITE | PROCESS_VM_OPERATION)


int _tmain();
BOOLEAN RunAllTests();
LONGLONG PerfTest();
