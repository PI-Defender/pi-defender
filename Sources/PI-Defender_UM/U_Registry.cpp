/**
 * @file		U_Registry.cpp
 * @brief		Create / Write / Delete registry key
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#include "U_Registry.hpp"


LSTATUS U_REGISTRY::_WriteDataInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ ULONG ulValueType, _In_ const BYTE* pData, _In_ DWORD dwDataSize)
/**
 * @brief		Write data in the registry key "szSubKey" located in the HIVE "hKeyParent"
 *
 * @param[in]	"hKeyParent"	- Hive(HKLM, HKCU, etc..)
 * @param[in]	"szSubKey"		- Key that has to be created
 * @param[in]	"szValueName"	- Name of the value
 * @param[in]	"ulValueType"	- Type of the data (ex: REG_DWORD)
 * @param[in]	"pData"			- Pointer to the data (value) to store in the Registry
 * @param[in]	"dwDataSize"	- Size of data in bytes
 *
 * @return		ERROR_SUCCESS - No error occurs.\n
 *				Other status - Error status.
*/
{
	LSTATUS lResult = 0;
	HKEY hKey = NULL;

	__try {

		lResult = RegOpenKeyEx(hKeyParent, szSubKey, 0, KEY_WRITE, &hKey);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("[-]Error opening the key %s\n"), szSubKey);
			__leave;
		}

		lResult = RegSetValueEx(hKey, szValueName, 0, ulValueType, pData, dwDataSize);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("[-]Error write %s in the subkey %s\n"), szValueName, szSubKey);
			__leave;
		}

	}
	__finally {

		if (hKey != NULL)
			RegCloseKey(hKey);

		return lResult;

	}
}

LSTATUS U_REGISTRY::CreateRegistryKey(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey)
/**
 * @brief		Create the registry key "szSubKey" located in the HIVE "hKeyParent"
 *
 * @param[in]	"hKeyParent"	- Hive (HKLM, HKCU, etc..)
 * @param[in]	"szSubKey"		- Key that has to be created
 *
 * @return		ERROR_SUCCESS	- No error occurs.\n
 *				Other status	- Error status.
*/
{
	LSTATUS lResult = 0;
	HKEY hKey = NULL ;

	lResult = RegCreateKeyEx(hKeyParent, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (lResult != ERROR_SUCCESS)
	{
		_tprintf_s(TEXT("[-]Error creating the key %s\n"), szSubKey);
		return lResult;
	}

	lResult = RegCloseKey(hKey);

	return lResult;
}

LSTATUS U_REGISTRY::WriteDwordInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ DWORD dwData)
/**
 * @brief		Write DWORD in the registry key "szSubKey" located in the HIVE "hKeyParent"
 *
 * @param[in]	"hKeyParent"	- Hive(HKLM, HKCU, etc..)
 * @param[in]	"szSubKey"		- Key that has to be created
 * @param[in]	"szValueName"	- Name of the value
 * @param[in]	"dwData"		- Value (int, hex, ..)
 *
 * @return		ERROR_SUCCESS - No error occurs.\n
 *				Other status - Error status.
*/
{
	LSTATUS lResult = 0;

	lResult = _WriteDataInRegistry(
		hKeyParent,
		szSubKey,
		szValueName,
		REG_DWORD,
		reinterpret_cast<BYTE*>(&dwData),
		sizeof(DWORD)
	);

	return lResult;
}

LSTATUS U_REGISTRY::WriteStringInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ PWSTR szData)
/**
 * @brief		Write STRING in the registry key "szSubKey" located in the HIVE "hKeyParent"
 *
 * @param[in]	"hKeyParent"	- Hive(HKLM, HKCU, etc..)
 * @param[in]	"szSubKey"		- Key that has to be created
 * @param[in]	"szValueName"	- Name of the value
 * @param[in]	"szData"		- Value (str)
 *
 * @return		ERROR_SUCCESS - No error occurs.\n
 *				Other status - Error status.
*/
{
	LSTATUS lResult = 0;

	lResult = _WriteDataInRegistry(
		hKeyParent,
		szSubKey,
		szValueName,
		REG_SZ,
		reinterpret_cast<BYTE*>(szData),
		((SIZE_T)lstrlen(szData) + 1) * sizeof(WCHAR)
	);

	return lResult; 
}


LSTATUS U_REGISTRY::WriteMultiStringInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ const BYTE* pData, _In_ DWORD dwDataSize)
/**
 * @brief		Write MULTI_SZ STRING in the registry key "szSubKey" located in the HIVE "hKeyParent"
 *
 * @param[in]	"hKeyParent"	- Hive(HKLM, HKCU, etc..)
 * @param[in]	"szSubKey"		- Key that has to be created
 * @param[in]	"szValueName"	- Name of the value
 * @param[in]	"pData"			- Value (str with TWO terminating null-bytes)
 * @param[in]	"dwDataSize"		- Size of data in bytes
 *
 * @return		ERROR_SUCCESS - No error occurs.\n
 *				Other status - Error status.
*/
{
	LSTATUS lResult = 0;

	lResult = _WriteDataInRegistry(
		hKeyParent,
		szSubKey,
		szValueName,
		REG_MULTI_SZ,
		pData,
		dwDataSize
	);

	return lResult; 
}

LSTATUS U_REGISTRY::ReadValue(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ ULONG ulValueType, _Out_ PVOID* pData)
/**
 * @brief		Read the content (DWORD) of the registry key "szSubKey" located in the HIVE "hKeyParent".
 *				The return pointer pData is allocated here and needs to be free after use by the caller.
 *
 * @param[in]	"hKeyParent"	- Hive(HKLM, HKCU, etc..).
 * @param[in]	"szSubKey"		- Key that has to be created.
 * @param[in]	"szValueName"	- Name of the value.
 * @param[in]	"ulValueType"	- Type of the data (ex: REG_DWORD)
 * @param[out]	"pData"			- Pointer to the data (value) extracted from the Registry.
 *
 * @return		ERROR_SUCCESS - No error occurs.\n
 *				Other status - Error status.
*/
{
	LSTATUS lResult = 0;
	HKEY hKey = NULL;
	DWORD dwDataSize = 0;
	DWORD dwValueType = 0;
	LPBYTE bData = nullptr;

	__try {

		lResult = RegOpenKeyEx(hKeyParent, szSubKey, 0, KEY_READ, &hKey);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error opening the key\n"));
			__leave;
		}

		// Get data size
		lResult = RegQueryValueEx(hKey, szValueName, NULL, &dwValueType, NULL, &dwDataSize);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error reading the key size\n"));
			__leave;
		}

		// Type verification
		if (dwValueType != ulValueType)
		{
			_tprintf_s(TEXT("Error reading the key size, type mismatch\n"));
			lResult = ERROR_CAN_NOT_COMPLETE;
			__leave;
		}

		// Allocate memory
		bData = (LPBYTE)malloc(dwDataSize);
		if (bData == NULL)
		{
			lResult = ERROR_CAN_NOT_COMPLETE;
			__leave;
		}

		// Read data
		lResult = RegQueryValueEx(hKey, szValueName, NULL, NULL, bData, &dwDataSize);
		if (lResult != ERROR_SUCCESS)
		{
			_tprintf_s(TEXT("Error reading the key\n"));
			__leave;
		}

		*pData = bData;

	}
	__finally {

		if (hKey != NULL)
			RegCloseKey(hKey);
		
		return lResult;

	}
}