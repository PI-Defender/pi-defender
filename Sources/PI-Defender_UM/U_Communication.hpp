/**
 * @file       U_Communication.hpp
 * @brief      Header for U_Communication.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP
#pragma once
#pragma comment(lib, "ntdll.lib")

#include "U_GlobalHeader.hpp"

#include "U_Service.hpp"
#include "U_Logs.hpp"
#include "../SharedDefs/SharedDefs.hpp"
#include "U_Signature.hpp"

#include <winternl.h>


#define REPLY_SIZE (sizeof(FILTER_REPLY_HEADER) + sizeof(BOOLEAN))


class U_COMMUNICATION
{
	public:

		typedef struct _MESSAGE {
			FILTER_MESSAGE_HEADER messageHeader;	// Required structure header
			DATA_TRANSMIT Data;			// Custom data structure
			OVERLAPPED Ovlp;			// Required structure when dealing with asynchronous message handling
		} MESSAGE, * PMESSAGE;

		typedef struct _THREAD_PARAMETERS {
			HANDLE hPort;
			HANDLE hCompletion;
			PMESSAGE Msg;
			DWORD dwMinWorkerThreads;
			DWORD dwMaxWorkerThreads;
		} THREAD_PARAMETERS, * PTHREAD_PARAMETERS;

		U_COMMUNICATION() {
			_hPort		= INVALID_HANDLE_VALUE;
			_hCompletion	= INVALID_HANDLE_VALUE;
		};

		_Success_(return == S_OK) HRESULT Connect(_In_ PWSTR szPortName, _In_ DWORD dwListeningThreads, _Out_ HANDLE * hCompletion, _Out_ HANDLE * hPort);
		static VOID ListenerThread(_In_ LPVOID lpParam);
		VOID Disconnect();

	private:

		typedef struct _REPLY_MESSAGE
		{
			FILTER_REPLY_HEADER ReplyHeader;	// Required structure header
			DATA_REPLY DataReply;			// Is the exe provided is signed ?
		} REPLY_MESSAGE, * PREPLY_MESSAGE;

		HANDLE _hPort;
		HANDLE _hCompletion;

		static VOID CALLBACK _WorkerThread(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_opt_ PVOID Context, _Inout_ PTP_WORK Work);
		static _Success_(return) VOID _CreateThreadPool(_In_ DWORD dwMinWorkerThreads, _In_ DWORD dwMaxWorkerThreads, _Out_ TP_CALLBACK_ENVIRON * pEnv);
		static VOID _Reply(_In_ HANDLE hPort, _In_ PMESSAGE pMsg, _In_ DATA_REPLY DataToReply);
		static VOID _PumpMessageInQueue(_In_ HANDLE hPort, _In_ PMESSAGE pMsg);
		
};


#endif	// !COMMUNICATION_HPP
