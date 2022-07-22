/**
 * @file		K_Cache.cpp
 * @brief		Store hash of signed and unsigned executable previously scanned
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#include "K_Cache.hpp"


PKGUARDED_MUTEX K_CACHE::_pMutex = nullptr;
K_CACHE::CACHELIST K_CACHE::_Cache = { 0, 0, nullptr };


VOID K_CACHE::Initialize(_In_ DWORD dwSize)
/**
 * @brief		Set maximum size of Cache and initialize mutex
 *
 * @param[in]	"dwSize" - Size of the buffer (cache), number of items to be kept.
 *
 * @return		None.
*/
{
	// Initialize _Cache static attribute
	_Cache.dwMaxLength = dwSize;
	_Cache.dwLength = 0;
	_Cache.pBuffer = (PHASH)ExAllocatePoolWithTag(PagedPool, dwSize * sizeof(HASH), DRIVER_TAG);

	// Initialize Cache mutex
	_pMutex = (PKGUARDED_MUTEX)ExAllocatePoolWithTag(NonPagedPool, sizeof(KGUARDED_MUTEX), DRIVER_TAG);
	if (!_pMutex)
		return;

	KeInitializeGuardedMutex(_pMutex);
}


BOOLEAN K_CACHE::_GetCachedValue(_In_ ULONG ulHashSize, _In_ PVOID pHash, _Out_opt_ BOOLEAN * bIsOk)
/**
 * @brief			Set "pbIsOk" to TRUE if cache contains specified Hash, FALSE otherwise.\n
 *					/!\ WARNING, not protected with mutex, use GetCachedValue() for public access instead
 *
 * @param[in]		"ulHashSize"- Size of bHash in bytes
 * @param[in]		"pHash"		- Pointer to buffer that holds Hash
 * @param[outopt]	"bIsOk"		- Pointer to a BOOLEAN variable that will get cache result (optional)
 *
 * @return			TRUE	- No error occurs.\n
 *					FALSE	- Error occurs
*/
{
	BOOLEAN bResult = FALSE;
	PHASH pMyHash = nullptr;

	// The thread got ownership of the mutex
	for (DWORD i = 0; i < _Cache.dwLength; i++) {
		pMyHash = &_Cache.pBuffer[i];
		if (pMyHash->ulHash == ulHashSize && RtlEqualMemory(pMyHash->pHash, pHash, ulHashSize)) {
			if (bIsOk) {
				// Only write if needed
				*bIsOk = pMyHash->bIsOk;
			}

			bResult = TRUE;
			break;
		}
	}

	return bResult;
}


BOOLEAN K_CACHE::GetCachedValue(_In_ ULONG ulHashSize, _In_ PVOID pHash, _Out_opt_ BOOLEAN * bIsOk)
/**
 * @brief			Set bIsOk to TRUE if cache contains specified Hash, FALSE otherwise.\n
 *					Wrapper for _GetCachedValue() with mutex protection (public use).
 *
 * @param[in]		"ulHashSize"- Size of bHash in bytes
 * @param[in]		"pHash"		- Pointer to buffer that holds Hash
 * @param[outopt]	"bIsOk"		- Pointer to a BOOLEAN variable that will get cache result (optional)
 *
 * @return			TRUE - No error occurs.\n
 *					FALSE - Error occurs
*/
{
	BOOLEAN bResult = FALSE;

	// Enter guarded region (APCs are disabled)
	KeAcquireGuardedMutex(_pMutex);

	bResult = _GetCachedValue(ulHashSize, pHash, bIsOk);

	// Exit guarded region
	KeReleaseGuardedMutex(_pMutex);

	return bResult;
}


VOID K_CACHE::Add(_In_ ULONG ulHashSize, _In_ PVOID pHash, _In_ BOOLEAN bIsOk)
/**
 * @brief			Add a Hash in the Cache and remember if it is signed or not
 *
 * @param[in]		"ulHashSize"- Size of bHash in bytes
 * @param[in]		"pHash"		- Pointer to buffer that holds Hash
 * @param[outopt]	"bIsOk"		- Pointer to a BOOLEAN variable that will get cache result (optional)
 *
 * @return			None.
*/
{
	PHASH pMyHash = nullptr;

	// Enter guarded region (APCs are disabled)
	KeAcquireGuardedMutex(_pMutex);

	// The thread got ownership of the mutex
	__try {

		// Check if not already in cache
		if (_GetCachedValue(ulHashSize, pHash, NULL))
			__leave;

		pMyHash = &_Cache.pBuffer[_Cache.dwLength];

		// Save information
		pMyHash->pHash = (PVOID)ExAllocatePoolWithTag(PagedPool, ulHashSize, DRIVER_TAG);
		if (!pMyHash->pHash)
			__leave;

		RtlCopyMemory(pMyHash->pHash, pHash, ulHashSize);

		pMyHash->ulHash = ulHashSize;
		pMyHash->bIsOk = bIsOk;

		// Update cache length
		_Cache.dwLength = (_Cache.dwLength + 1) % _Cache.dwMaxLength;

	}

	__finally {

		// Exit guarded region
		KeReleaseGuardedMutex(_pMutex);

	}
}
