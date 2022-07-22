/**
 * @file		K_Whitelist.hpp
 * @brief		Header for K_Whitelist.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#ifndef WHITELIST_HPP
#define WHITELIST_HPP
#pragma once

#include "K_GlobalHeader.hpp"


class K_WHITELIST
{
	public:
		
		static VOID Unload();
		static NTSTATUS Initialize(_In_ PKEY_VALUE_PARTIAL_INFORMATION pInfo);
		static NTSTATUS IsInWhitelist(_In_ PUNICODE_STRING szTestString, _Out_ BOOLEAN * bResult);

	private:

		// Custom WHITELIST structure that contains a dynamically allocated WCHARs* and pointers of WCHARs* arrays and its number of elements
		typedef struct _WHITELIST {
			UNICODE_STRING* szBuffer;		// Array of UNICODE_STRINGs
			ULONG ulLength;					// Number of elements in Buffer
		} WHITELIST, * PWHITELIST;
	
		static WHITELIST _Whitelist;

};


#endif // !WHITELIST_HPP
