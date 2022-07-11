#pragma once
#pragma comment(lib, "fltlib.lib")

#include <Windows.h>
#include <fltUser.h>	// for FilterConnectCommunicationPort
#include <tchar.h>
#include <strsafe.h>	// for _fileno
#include <io.h>			// for _setmode
#include <fcntl.h>		// for _O_U16TEXT

#include "Utilitaire.hpp"

#define COMMUNICATION_PORT	TEXT("\\PIDefenderPort")
#define DRV_NAME			TEXT("PI-Defender")


int _tmain();
BOOLEAN RunAllTests();