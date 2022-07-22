/**
 * @file		K_Driver.hpp
 * @brief		Header for K_Driver.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#ifndef DRIVER_HPP
#define DRIVER_HPP
#pragma once

#include "K_GlobalHeader.hpp"
#include "K_Configuration.hpp"
#include "K_Filter.hpp"
#include "K_ObCallback.hpp"


extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING szRegistryPath);
extern "C" VOID DriverUnload(_In_ PDRIVER_OBJECT pDriverObject);


#endif // !DRIVER_HPP