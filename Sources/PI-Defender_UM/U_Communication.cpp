/**
 * @file		U_Communication.cpp
 * @brief		Manage the communication with the driver (Connect|Reply|ListenerThread|WorkerThread)
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#include "U_Communication.hpp"


_Success_(return == S_OK) HRESULT U_COMMUNICATION::Connect(_In_ PWSTR szPortName, _In_ DWORD dwListeningThreads, _Out_ HANDLE* hCompletion, _Out_ HANDLE* hPort)
/**
 * @brief		Connection to the driver.
 *
 * @param[in]	"szPortName" - Communication port name.
 * @param[in]	"dwListeningThreads" - Number of clients.
 * @param[out]	"hCompletion" - Handle of the completion port.
 * @param[out]	"hPort" - Handle of the created communication port.
 *
 * @return		S_OK - No error occurs.\n
 *				!= S_OK - Error occurs.
*/
{
	HRESULT hResult = S_OK;
	HANDLE hMyPort = INVALID_HANDLE_VALUE;
	HANDLE hMyCompletion = INVALID_HANDLE_VALUE;

	__try {

		// Connect to the driver
		hResult = FilterConnectCommunicationPort(
			szPortName,
			0,
			NULL,
			0,
			NULL,
			&hMyPort
		);
		if (FAILED(hResult))
		{
			U_LOGS::SvcReportEvent(TEXT("FilterConnectCommunicationPort"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to initialize the connection to the server"));
			__leave;
		}
		 
		// Completion Port to queue message received from the kernel driver
		hMyCompletion = CreateIoCompletionPort(hMyPort, NULL, 0, dwListeningThreads);
		if (hMyCompletion == INVALID_HANDLE_VALUE)
		{
			U_LOGS::SvcReportEvent(TEXT("CreateIoCompletionPort"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to create message queue"));
			__leave;
		}

		*hCompletion = hMyCompletion;
		*hPort = hMyPort;

	}
	__finally {

		return hResult;

	}
}

VOID U_COMMUNICATION::ListenerThread(_In_ LPVOID lpParam)
/**
 * @brief		Thread listening message sends from kernel driver.
 *
 * @param[in]	"lpParam" - Thread parameters pass to the function (Completion & Client Port).
 *
 * @return		None.
*/
{
	BOOLEAN bResult = TRUE;
	HRESULT hResult = S_OK;
	DWORD dwOutsize = 0;
	ULONG_PTR uKey = NULL;
	LPOVERLAPPED pOvlp = NULL;
	PTHREAD_PARAMETERS Parameters = NULL;
	
	// Thread Pool
	PTP_WORK_CALLBACK WorkCallback = _WorkerThread;
	PTP_WORK Work = NULL;
	TP_CALLBACK_ENVIRON ThreadEnv = { 0 };

	REPLY_MESSAGE Reply = { 0 };

	Parameters = (PTHREAD_PARAMETERS)lpParam;

	U_LOGS::SvcReportEvent(const_cast <LPTSTR> (TEXT("ListeningThread")), EVENTLOG_INFORMATION_TYPE, TASK_OK, TEXT("Created"));

	if (Parameters->hCompletion == NULL)
	{
		U_LOGS::SvcReportEvent(const_cast <LPTSTR> (TEXT("hCompletion")), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Empty"));
		return;
	}

	if (Parameters->hPort == NULL)
	{
		U_LOGS::SvcReportEvent(const_cast <LPTSTR> (TEXT("hPort")), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Empty"));
		return;
	}

	U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Waiting for message..."));

	__try {

		//
		// Create thread pool to verify signature faster
		//

		_CreateThreadPool(Parameters->dwMinWorkerThreads, Parameters->dwMaxWorkerThreads, &ThreadEnv);
		if (ThreadEnv.Size == 0)
		{
			U_LOGS::SvcReportEvent(TEXT("CreateThreadPool"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("ThreadEnv is empty"));
			__leave;
		}

		Parameters->Msg = (PMESSAGE)HeapAlloc(GetProcessHeap(), 0, sizeof(MESSAGE));
		if (Parameters->Msg == NULL)
		{
			U_LOGS::SvcReportEvent(TEXT("malloc"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to alloc memory for the message"));
			__leave;
		}

		FillMemory(&Parameters->Msg->Ovlp, sizeof(OVERLAPPED), 0);

		hResult = FilterGetMessage(
			Parameters->hPort,
			&Parameters->Msg->messageHeader,
			FIELD_OFFSET(MESSAGE, Ovlp),
			&Parameters->Msg->Ovlp
		);

		if (hResult == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			hResult = S_OK;
		else
		{
			U_HELPER::DisplayError(TEXT("FilterGetMessage"));
			HeapFree(GetProcessHeap(), 0, Parameters->Msg);
			__leave;
		}

		while (TRUE)
		{

			//
			// Dequeue message
			//

			Parameters->Msg = NULL;

			bResult = GetQueuedCompletionStatus(Parameters->hCompletion, &dwOutsize, &uKey, &pOvlp, INFINITE);
			if (!bResult)
			{
				hResult = HRESULT_FROM_WIN32(GetLastError());

				if ((hResult == E_HANDLE) || (hResult == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0)))
					hResult = S_OK;

				U_HELPER::DisplayError(TEXT("GetQueuedCompletionStatus"));
				U_LOGS::SvcReportEvent(TEXT("GetQueuedCompletionStatus"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unable to dequeue message"));

				__leave;
			}

			Parameters->Msg = CONTAINING_RECORD(pOvlp, MESSAGE, Ovlp);
			if (Parameters->Msg == NULL)
			{
				U_LOGS::SvcReportEvent(TEXT("CONTAINING_RECORD"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("?"));
				__leave;
			}

			//
			// HANDLE MESSAGE HERE
			//

			U_LOGS::SvcReportEvent(TEXT("Message received from driver"), EVENTLOG_INFORMATION_TYPE, TASK_INFO, Parameters->Msg->Data.wFileName);

			Work = CreateThreadpoolWork(WorkCallback, (PVOID)Parameters, &ThreadEnv);
			if (Work == NULL)
			{
				U_LOGS::SvcReportEvent(TEXT("CreateThreadpoolWork"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT(""));
				__leave;
			}

			SubmitThreadpoolWork(Work);

			WaitForThreadpoolWorkCallbacks(Work, FALSE);

			CloseThreadpoolWork(Work);

		}

		if (Parameters->Msg != NULL)
			HeapFree(GetProcessHeap(), 0, Parameters->Msg);

	}
	__finally {

		if (IS_ERROR(hResult))
		{
			if (hResult == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) 
				U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Port is disconnected, probably due to filter unloading"));
			else 
				U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Unknown error occured"));
		}

		return;

	}
}

_Success_(return) VOID U_COMMUNICATION::_CreateThreadPool(_In_ DWORD dwMinWorkerThreads, _In_ DWORD dwMaxWorkerThreads, _Out_ TP_CALLBACK_ENVIRON* pEnv)
/**
 * @brief		Creation of a thread pool used to create worker thread to check file signature.
 *
 * @param[in]	"dwMinWorkerThreads"	- Minimum number of worker threads.
 * @param[in]	"dwMaxWorkerThreads"	- Maximum number of worker threads.
 * @param[out]	"pEnv"					- Thread Pool environment.
 *
 * @return		None.
*/
{
	PTP_WORK Work = NULL;
	PTP_POOL ThreadPool = NULL;
	PTP_CLEANUP_GROUP cleanupGroup = NULL;
	TP_CALLBACK_ENVIRON ThreadEnv = { 0 };

	*pEnv = { 0 };

	__try {

		ThreadPool = CreateThreadpool(NULL);
		if (ThreadPool == NULL)
		{
			U_LOGS::SvcReportEvent(TEXT("CreateThreadpool"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT(""));
			__leave;
		}

		cleanupGroup = CreateThreadpoolCleanupGroup();
		if (cleanupGroup == NULL)
		{
			U_LOGS::SvcReportEvent(TEXT("CreateThreadpoolCleanupGroup"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT(""));
			__leave;
		}

		// Configure the pool
		SetThreadpoolThreadMaximum(ThreadPool, dwMaxWorkerThreads);
		SetThreadpoolThreadMinimum(ThreadPool, dwMinWorkerThreads);
		InitializeThreadpoolEnvironment(&ThreadEnv);
		SetThreadpoolCallbackPool(&ThreadEnv, ThreadPool);
		SetThreadpoolCallbackCleanupGroup(&ThreadEnv, cleanupGroup, NULL);

		*pEnv = ThreadEnv;

	}
	__finally {

		return;

	}
}

VOID CALLBACK U_COMMUNICATION::_WorkerThread(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_opt_ PVOID Context, _Inout_ PTP_WORK Work)
/**
 * @brief			Worker thread used to check file signature.
 *
 * @param[inout]	"Instance"	- Structure that defines the callback instance.
 * @param[inout]	"Context"	- Parameter passed to the thread
 * @param[inout]	"Work"		- Structure that defines the Work object generated by the callback.
 *
 * @return			None.
*/
{
	HANDLE hFileToCheck = INVALID_HANDLE_VALUE;
	BOOLEAN bResult = FALSE;
	
	// Reply
	DATA_REPLY DataToReply = { 0 };
		
	// File signature
	BOOLEAN bSigned = FALSE;

	// Parameters
	PTHREAD_PARAMETERS Parameters = NULL;

	Parameters = (PTHREAD_PARAMETERS)Context;

	__try {

		if (Parameters == NULL || Parameters->Msg == NULL)
			__leave;

		bResult = U_HELPER::GetHandleFromFileName(&hFileToCheck, Parameters->Msg->Data.wFileName);
		if (!bResult)
		{
			U_LOGS::SvcReportEvent(TEXT("GetHandleFromFileName"), EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT(""));
			__leave;
		}

		//
		// Once the path and the handle are set, we verify the signature
		//

		// First check for embedded signature.
		bSigned = U_SIGNATURE::VerifyEmbeddedSignature(Parameters->Msg->Data.wFileName);
		
		// Else, check if the signature is present in catalog.
		if (!bSigned)
			bSigned = U_SIGNATURE::VerifyCatalogSignature(Parameters->Msg->Data.wFileName, hFileToCheck);

	}
	__finally {

		DataToReply.bIsSigned = bSigned;

		if (hFileToCheck && hFileToCheck != INVALID_HANDLE_VALUE)
			CloseHandle(hFileToCheck);
		
		if (Parameters != NULL && Parameters->Msg != NULL)
		{
			// Reply to the Drive
			_Reply(Parameters->hPort, Parameters->Msg, DataToReply);

			// Pump message into queue again (Get a new message)
			_PumpMessageInQueue(Parameters->hPort, Parameters->Msg);
		}
	}
}

VOID U_COMMUNICATION::_Reply(_In_ HANDLE hPort, _In_ PMESSAGE pMsg, _In_ DATA_REPLY DataToReply)
/**
 * @brief		Reply to the kernel driver.
 *
 * @param[in]	"hPort"			- Communication handle port.
 * @param[in]	"pMsg"			- Structure that will receive the message from the kernel.
 * @param[in]	"DataToReply"	- Structure used to hold the data that will be send to the kernel.
 *
 * @return		None.
*/
{
	HRESULT hResult = S_OK;
	PREPLY_MESSAGE Reply = NULL;

	__try {

		Reply = (PREPLY_MESSAGE)calloc(1, sizeof(REPLY_MESSAGE));

		if (Reply == NULL) 
			__leave;

		Reply->ReplyHeader.MessageId = pMsg->messageHeader.MessageId;
		Reply->ReplyHeader.Status = 0;

		Reply->DataReply.bIsSigned = DataToReply.bIsSigned;
		U_LOGS::SvcReportEvent(TEXT("Reponse envoyé au driver (signe / non signe)"), EVENTLOG_INFORMATION_TYPE, TASK_INFO, (PCWSTR)Reply->DataReply.bIsSigned);

		hResult = FilterReplyMessage(hPort, &Reply->ReplyHeader, REPLY_SIZE);

		if (IS_ERROR(hResult))
		{
			U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_ERROR_TYPE, TASK_ERROR, TEXT("Reply message error"));
			__leave;
		}

		U_LOGS::SvcReportEvent(SERVICE_NAME, EVENTLOG_INFORMATION_TYPE, TASK_INFO, TEXT("Replied Message"));

	}
	__finally {

		return;

	}
}

VOID U_COMMUNICATION::_PumpMessageInQueue(_In_ HANDLE hPort, _In_ PMESSAGE pMsg)
/**
 * @brief		Push a new message in the queue.
 *
 * @param[in]	"hPort"	- Communication handle port.
 * @param[in]	"pMsg"	- Structure that defines the data transmitted from kernel <-> user-mode .
 *
 * @return		None.
*/
{
	HRESULT hResult = S_OK;

	memset(&pMsg->Ovlp, 0, sizeof(OVERLAPPED));

	hResult = FilterGetMessage(hPort,
		&pMsg->messageHeader,
		FIELD_OFFSET(MESSAGE, Ovlp),
		&pMsg->Ovlp
	);

	if (hResult == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
	{
		hResult = S_OK;
	}
	else {
		U_HELPER::DisplayError(TEXT("FilterGetMessage"));
		HeapFree(GetProcessHeap(), 0, pMsg);
	}

	return;
}

VOID U_COMMUNICATION::Disconnect()
/**
 * @brief		Disconnect the client.
 *
 * @param		None.
 *
 * @return		None.
*/
{
	if (this->_hPort)
		CloseHandle(this->_hPort);

	if (this->_hCompletion)
		CloseHandle(this->_hCompletion);

	return;
}