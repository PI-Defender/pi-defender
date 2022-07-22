/**
 * @file       U_Signature.hpp
 * @brief      Header for U_Signature.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
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

