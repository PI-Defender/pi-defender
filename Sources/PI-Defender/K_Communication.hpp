/**
 * @file		K_Communication.hpp
 * @brief		Header for K_Communication.cpp
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP
#pragma once

#include "K_GlobalHeader.hpp"
#include "../SharedDefs/SharedDefs.hpp"


#define REPLY_SIZE sizeof(BOOLEAN)


class K_COMMUNICATION
{
	public:

		VOID Unload();
		NTSTATUS Initialize(_In_ PCWSTR szPort, _In_ LONG lMaxClients);
		NTSTATUS Send(_In_ PDATA_TRANSMIT pData, _Out_ DATA_REPLY* pReply);
	
	private:

		PFLT_PORT _pServerPort;

		static NTSTATUS _NewConnectionCallback(
			_In_ PFLT_PORT pClientPort,
			_In_ PVOID pServerPortCookie,
			_In_ PVOID pConnectionContext,
			_In_ ULONG ulSizeOfContext,
			_Out_ PVOID * pConnectionPortCookie
		);
	
		static VOID _DisconnectionCallback(_In_ PVOID pConnectionCookie);
};

#endif // !COMMUNICATION_HPP