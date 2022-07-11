/**
 * @file		K_Driver.hpp
 * @brief		Header for K_Driver.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
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