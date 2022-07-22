/**
 * @file		K_Helper.hpp
 * @brief		Header for K_Helper.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#ifndef HELPER_HPP
#define HELPER_HPP
#pragma once


#include "K_GlobalHeader.hpp"
#include "../SharedDefs/SharedDefs.hpp"
#include <bcrypt.h>


#define FILE_MAX_SIZE		(32 * 1024 * 1024)		// 32 MB
#define FILE_BUFFER_SIZE	(2 * PAGE_SIZE)		


class K_HELPER {
public:

	// Undocumented API

	typedef NTSTATUS(*QUERY_INFO_PROCESS) (
		__in HANDLE hProcessHandle,
		__in PROCESSINFOCLASS ProcessInformationClass,
		__out_bcount(ProcessInformationLength) PVOID pProcessInformation,
		__in ULONG ulProcessInformationLength,
		__out_opt PULONG ulReturnLength
	);

	// !Undocumented API

	static NTSTATUS GetFileHash(_In_ PUNICODE_STRING szFileName, _Out_ PVOID * pHash, _Out_ ULONG * ulHashSize);
	static NTSTATUS ConvertNtPathToDosPath(_In_ UNICODE_STRING szFullImageName, _Out_ UNICODE_STRING * szDosPath);
	static NTSTATUS MyZwOpenKey(_Out_ PHANDLE hKey, _In_ PUNICODE_STRING szKeyPath);
	static NTSTATUS MyZwQueryValueKey(_In_ const HANDLE hKey, _In_ PUNICODE_STRING szValueName, _Out_ PKEY_VALUE_PARTIAL_INFORMATION * pRetAddress);
	static BOOLEAN IsValidString(_In_ const _KEY_VALUE_PARTIAL_INFORMATION * pInfo);
	static BOOLEAN IsValidStringArray(_In_ const _KEY_VALUE_PARTIAL_INFORMATION * pInfo);
	static ULONG CountWStringsInBuffer(_In_ PCWSTR szBuffer, _In_ const ULONG ulSize);
	static NTSTATUS GetFullImageNameByHandle(_In_ HANDLE hProcessId, _Out_ UNICODE_STRING * szFullImageName);
	static INT MyNtPathCchRemoveFileSpec(_In_ PUNICODE_STRING szPath);
	static NTSTATUS DosPathtoNtPath(_In_ PCUNICODE_STRING szPath, _Out_ UNICODE_STRING * szNormalized);

};


#endif // !UTILITAIRE_HPP