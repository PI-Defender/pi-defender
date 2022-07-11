/**
 * @file		K_Configuration.hpp
 * @brief		Header for K_Configuration.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP
#pragma once

#pragma comment(lib, "ntdll.lib")

#include "K_GlobalHeader.hpp"
#include "K_Cache.hpp"
#include "K_Whitelist.hpp"


//
// Custom CONFIGURATION class that contains configuration parameters loaded from the Registry
//

class K_CONFIGURATION
{
	public:
		
		VOID Unload();
		NTSTATUS Initialize(_In_ PUNICODE_STRING szRegistryPath);

		PCWSTR GetCommunicationPort() { return this->_szCommunicationPort; }
		ULONG GetMaxClients() { return this->_ulMaxClients; }

		// Whitelist
		static NTSTATUS IsInWhitelist(_In_ PUNICODE_STRING szTestString, _Out_ BOOLEAN * bResult) {	return K_WHITELIST::IsInWhitelist(szTestString, bResult); }

		// Cache
		static BOOLEAN IsInCache(_In_ ULONG ulHashSize, _In_ PVOID pHash, _Out_opt_ BOOLEAN * bResult) { return K_CACHE::GetCachedValue(ulHashSize, pHash, bResult); }
		static VOID K_CONFIGURATION::AddToCache(_In_ ULONG cbHash, _In_ PVOID pHash, _In_ BOOLEAN bIsSigned) { K_CACHE::Add(cbHash, pHash, bIsSigned); }

	private:
		
		PCWSTR _szCommunicationPort;
		ULONG _ulMaxClients;

};

#endif // !CONFIGURATION_HPP