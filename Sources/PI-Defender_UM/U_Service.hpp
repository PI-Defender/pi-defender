/**
 * @file		U_Service.hpp
 * @brief		Header for U_Service.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef SERVICE_HPP
#define SERVICE_HPP
#pragma once


#define SERVICE_NAME	L"PI-Defender_UM"
#define DRIVER_NAME		L"PI-Defender"
#define DRIVER_SYS		L"PI-Defender.sys"
#define DLL_NAME		L"PI-Defender_UM_MsgFile.dll"


#include "U_GlobalHeader.hpp"

#include "U_Configuration.hpp"
#include "U_Controller.hpp"
#include "U_Logs.hpp"
#include "U_Helper.hpp"
#include "U_Communication.hpp"


typedef struct _sSERVICE
{
	HANDLE hSvcStopEvent;
	SERVICE_STATUS_HANDLE hSvcStatus;
	SERVICE_STATUS SvcStatus;
} sSERVICE;


BOOLEAN Install();
BOOLEAN Query();
BOOLEAN Delete();
BOOLEAN Start();
BOOLEAN Stop();
VOID Service();

VOID WINAPI SvcMain(_In_ DWORD dwArgc, _In_ PWSTR* pszArgv);

#endif // !SERVICE_HPP
