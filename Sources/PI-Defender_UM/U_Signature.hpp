/**
 * @file		U_Signature.hpp
 * @brief		Header for U_Signature.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef SIGNATURE_HPP
#define SIGNATURE_HPP
#pragma once
#pragma comment (lib, "wintrust.lib")

#include "U_GlobalHeader.hpp"

//
// Lib to verify signature
//

#include <wincrypt.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <mscat.h>


class U_SIGNATURE
{
	public:
	
		static BOOLEAN VerifyEmbeddedSignature(_In_ PCWSTR szFile);
		static BOOLEAN VerifyCatalogSignature(_In_ PCWSTR szFile, _In_ HANDLE hFile);
	
};



#endif // !SIGNATURE_HPP

