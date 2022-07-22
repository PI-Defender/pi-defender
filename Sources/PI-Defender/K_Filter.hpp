/**
 * @file        K_Filter.hpp
 * @brief       Header for K_Filter.cpp
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


#ifndef FILTER_HPP
#define FILTER_HPP
#pragma once


#include "K_GlobalHeader.hpp"
#include <ntstrsafe.h>							// Send message
#include "K_Helper.hpp"
#include "K_Configuration.hpp"
#include "K_Communication.hpp"


class K_FILTER
{
	public:

		NTSTATUS Initialize(_In_ PDRIVER_OBJECT pDriverObject);
		static NTSTATUS Unload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags);

};


#endif // !FILTER_HPP
