/**
 * @file       U_Registry.hpp
 * @brief      Header for U_Registry.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#ifndef REGISTRY_HPP
#define REGISTRY_HPP
#pragma once

#include "U_GlobalHeader.hpp"


class U_REGISTRY
{
	public:

		static LSTATUS CreateRegistryKey(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey);
		static LSTATUS WriteDwordInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ DWORD dwData);
		static LSTATUS WriteStringInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ PWSTR szData);
		static LSTATUS WriteMultiStringInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ const BYTE* pData, _In_ DWORD dwDataSize);
		
		static LSTATUS ReadValue(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ ULONG ulValueType, _Out_ PVOID* pData);

	private:

		static LSTATUS _WriteDataInRegistry(_In_ HKEY hKeyParent, _In_ PCWSTR szSubKey, _In_ PCWSTR szValueName, _In_ ULONG ulValueType, _In_ const BYTE* pData, _In_ DWORD dwDataSize);

};

#endif
