/**
 * @file        K_Whitelist.cpp
 * @brief       Store paths of autorized files and folders
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


#include "K_Whitelist.hpp"
#ifdef _DEBUG
#include "K_Whitelist.tmh"
#endif


K_WHITELIST::WHITELIST K_WHITELIST::_Whitelist = { nullptr, 0 };


VOID K_WHITELIST::Unload()
/**
 * @brief       Free previously allocated memory. Called by DriverUnload.
 *
 * @param       None.
 *
 * @return      None.
*/
{
	if (_Whitelist.szBuffer) {
		for (ULONG i = 0; i < _Whitelist.ulLength; i++) {
			if (_Whitelist.szBuffer[i].Buffer) {
				ExFreePool(_Whitelist.szBuffer[i].Buffer);
			}
		}
		ExFreePool(_Whitelist.szBuffer);
	}

}

NTSTATUS K_WHITELIST::Initialize(_In_ PKEY_VALUE_PARTIAL_INFORMATION pInfo)
/**
 * @brief       Set maximum size of Cache and initialize mutex
 *
 * @param[in]   "pInfo" - Pointer to a structure containing data extracted from the registy relative to the whitelist.
 *
 * @return      STATUS_SUCCESS	- No error occurs.\n
 *              Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	PCWSTR pBegin;
	SIZE_T Size;
	const UNICODE_STRING szGlobalPrefix = RTL_CONSTANT_STRING(TEXT("\\??\\"));
	UNICODE_STRING szWhitelistDosPath;


	__try {

		// Save the number of strings (paths) to be whitelisted in "Whitelist.Length" attribute
		_Whitelist.ulLength = K_HELPER::CountWStringsInBuffer((PCWSTR)pInfo->Data, pInfo->DataLength);

		if (_Whitelist.ulLength == 0) {
			_Whitelist.szBuffer = nullptr;
		}
		else {
			// Allocate memory and save a pointers array
			_Whitelist.szBuffer = (UNICODE_STRING*)ExAllocatePoolWithTag(PagedPool, _Whitelist.ulLength * sizeof(UNICODE_STRING), DRIVER_TAG);
			if (!_Whitelist.szBuffer) {
#ifdef _DEBUG
				TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] Error ExAllocatePoolWithTag : cannot allocate memory");
#endif
				status = STATUS_MEMORY_NOT_ALLOCATED;
				__leave;
			}


			// This block parses raw data (REG_MUTLI_SZ format) from pInfo->Data and save extracted strings in Whitelist.Buffer
			// The REG_MULTI_SZ type is a sequence of null-terminated strings, terminated by a empty string (\0). ex: String1\0String2\0String3\0LastString\0\0

			pBegin = (PCWSTR)&pInfo->Data;

			for (ULONG i = 0; i < _Whitelist.ulLength; i++) {

				Size = wcsnlen_s(pBegin, TRIPLE_MAX_PATH) * sizeof(WCHAR);

				szWhitelistDosPath.Length = 0;
				szWhitelistDosPath.MaximumLength = (USHORT)(szGlobalPrefix.Length + Size);
				szWhitelistDosPath.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, szWhitelistDosPath.MaximumLength, DRIVER_TAG);

				if (!szWhitelistDosPath.Buffer) {
#ifdef _DEBUG
					TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] ExAllocatepoolWithTag error : cannot allocate memory");
#endif
					status = STATUS_MEMORY_NOT_ALLOCATED;
					__leave;
				}

				RtlCopyUnicodeString(&szWhitelistDosPath, &szGlobalPrefix);

				status = RtlAppendUnicodeToString(&szWhitelistDosPath, pBegin);
				if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
					TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] RtlAppendUnicodeToString error : 0x%8.8X", status);
#endif
					__leave;
				}

				// remove trailing backslash (if existing)
				if (szWhitelistDosPath.Buffer[(szWhitelistDosPath.Length - 1) / sizeof(WCHAR)] == L'\\') {
					szWhitelistDosPath.Buffer[(szWhitelistDosPath.Length - 1) / sizeof(WCHAR)] = L'\0';
					szWhitelistDosPath.Length = (USHORT)wcsnlen_s(szWhitelistDosPath.Buffer, szWhitelistDosPath.MaximumLength) * sizeof(WCHAR);
				}

				_Whitelist.szBuffer[i].Length = 0;
				_Whitelist.szBuffer[i].MaximumLength = szWhitelistDosPath.Length + 0x200; // 0x200 -> NORMALIZE_INCREMENT (https://github.com/JKornev/hidden/blob/536a3ec3e88429de7ca6e8c3491fb5960bfae4cb/Hidden/Helper.h#L145)
				_Whitelist.szBuffer[i].Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, _Whitelist.szBuffer[i].MaximumLength, DRIVER_TAG);

				if (!_Whitelist.szBuffer[i].Buffer) {
#ifdef _DEBUG
					TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] ExAllocatepoolWithTag error : cannot allocate memory");
#endif
					status = STATUS_MEMORY_NOT_ALLOCATED;
					__leave;
				}

				status = K_HELPER::DosPathtoNtPath(&szWhitelistDosPath, &_Whitelist.szBuffer[i]);
				ExFreePool(szWhitelistDosPath.Buffer);
				szWhitelistDosPath.Buffer = NULL;

				if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
					TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] DosPathtoNtPath error : 0x%8.8X", status);
#endif
					__leave;
				}


				// Update pBegin
				pBegin += (Size + 2) / sizeof(WCHAR);
			}
		}
	}

	__finally {
	
		return status;
	
	}
}

NTSTATUS K_WHITELIST::IsInWhitelist(_In_ PUNICODE_STRING szTestString, _Out_ BOOLEAN * bResult)
/**
 * @brief       Check if whitelist contains provided string
 *
 * @param[in]   "szTestString"  - String to test (UNICODE_STRING)
 * @param[in]   "bResult"       - Pointer that receive result (out)
 *
 * @return      status - NTSTATUS Error code.
*/
{
	NTSTATUS status = STATUS_FAIL_CHECK;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iostatus;
	FILE_STAT_INFORMATION fileInfo;
	UNICODE_STRING szTempString;
	INT iResult;

	szTempString.Buffer = nullptr;

	*bResult = FALSE;

	__try {

		for (ULONG i = 0; i < _Whitelist.ulLength; i++) {
			szTempString.MaximumLength = szTestString->Length;
			szTempString.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, szTempString.MaximumLength, DRIVER_TAG);

			RtlCopyUnicodeString(&szTempString, szTestString);

			InitializeObjectAttributes(&oa, &_Whitelist.szBuffer[i], OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

			status = ZwQueryInformationByName(&oa, &iostatus, &fileInfo, sizeof(fileInfo), FileStatInformation);
			if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
				TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] ZwQueryInformationByName error : 0x%8.8X", status);
#endif
				__leave;
			}

			// Directory
			if (fileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Compare parent directory again and again
				do {
					// Remove the last element of the path string
					iResult = K_HELPER::MyNtPathCchRemoveFileSpec(&szTempString);
					if (iResult == -1) {
#ifdef _DEBUG
						TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] MyNtPathCchRemoveFileSpec error : %d", iResult);
#endif
						__leave;
					}

					// Compare with string in whitelist
					if (RtlEqualUnicodeString(&_Whitelist.szBuffer[i], &szTempString, TRUE)) {
						*bResult = TRUE;
						status = STATUS_SUCCESS;
						__leave;
					}
				} while (iResult == 1); // 0 when there are nothing to remove

			}

			// File
			else {
				// Compare with string in whitelist
				if (RtlEqualUnicodeString(&_Whitelist.szBuffer[i], &szTempString, TRUE)) {
					*bResult = TRUE;
					status = STATUS_SUCCESS;
					__leave;
				}
			}

		}

		status = STATUS_SUCCESS;

	}

	__finally {

		if (szTempString.Buffer)
			ExFreePool(szTempString.Buffer);

		return status;

	}
}