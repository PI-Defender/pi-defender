/**
 * @file       U_Signature.cpp
 * @brief      erify embedded signature and catalog signature of an executable
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Signature.hpp"


BOOLEAN U_SIGNATURE::VerifyEmbeddedSignature(_In_ PCWSTR szFile)
/**
 * @brief      Check if the file "szFile" is signed by a trusted authority.\n
 *             [i] WINTRUST_ACTION_GENERIC_VERIFY_V2:
 *                 - Check if the certificate used to sign the file chains up to a root certificate located in the trusted root certificate store.
 *
 * @param[in]  "szFile" - Name of the .exe to be verify
 *
 * @return     TRUE  - File signed
 *             FALSE - File not signed
*/
{
	WINTRUST_FILE_INFO winTrustFileInfo = { 0 };
	WINTRUST_DATA winTrustData = { 0 };
	WINTRUST_SIGNATURE_SETTINGS winTrustSettings = { 0 };
	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	BOOLEAN bSigned = FALSE;
	
	__try {

		//
		// Initialize WinTrust Data
		//

		winTrustData.cbStruct = sizeof(WINTRUST_DATA);
		winTrustData.pPolicyCallbackData = NULL;		// Default code signing EKU
		winTrustData.pSIPClientData = NULL;			// No data to pass to SIP
		winTrustData.dwUIChoice = WTD_UI_NONE;			// Disable WVT UI
		winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;	// No revocation checking
		winTrustData.dwUnionChoice = WTD_CHOICE_FILE;		// Verify embedded signature
		winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;	// Verify action
		winTrustData.hWVTStateData = NULL;			// Verification sets the value
		winTrustData.pwszURLReference = NULL;			// Not used
		winTrustData.dwUIContext = 0;				// UI Disable so no context
		
		winTrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
		winTrustFileInfo.pcwszFilePath = szFile;
		winTrustFileInfo.hFile = NULL;
		winTrustData.pFile = &winTrustFileInfo;			// Set pFile

		winTrustSettings.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
		winTrustSettings.dwFlags = WSS_VERIFY_SPECIFIC | WSS_GET_SECONDARY_SIG_COUNT;
		winTrustSettings.dwIndex = 0;
		winTrustData.pSignatureSettings = &winTrustSettings;

		//
		// Verify the primary embedded signature of a file.
		//
		
		// According to MSDN, WinVerifyTrust return a LONG that cannot be cast into a HRESULT!
		if (WinVerifyTrust(NULL, &guidAction, &winTrustData) != 0)
			__leave;

		_tprintf_s(TEXT("\t[+] The file %s is signed with an embedded signature.\n"), szFile);
		bSigned = TRUE;

	}
	__finally {

		winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &guidAction, &winTrustData);

		return bSigned;

	}
}

BOOLEAN U_SIGNATURE::VerifyCatalogSignature(_In_ PCWSTR szFile, _In_ HANDLE hFile)
/**
 * @brief      Verify the signature of 'szFile' by looking at catalogs files.
 *
 * @param[in]  "szFile" - Name of the file to check.
 * @param[in]  "hFile"  - Handle of the file to check
 *
 * @return     TRUE  - File signed
 *             FALSE - File not signed
*/
{
	BOOLEAN bSigned = FALSE;
	HCATADMIN hCatAdmin = NULL;
	HCATINFO hCatInfo = NULL;
	CATALOG_INFO catalogInfo = { 0 };
	DWORD dwHashLength = 0;
	PBYTE pFileHash = nullptr;

	__try {

		// Acquire catalog context (signature algorithm, ...)
		if (CryptCATAdminAcquireContext2(&hCatAdmin, NULL, NULL, NULL, 0) == NULL) {
			_tprintf_s(TEXT("[-] Error CryptCATAdminAcquireContext2\n"));
			__leave;
		}

		// First call for the size of the HASH
		if (CryptCATAdminCalcHashFromFileHandle2(hCatAdmin, hFile, &dwHashLength, NULL, NULL) == NULL) {
			_tprintf_s(TEXT("[-] Error CryptCATAdminCalcHashFromFileHandle2\n"));
			__leave;
		}

		// Allocate memory
		pFileHash = (PBYTE)calloc(dwHashLength, sizeof(BYTE));
		if (pFileHash == NULL) {
			_tprintf_s(TEXT("[-] Error allocating memory\n"));
			__leave;
		}

		// Second call to get the hash
		if (CryptCATAdminCalcHashFromFileHandle2(hCatAdmin, hFile, &dwHashLength, pFileHash, NULL) == NULL) {
			_tprintf_s(TEXT("[-] (2) Error CryptCATAdminCalcHashFromFileHandle2\n"));
			__leave;
		}
		
		
		//
		// Check if the hash is in a catalog
		//

		// get first catalog containing hash
		hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, pFileHash, dwHashLength, 0, &hCatInfo);
		if (hCatInfo == NULL) {
			// No catalog? --> exit
			_tprintf_s(TEXT("\t[i] File: %s, Hash not found in catalogs\n"), szFile);
			__leave;
		}

		catalogInfo.cbStruct = sizeof(CATALOG_INFO);

		// Get catalog info (just for information purpose) --> printf
		if (CryptCATCatalogInfoFromContext(hCatInfo, &catalogInfo, 0)) {
			_tprintf_s(TEXT("[-] Error CryptCATCatalogInfoFromContext\n"));
			__leave;
		}

		_tprintf_s(TEXT("\t[+] File: %s, Hash found in catalogs %s\n"), szFile, catalogInfo.wszCatalogFile);
		bSigned = TRUE;

	}
	__finally {

		if (hCatInfo != NULL)
			CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);

		if (hCatAdmin != NULL)
			CryptCATAdminReleaseContext(hCatAdmin, 0);

		return bSigned;

	}
}
