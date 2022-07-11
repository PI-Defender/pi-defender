#pragma once


#include <windows.h>
#include <tchar.h>
#include <strsafe.h>	// for StringCchCopy
#include <winternl.h>	// for NtQueryObject
#include <TlHelp32.h>

#pragma comment (lib, "ntdll.lib")	// for NtQueryObject


// Print
typedef enum class _MESSAGE_TYPE {
	TYPE_ERROR,		// print in red
	TYPE_SUCCESS	// print in green
} MESSAGE_TYPE;

VOID PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...);
VOID DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...);
VOID DisplayError(_In_ LPCTSTR lpszFunction);
VOID FormatAccessMask(_In_ DWORD dwMask, _In_ SIZE_T szFormatSize, _Out_ LPTSTR lpszFormat);

// Service
SC_HANDLE GetSCMHandle();
DWORD GetServiceState(_In_ LPCTSTR lpszServiceName);

// Process Handle & access rights
HANDLE GetProcessHandleByName(_In_ LPCTSTR lpszProcessName, _In_ DWORD dwDesiredAccess);
_Success_(return) BOOLEAN GetProcessGrantedAccessRights(_In_ HANDLE hProcess, _In_ DWORD dwDesiredAccess, _Out_ DWORD * dwGrantedAccess);
