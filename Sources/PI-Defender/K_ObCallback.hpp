/**
 * @file		K_ObCallback.hpp
 * @brief		Header for K_ObCallback.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef OBCALLBACK_HPP
#define OBCALLBACK_HPP
#pragma once

#include "K_GlobalHeader.hpp"
#include "K_Helper.hpp"
#include "K_Communication.hpp"
#include "K_Configuration.hpp"


#define NumberOfOperations		2
#define OB_ALTITUDE				L"1000"
#define PROCESS_VM_OPERATION	0x0008
#define PROCESS_VM_WRITE		0x0020
#define ACCESS_BITS_REMOVED		(PROCESS_VM_WRITE | PROCESS_VM_OPERATION)


class K_OB
{
public:

	NTSTATUS Initialize();
	static VOID Unload();

private:

	static BOOLEAN _Checks(_In_ PVOID pPreOperationInfo);
	static OB_PREOP_CALLBACK_STATUS _PreObCallback(_In_ PVOID pRegistrationContext, _In_ POB_PRE_OPERATION_INFORMATION pPreInfo);

};

#endif // !OBCALLBACK_HPP