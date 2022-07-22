/**
 * @file        K_Configuration.cpp
 * @brief       Load the configuration of the driver
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


#include "K_Configuration.hpp"
#ifdef _DEBUG
#include "K_Configuration.tmh"
#endif


VOID K_CONFIGURATION::Unload()
/**
 * @brief       Free previously allocated memory. Called by DriverUnload.
 *
 * @param       None.
 *
 * @return      None.
*/
{
	// Free allocated memory to save configuration strings
	if (this->_szCommunicationPort) 
		ExFreePool((PVOID)this->_szCommunicationPort);

	// Whitelist
	K_WHITELIST::Unload();

}


NTSTATUS K_CONFIGURATION::Initialize(_In_ PUNICODE_STRING szRegistryPath)
/**
 * @brief       Initialize configuration from Registry.\n
 *              Must be called once before calling others methods of this class.\n
 *              The configuration is loaded from HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender\Parameters.
 *
 * @param[in]	"szRegistryPath" - Registry path to service key
 *
 * @return      STATUS_SUCCESS	- No error occurs.\n
 *              Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hRegistryKey = { 0 };
	UNICODE_STRING szFullRegistryPath = { 0 };
	UNICODE_STRING szParametersString = RTL_CONSTANT_STRING(L"\\Parameters");
	UNICODE_STRING szCommunicationPortValueName = RTL_CONSTANT_STRING(L"CommunicationPort");
	UNICODE_STRING szMaxClientsValueName = RTL_CONSTANT_STRING(L"MaxClients");
	UNICODE_STRING szWhitelistValueName = RTL_CONSTANT_STRING(L"Whitelist");
	UNICODE_STRING szCacheSizeValueName = RTL_CONSTANT_STRING(L"CacheSize");
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = { 0 };

	__try {

		//
		// Append "\Parameters" to RegistryPath
		//

		// Add the lengths of the two strings
		szFullRegistryPath.MaximumLength = szRegistryPath->Length + szParametersString.Length;
		// Allocate memory
		szFullRegistryPath.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, szFullRegistryPath.MaximumLength, DRIVER_TAG);
		if (!szFullRegistryPath.Buffer) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, CONF, "[-] ExAllocatePoolWithTag error : Cannot allocate memory");
#endif
			status = STATUS_MEMORY_NOT_ALLOCATED;
			__leave;
		}
		// Copy first string
		RtlCopyUnicodeString(&szFullRegistryPath, szRegistryPath);
		// Append second string
		status = RtlAppendUnicodeStringToString(&szFullRegistryPath, &szParametersString);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, CONF, "[-] RtlAppendUnicodeStringToString error : 0x%08X", status);
#endif
			__leave;
		}

		// Open Registry key
		status = K_HELPER::MyZwOpenKey(&hRegistryKey, &szFullRegistryPath);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] MyZwQueryKey error : 0x%08X", status);
#endif
			__leave;
		}

		//
		// Get CommunicationPort Value
		//

		status = K_HELPER::MyZwQueryValueKey(hRegistryKey, &szCommunicationPortValueName, &pInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] MyZwQueryValueKey error : 0x%08X", status);
#endif
			__leave;
		}

		// Check data integrity (REG_SZ)
		if (!K_HELPER::IsValidString(pInfo)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] Type mismatch, expect REG_SZ");
#endif
			status = STATUS_OBJECT_TYPE_MISMATCH;
			__leave;
		}

		// Save value in "CommunicationPort" attribute
		// Allocate memory (this process is needed to have strings aligned on memory)
		this->_szCommunicationPort = (PCWSTR)ExAllocatePoolWithTag(PagedPool, pInfo->DataLength, DRIVER_TAG);
		if (this->_szCommunicationPort == nullptr) __leave;
		// Copy string
		RtlCopyMemory((PVOID)this->_szCommunicationPort, &pInfo->Data, pInfo->DataLength);


		//
		// Get MaxClients Value
		//

		status = K_HELPER::MyZwQueryValueKey(hRegistryKey, &szMaxClientsValueName, &pInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] MyZwQueryValueKey error : 0x%08X", status);
#endif
			__leave;
		}

		// Check data integrity (REG_DWORD)
		if (pInfo->Type != REG_DWORD || pInfo->DataLength != 4) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] Type mismatch, expect REG_DWORD");
#endif
			status = STATUS_OBJECT_TYPE_MISMATCH;
			__leave;
		}

		// Save value in "MaxClients" attribute
		this->_ulMaxClients = *pInfo->Data;


		//
		// Get Whitelist Value
		//

		status = K_HELPER::MyZwQueryValueKey(hRegistryKey, &szWhitelistValueName, &pInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] MyZwQueryValueKey error : 0x%08X", status);
#endif
			__leave;
		}

		// Check data integrity (REG_MULTI_SZ)
		if (!K_HELPER::IsValidStringArray(pInfo)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] Type mismatch, expect REG_MULTI_SZ");
#endif
			status = STATUS_OBJECT_TYPE_MISMATCH;
			__leave;
		}

		// Save value in "Whitelist" attribute
		status = K_WHITELIST::Initialize(pInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, CONF, "[-] K_WHITELIST::Initialize error : 0x%8.8X", status);
#endif
			__leave;
		}
		

		//
		// Initialize Cache
		//

		status = K_HELPER::MyZwQueryValueKey(hRegistryKey, &szCacheSizeValueName, &pInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] MyZwQueryValueKey error : 0x%8.8X", status);
#endif
			__leave;
		}

		// Check data integrity (REG_DWORD)
		if (pInfo->Type != REG_DWORD || pInfo->DataLength != 4) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, REG, "[-] Type mismatch, expect REG_DWORD");
#endif
			status = STATUS_OBJECT_TYPE_MISMATCH;
			__leave;
		}

		// Initialize Cache
		K_CACHE::Initialize(*pInfo->Data);

	}

	__finally {

		if (hRegistryKey) 
			ZwClose(hRegistryKey);

		if (szFullRegistryPath.Buffer) 
			ExFreePool(szFullRegistryPath.Buffer);

		if (pInfo) 
			ExFreePool(pInfo);

		return status;

	}
}
