/**
 * @file		K_Helper.cpp
 * @brief		Help functions used by several .cpp files
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#include "K_Helper.hpp"
#ifdef _DEBUG
#include "K_Helper.tmh"
#endif


_Success_(return == STATUS_SUCCESS) NTSTATUS K_HELPER::GetFileHash(_In_ PUNICODE_STRING szFileName, _Out_ PVOID * pHash, _Out_ ULONG * ulHashSize)
/**
 * @brief		Retrieve the file hash of "FileName"
 *
 * @param[in]	"szFileName"	- Name of the file to hash
 * @param[out]	"pHash"			- Hash returned
 * @param[out]	"ulHashSize"	- Size of the hash returned
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = 0;
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;
	ULONG ulHashObjectSize = 0;
	ULONG ulMyHashSize = 0;
	ULONG ulQuerySize = 0;
	ULONG ulRemainingBytes = 0;
	ULONG ulBytesToRead = 0;
	PVOID pHashObject = nullptr;
	PVOID pMyHash = nullptr;
	PVOID pBuffer = nullptr;
	IO_STATUS_BLOCK iostatus = { 0 };
	FILE_STANDARD_INFORMATION fileInfo = { 0 };
	OBJECT_ATTRIBUTES oa = { 0 };
	HANDLE hFile = NULL;
	LARGE_INTEGER byteOffset = { 0 };

	PAGED_CODE();

	__try {
		// Open algorithm handle
		status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, NULL);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptOpenAlgorithmProvider, status = 0x%x", status);
#endif
			__leave;
		}

		// Calculate the size of the buffer to hold the hash object
		status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&ulHashObjectSize, sizeof(ULONG), &ulQuerySize, NULL);
		if(!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptGetProperty, status = 0x%x", status);
#endif
			__leave;
		}
		
		// Calculate the length of the hash
		status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&ulMyHashSize, sizeof(ULONG), &ulQuerySize, NULL);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptGetProperty, status = 0x%x", status);
#endif
			__leave;
		}

		// Allocate the hash object on the heap
		pHashObject = ExAllocatePoolWithTag(PagedPool, ulHashObjectSize, DRIVER_TAG);
		if (pHashObject == nullptr)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to allocate memory");
#endif
			__leave;
		}

		// Allocate the hash var on the heap
		pMyHash = ExAllocatePoolWithTag(PagedPool, ulMyHashSize, DRIVER_TAG);
		if (pMyHash == nullptr)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to allocate memory");
#endif
			__leave;
		}

		// Create a hash
		status = BCryptCreateHash(hAlg, &hHash, (PUCHAR)pHashObject, ulHashObjectSize, NULL, NULL, NULL);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptCreateHash, status = 0x%x", status);
#endif
			__leave;
		}

		// Get handle to the file
		InitializeObjectAttributes(&oa, szFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		status = ZwCreateFile(&hFile, FILE_GENERIC_READ, &oa, &iostatus, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_NON_DIRECTORY_FILE, NULL, 0);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "ZwCreateFile failed: 0x%x", status);
#endif
			__leave;
		}

		// Get buffer size
		status = ZwQueryInformationFile(hFile, &iostatus, &fileInfo, sizeof(fileInfo), FileStandardInformation);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to ZwQueryInformationFile, status = 0x%x", status);
#endif
			__leave;
		}

		if (fileInfo.EndOfFile.QuadPart <= 0)
		{
			status = STATUS_UNSUCCESSFUL;
			__leave;
		}

		if (fileInfo.EndOfFile.QuadPart > FILE_MAX_SIZE)
		{
			status = STATUS_FILE_TOO_LARGE;
			__leave;
		}

		// Allocate the neeeded size hash on the heap
		pBuffer = ExAllocatePoolWithTag(PagedPool, FILE_BUFFER_SIZE, DRIVER_TAG);
		if (pBuffer == nullptr)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to allocate memory");
#endif
			__leave;
		}
		
		ulRemainingBytes = (ULONG)fileInfo.EndOfFile.QuadPart;

		while (ulRemainingBytes != 0)
		{
			ulBytesToRead = FILE_BUFFER_SIZE;

			if (ulBytesToRead > ulRemainingBytes)
				ulBytesToRead = ulRemainingBytes;

			status = ZwReadFile(hFile, NULL, NULL, NULL, &iostatus, pBuffer, ulBytesToRead, &byteOffset, NULL);
			if (!NT_SUCCESS(status))
			{
#ifdef _DEBUG
				TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to ZwReadFile, status = 0x%x", status);
#endif
				__leave;
			}

			if ((ULONG)iostatus.Information != ulBytesToRead)
			{
				status = STATUS_INTERNAL_ERROR;
				__leave;
			}
		
			status = BCryptHashData(hHash, (PUCHAR)pBuffer, ulBytesToRead, NULL);
			if (!NT_SUCCESS(status))
			{
#ifdef _DEBUG
				TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptHashData, status = 0x%x", status);
#endif
				__leave;
			}

			ulRemainingBytes -= ulBytesToRead;

		}

		// Hash
		status = BCryptFinishHash(hHash, (PUCHAR)pMyHash, ulMyHashSize, 0);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] Failed to BCryptFinishHash, status = 0x%x", status);
#endif
			__leave;
		}
		
		*pHash = pMyHash;
		*ulHashSize = ulMyHashSize;

		// Don't free the hash 
			
	}
	__finally {

		if (pBuffer) 
			ExFreePoolWithTag(pBuffer, DRIVER_TAG);

		if (hFile) 
			ZwClose(hFile);

		if (hHash) 
			BCryptDestroyHash(hHash);

		if (pHashObject) 
			ExFreePoolWithTag(pHashObject, DRIVER_TAG);

		if (hAlg) 
			BCryptCloseAlgorithmProvider(hAlg, 0);

		return status;

	}
}


_Success_(return != STATUS_SUCCESS) NTSTATUS K_HELPER::ConvertNtPathToDosPath(_In_ UNICODE_STRING szFullImageName, _Out_ UNICODE_STRING * szDosPath)
/**
 * @brief		Convert NT Path to DOS Path (letter)
 *
 * @param[in]	"szFullImageName"	- Path to convert
 * @param[out]	"szDosPath"		- Path converted (DOS)
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES oa = { 0 };
	IO_STATUS_BLOCK iostatus = { 0 };
	PVOID pObject = nullptr;
	POBJECT_NAME_INFORMATION pNameInformation = nullptr;

	*szDosPath = { 0 };
	
	__try {

		InitializeObjectAttributes(&oa, &szFullImageName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		status = ZwOpenFile(&hFile, FILE_READ_ATTRIBUTES, &oa, &iostatus, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "ZwOpenFile failed: 0x%x", status);
#endif
			__leave;
		}

		status = ObReferenceObjectByHandle(hFile, GENERIC_ALL, NULL, KernelMode, &pObject, NULL);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "ObReferenceObjectByHandle failed");
#endif
			__leave;
		}

		status = IoQueryFileDosDeviceName((PFILE_OBJECT)pObject, &pNameInformation);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "IoQueryFileDosDeviceName failed");
#endif
			__leave;
		}

		*szDosPath = pNameInformation->Name;

	}
	__finally {

		if (pObject) 
			ObDereferenceObject(pObject);

		if (hFile) 
			ZwClose(hFile);

		return status;

	}
}


_Success_(return != STATUS_SUCCESS) NTSTATUS K_HELPER::MyZwOpenKey(_Out_ PHANDLE hKey, _In_ PUNICODE_STRING szKeyPath)
/**
 * @brief		Wrapper for ZwOpenKey\n
 *
 * @param[out]	"hKey"		- Pointer to registry key handle (out parameter)
 * @param[in]	"szKeyPath"	- Registry path, WARNING : User-mode applications access registry keys relative to global handles, such as HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER.
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	
	// Initialize object Attribute
	OBJECT_ATTRIBUTES oa = { 0 };

	InitializeObjectAttributes(
		&oa,										// out struct
		szKeyPath,									// path
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,	// flags
		NULL,										// root directory (null for FQON)
		NULL										// security descriptor
	);

	// Open key
	status = ZwOpenKey(hKey, KEY_READ, &oa);

	return status;
}


_Success_(return == STATUS_SUCCESS) NTSTATUS K_HELPER::MyZwQueryValueKey(_In_ const HANDLE hKey, _In_ PUNICODE_STRING szValueName, _Out_ PKEY_VALUE_PARTIAL_INFORMATION * pRetAddress)
/**
 * @brief		Wrapper for ZwQueryValueKey.\n
 *				Warning: pRetAddress is a pointer to allocated memory that needs to be freed after use!
 *
 * @param[in]	"hKey"			- Registry key handle
 * @param[in]	"pValueName"	- Name of the value to read from registry key (hKey)
 * @param[out]	"pRetAddress"	- Pointer to value reading information structure, that contains wanted data (out parameter)
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	// Define an info to test the size of key value
	KEY_VALUE_PARTIAL_INFORMATION firstInfo = { 0 };
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = nullptr;
	// Define a pointer to the actual info structure, and then dynamically request the heap space
	ULONG length = 0;

	// Start reading
	status = ZwQueryValueKey(
		hKey,									// KeyHandle
		szValueName,							// ValueName
		KeyValuePartialInformation,				// KeyValueInformationClass
		&firstInfo,								// KeyValueInformation
		sizeof(KEY_VALUE_PARTIAL_INFORMATION),	// Length
		&length									// ResultLength
	);
	if (!NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL)
		return status;

	// Read successfully, apply for space, read again
	pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, length, DRIVER_TAG);
	status = ZwQueryValueKey(
		hKey,									// KeyHandle
		szValueName,							// ValueName
		KeyValuePartialInformation,				// KeyValueInformationClass
		pInfo,									// KeyValueInformation
		length,									// Length
		&length									// ResultLength
	);
	if (!NT_SUCCESS(status))
		return status;

	// Data is stored in pInfo->Data;
	*pRetAddress = pInfo;

	// pInfo allocated memory have to be freed after use one level above

	return status;
}


BOOLEAN K_HELPER::IsValidString(_In_ const _KEY_VALUE_PARTIAL_INFORMATION* pInfo)
/**
 * @brief		Returns TRUE if the passed _KEY_VALUE_PARTIAL_INFORMATION structure extracted from the Registry contains a valid Unicode string (REG_SZ type), else returns FALSE
 *
 * @param[in]	"pInfo"	- Pointer to a _KEY_VALUE_PARTIAL_INFORMATION that contains data extracted from a value in the Registry
 *
 * @return		TRUE	- The provided structure contains a valid Unicode string (REG_SZ).
 *				FALSE	- The provided structure does not contain a valid Unicode string (REG_SZ).
*/
{
	// must be tagged as REG_SZ
	if (pInfo->Type != REG_SZ)
		return FALSE;

	// Must be WCHAR array
	if (0 != (pInfo->DataLength % sizeof(WCHAR)))
		return FALSE;

	// Must be big enough for at least 1 NULL
	if (0 == pInfo->DataLength)
		return FALSE;

	// Must not be too big to fit into a UNICODE_STRING
	if (MAXUSHORT - 1 < pInfo->DataLength)
		return FALSE;

	// Must end in a NULL
	if (L'\0' != *(WCHAR*)&pInfo->Data[pInfo->DataLength - sizeof(WCHAR)])
		return false;

	// Must not contain other NULLs
	if (((wcsnlen_s((PCWSTR)pInfo->Data, 0xfffff) + 1) * sizeof(WCHAR)) != pInfo->DataLength)
		return FALSE;

	return TRUE;
}


BOOLEAN K_HELPER::IsValidStringArray(_In_ const _KEY_VALUE_PARTIAL_INFORMATION* pInfo)
/**
 * @brief		Returns TRUE if the passed _KEY_VALUE_PARTIAL_INFORMATION structure extracted from the Registry contains a valid Unicode string array (REG_MULTI_SZ type), else returns FALSE\n
 *				The REG_MULTI_SZ type is a sequence of null-terminated strings, terminated by a empty string (\0). ex: String1\0String2\0String3\0LastString\0\0
 *
 * @param[in]	"pInfo"	- pointer to a _KEY_VALUE_PARTIAL_INFORMATION that contains data extracted from a value in the Registry
 *
 * @return		TRUE	- The provided structure contains a valid Unicode string array (REG_MULTI_SZ).
 *				FALSE	- The provided structure does not contain a valid Unicode string array (REG_MULTI_SZ).
*/
{
	// must be tagged as REG_MULTI_SZ
	if (pInfo->Type != REG_MULTI_SZ)
		return FALSE;

	// Must be WCHAR array
	if (0 != (pInfo->DataLength % sizeof(WCHAR)))
		return FALSE;

	// Must be big enough for at least 2 NULLs
	if (0 == pInfo->DataLength || 1 == pInfo->DataLength)
		return FALSE;

	// Must end with 2 NULLs
	if (L'\0' != *(WCHAR*)&pInfo->Data[pInfo->DataLength - sizeof(WCHAR)] ||
		L'\0' != *(WCHAR*)&pInfo->Data[pInfo->DataLength - sizeof(WCHAR) * 2])
		return FALSE;

	return TRUE;
}


ULONG K_HELPER::CountWStringsInBuffer(_In_ PCWSTR szBuffer, _In_ const ULONG ulSize)
/**
 * @brief		Returns the number of Unicode strings found in a buffer (raw data from REG_MULTI_SZ).
 *
 * @param[in]	szBuffer	- Pointer to a buffer that contains raw data from a REG_MULTI_SZ
 * @param[in]	ulSize		- Buffer size in bytes
 *
 * @return		count		- Number of Unicode strings found in Buffer.
*/
{
	ULONG lCount = 0;

	// Trim ending \0 to not count the end of the REG_MULTI_SZ
	for (ULONG i = 0; i < (ulSize - sizeof(WCHAR)) / sizeof(WCHAR); i++) {
		if (L'\0' == *(WCHAR*)&szBuffer[i]) {
			lCount++;
		}
	}

	return lCount;
}


_Success_(return == STATUS_SUCCESS) NTSTATUS K_HELPER::GetFullImageNameByHandle(_In_ HANDLE hProcessId, _Out_ UNICODE_STRING * szFullImageName)
/**
 * @brief		Get the full image name by providing a handle to a process.
 *
 * @param[in]	"hProcessId"		- Handle to a process
 * @param[out]	"szFullImageName"	- Name of the loaded process
 *
 * @return		STATUS_SUCCESS		- No error occurs.
 *				!STATUS_SUCCES		- An error occurs.
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS eProcess = nullptr;
	HANDLE hProcess = NULL;
	K_HELPER::QUERY_INFO_PROCESS ZwQueryInformationProcess = { 0 };		// Undocumented API
	UNICODE_STRING szRoutine = { 0 };
	PUNICODE_STRING szImageName = nullptr;
	ULONG ulReturnedLength = 0;

	PAGED_CODE();

	__try {

		// Check if hProcessId is not empty
		if (hProcessId == NULL)
		{
			status = STATUS_INVALID_PARAMETER_1;
			__leave;
		}

		//
		// Get EPROCESS structure from the handle
		//

		status = PsLookupProcessByProcessId(hProcessId, &eProcess);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, FLT, "[-] PsLookupProcessByProcessId error: %8.8X", status);
#endif
			__leave;
		}

		status = ObOpenObjectByPointer(eProcess, 0, NULL, 0, 0, KernelMode, &hProcess);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "[-] ObOpenObjectByPointer Failed");
#endif
			__leave;
		}

		ObDereferenceObject(eProcess);

		//
		// Similar to GetProcAddress in UM, we retrieve the address of the routine ZwQueryInformationProcess to load it dynamically
		//

		szRoutine = RTL_CONSTANT_STRING(TEXT("ZwQueryInformationProcess"));

		ZwQueryInformationProcess = (K_HELPER::QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&szRoutine);
		if (ZwQueryInformationProcess == NULL)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "[-] Unable to retrieve the address of ZwQueryInformationProcess routine");
#endif
			status = STATUS_UNSUCCESSFUL;
			__leave;
		}

		// Allocate the needed memory
		szImageName = (PUNICODE_STRING)ExAllocatePoolWithTag(PagedPool, TRIPLE_MAX_PATH, DRIVER_TAG);
		if (szImageName == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}

		// Get the path from the handle
		status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, szImageName, TRIPLE_MAX_PATH, &ulReturnedLength);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "[-] ZwQueryInformationProcess 2 Failed");
#endif
			__leave;
		}

	}

	__finally {

		if (!NT_SUCCESS(status))
		{
			if (szImageName != nullptr) 
				ExFreePool(szImageName);

			return status;
		}

// Remove warning C6001 "Using uninitialized memory '*FullImageName'"
#pragma warning (suppress : 6001)
		if (szImageName != nullptr) 
			RtlCopyUnicodeString(szFullImageName, szImageName);

		return status;

	}
}


_Success_(return != -1) INT K_HELPER::MyNtPathCchRemoveFileSpec(_In_ PUNICODE_STRING szPath)
/**
 * @brief		Remake of PathCchRemoveFileSpec available in kernel-mode.\n
 *				Remove the last part of a path
 *
 * @param[in]	"szPath" - PUNICODE_STRING representing a path (NT path format!)
 *
 * @return		1	- Routine removed a part of the specified path.
 *				0	- Nothing left to remove.
 *				-1	- Error occurs.
*/
{
	INT status = -1;
	PWSTR szLimit, szLast = nullptr;

	__try {

		if (szPath->Length < 22 || szPath->Buffer == nullptr)
			__leave;

		// (+22) to jump over "\Device\HarddiskVolume"
		szLimit = wcschr(szPath->Buffer + 22, L'\\'); // Search first occurence of '\'
		if (szLimit == nullptr)
			__leave;

		szLast = wcsrchr(szLimit, L'\\'); // Search last occurence of '\'
		
		*szLast = L'\0';
		szPath->Length = (USHORT)wcsnlen_s(szPath->Buffer, szPath->MaximumLength) * sizeof(WCHAR);
		status = szLast == szLimit ? 0 : 1;

	}

	__finally {

		return status;

	}
}


_Success_(return == STATUS_SUCCESS) NTSTATUS K_HELPER::DosPathtoNtPath(_In_ PCUNICODE_STRING szPath, _Out_ UNICODE_STRING * szNormalized)
/**
 * @brief		Translate a DOS path into a NT path.
 *				(ex: \??\C:\Windows -> \Device\HarddiskVolume1\Windows)
 *
 * @param[in]	"szPath"		- PUNICODE_STRING representing a path (NT path format!)
 * @param[out]	"szNormalized"	- Pointer to UNICODE_STRING that receive the NT Path (out) (SHOULD BE ALLOCATED)
 *
 * @return		status - NTSTATUS error code.
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING szGlobalPrefix, szSubPath = { 0 };
	OBJECT_ATTRIBUTES attribs = { 0 };
	HANDLE hSymLink = NULL;
	ULONG ulSize = 0;

	if (!szNormalized)
		return STATUS_INVALID_PARAMETER;

	RtlInitUnicodeString(&szGlobalPrefix, L"\\??\\");

	if (!RtlPrefixUnicodeString(&szGlobalPrefix, szPath, TRUE))
		return STATUS_INVALID_PARAMETER;


	szSubPath.Buffer = (PWCH)((PUCHAR)szPath->Buffer + szGlobalPrefix.Length);
	szSubPath.Length = szPath->Length - szGlobalPrefix.Length;

	for (ULONG i = 0; i < szSubPath.Length; i++)
	{
		if (szSubPath.Buffer[i] == L'\\')
		{
			szSubPath.Length = (USHORT)(i * sizeof(WCHAR));
			break;
		}
	}

	if (szSubPath.Length == 0)
		return STATUS_INVALID_PARAMETER_1;

	szSubPath.Buffer = szPath->Buffer;
	szSubPath.Length += szGlobalPrefix.Length;
	szSubPath.MaximumLength = szSubPath.Length;

	// Open symlink

	InitializeObjectAttributes(&attribs, &szSubPath, OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwOpenSymbolicLinkObject(&hSymLink, GENERIC_READ, &attribs);
	if (!NT_SUCCESS(status))
		return status;

	// Query original name

// Remove warning C6001 "Using uninitialized memory '*Normalized'"
#pragma warning (suppress : 6001)
	status = ZwQuerySymbolicLinkObject(hSymLink, szNormalized, NULL);
	ZwClose(hSymLink);
	if (!NT_SUCCESS(status))
		return status;

	// Construct new variable

	ulSize = szPath->Length - szSubPath.Length + szNormalized->Length;
	if (ulSize > szNormalized->MaximumLength)
		return STATUS_BUFFER_OVERFLOW;

	szSubPath.Buffer = (PWCH)((PUCHAR)szPath->Buffer + szSubPath.Length);
	szSubPath.Length = szPath->Length - szSubPath.Length;
	szSubPath.MaximumLength = szSubPath.Length;

	status = RtlAppendUnicodeStringToString(szNormalized, &szSubPath);
	if (!NT_SUCCESS(status))
		return status;

	return STATUS_SUCCESS;
}