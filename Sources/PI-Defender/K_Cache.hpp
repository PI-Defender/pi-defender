/**
 * @file        K_Cache.hpp
 * @brief       Header for K_Cache.cpp
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


#ifndef CACHE_HPP
#define CACHE_HPP
#pragma once

#include "K_GlobalHeader.hpp"


class K_CACHE {
public:

	static VOID Initialize(_In_ DWORD dwSize);
	static BOOLEAN GetCachedValue(_In_ ULONG ulHashSize, _In_ PVOID pHash, _Out_opt_ BOOLEAN* bIsOk);
	static VOID Add(_In_ ULONG ulHashSize, _In_ PVOID pHash, _In_ BOOLEAN bIsOk);

private:

	typedef struct _HASH {
		ULONG ulHash;
		PVOID pHash;
		BOOLEAN bIsOk;
	} HASH, * PHASH;

	typedef struct _CACHELIST {
		DWORD dwLength;
		DWORD dwMaxLength;
		HASH* pBuffer;
	} CACHELIST, * PCACHELIST;

	static CACHELIST _Cache;
	static PKGUARDED_MUTEX _pMutex;

	static BOOLEAN _GetCachedValue(_In_ ULONG ulHashSize, _In_ PVOID pHash, _Out_opt_ BOOLEAN* bIsOk);

};


#endif // !CACHE_HPP
