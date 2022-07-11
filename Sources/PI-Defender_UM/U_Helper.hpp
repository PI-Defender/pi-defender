/**
 * @file		U_Helper.hpp
 * @brief		Header for U_Helper.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef HELPER_HPP
#define HELPER_HPP
#pragma once

#pragma comment(lib, "Shlwapi.lib" )
#pragma comment(lib, "PathCch.lib" )

#include <Shlwapi.h>
#include <PathCch.h>
#include <string>
#include <iostream>
#include <TlHelp32.h>	// GetProcessByName
#include <strsafe.h>	// for _fileno
#include <io.h>			// for _setmode
#include <fcntl.h>		// for _O_U16TEXT

#include "U_GlobalHeader.hpp"
#include "U_Logs.hpp"
#include "U_Service.hpp"


class U_HELPER
{
	public:

		// Print
		typedef enum class _MESSAGE_TYPE {
			TYPE_ERROR,		// print in red
			TYPE_SUCCESS	// print in green
		} MESSAGE_TYPE;


		static BOOLEAN EnableVirtualTerminal(_Out_ HANDLE* hCon, _Out_ DWORD* dwDefaultMode);

		static VOID WelcomeMessage();
		static VOID PrintUsage();

		static BOOLEAN GetCurrentFolder(_Inout_ PWSTR szFolder);

		static BOOLEAN GetDllFullPath(_Out_ PWSTR szPath);

		static BOOLEAN GetHandleFromFileName(_Out_ HANDLE* hFileToCheck, _In_ PCWSTR szFileName);

		static VOID DisplayError(_In_ PCWSTR szFunction);
		static VOID PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR szFormat, ...);
		static VOID DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR szFormat, ...);

		static ULONG CountWStringsInBuffer(_In_ PCWSTR szBuffer, _In_ const ULONG ulSize);

		static HANDLE GetProcessHandleByName(_In_ PWSTR szProcessName);
};



#endif // !UTILITAIRE_HPP
