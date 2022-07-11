/**
 * @file		K_Communication.cpp
 * @brief		Create the communication server, send data to the user-mode service once a client is connected
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#include "K_Communication.hpp"
#ifdef _DEBUG
#include "K_Communication.tmh"
#endif


extern DRIVER_DATA globalDriverData;


NTSTATUS K_COMMUNICATION::Initialize(_In_ PCWSTR szPort, _In_ LONG lMaxClients)
/**
 * @brief		Initialize the communicaton.
 *
 * @param[in]	"szPort"		- Port communication handle.
 * @param[in]	"lMaxClients"	- Number of clients allowed to connect simultanely to the service.
 *
 * @return		STATUS_SUCCESS	- No error occurs.\n
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES oa = { 0 };
	PSECURITY_DESCRIPTOR pSd = nullptr;
	UNICODE_STRING szMyPort = { 0 };

	PAGED_CODE();

	__try {

#ifdef _DEBUG
		TraceEvents(TRACE_LEVEL_INFORMATION, COMM, "[i] Communication Initialisation ..");
#endif

		// Set with the conf
		RtlInitUnicodeString(&szMyPort, szPort);
		if (szMyPort.Length == 0)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, COMM, "[!] Error the communication port is empty.");
#endif
			__leave;
		}

		status = FltBuildDefaultSecurityDescriptor(&pSd, FLT_PORT_ALL_ACCESS);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, COMM, "[!] Error creating the security descriptor.");
#endif
			__leave;
		}

		InitializeObjectAttributes(&oa,
			&szMyPort,
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
			NULL,
			pSd
		);

		// Create the communication port
		status = FltCreateCommunicationPort(
			globalDriverData.pFilter,
			&this->_pServerPort,
			&oa,
			NULL,
			_NewConnectionCallback, 
			_DisconnectionCallback, 
			NULL,												// No NewMessage callback needed as we only expect replies
			lMaxClients
		);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, COMM, "[!] Error creating the communication port.");
#endif
			__leave;
		}

	}
	__finally {

		if (pSd != NULL) 
			FltFreeSecurityDescriptor(pSd);

		return status;

	}
}

NTSTATUS K_COMMUNICATION::_NewConnectionCallback(_In_ PFLT_PORT pClientPort, _In_ PVOID pServerPortCookie, _In_ PVOID pConnectionContext, _In_ ULONG ulSizeOfContext, _Out_ PVOID* pConnectionPortCookie)
/**
 * @brief		The connection notify callback is called when a user mode application connect to the server port.
 *
 * @param[in]	"pClientPort"			- Client port that is established between the user-mode application and the kernel-mode minifilter driver.
 * @param[in]	"pServerPortCookie"		- Pointer to context information defined by the minifilter driver (can be used to distinguish multiple communication port from same driver).
 * @param[in]	"pConnectionContext"	- Context information pointer that the user-mode application passed in the lpContext parameter.
 * @param[in]	"ulSizeOfContext"		- Size of the context
 * @param[out]	"pConnectionPortCookie"	- Pointer to information that uniquely identifies this client port.
 *
 * @return		STATUS_SUCCESS	- No error occurs.\n
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	DBG_UNREFERENCED_PARAMETER(pServerPortCookie);
	DBG_UNREFERENCED_PARAMETER(pConnectionContext);
	DBG_UNREFERENCED_PARAMETER(ulSizeOfContext);
	DBG_UNREFERENCED_PARAMETER(pConnectionPortCookie);

	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();

#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_ERROR, COMM, "[+] New client connected.");
#endif

	globalDriverData.pClientPort = pClientPort;

	// You can't send data from this callback. /!\ Undocumented issue /!\

	return status;
}

VOID K_COMMUNICATION::_DisconnectionCallback(_In_ PVOID pConnectionCookie)
/**
 * @brief		The disconnection notify callback is called when a user mode application disconnect from the server port.
 *
 * @param[in]	"pConnectionCookie" - Pointer to information that uniquely identifies this client port.
 *
 * @return		STATUS_SUCCESS	- No error occurs.\n
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	DBG_UNREFERENCED_PARAMETER(pConnectionCookie);

	PAGED_CODE();

#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_ERROR, COMM, "[+] Client disconnected.");
#endif

	FltCloseClientPort(globalDriverData.pFilter, &globalDriverData.pClientPort);
	
	return;
}

NTSTATUS K_COMMUNICATION::Send(_In_ PDATA_TRANSMIT pData, _Out_ DATA_REPLY* pReply)
/**
 * @brief		This function is used to send Data to the User-Mode service.
 *
 * @param[in]	"pData"		- Structure to be transmited to the user-mode service
 * @param[out]	"pReply"	- Structure used by the user-mode service to reply
 *
 * @return		STATUS_SUCCESS	- No error occurs.\n
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER liTimeout = { 0 };
	ULONG ulReplyLength = REPLY_SIZE;
	DATA_REPLY Reply = { 0 };

	__try {

#ifdef _DEBUG
		TraceEvents(TRACE_LEVEL_ERROR, COMM, "[!] Data content: FileName: %ws.", pData->wFileName);
#endif

		// Max response time before timeout
		liTimeout.QuadPart = -((LONGLONG)1 * 10 * 1000 * 1000); // 1 second

		// Send message
		status = FltSendMessage(globalDriverData.pFilter,
			&globalDriverData.pClientPort,
			pData,
			sizeof(DATA_TRANSMIT),
			(PVOID)&Reply,
			&ulReplyLength,
			&liTimeout
		);
		if (status == STATUS_SUCCESS)
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, COMM, "[i] Message sended to user-mode, status 0x%X\n", status);
#endif
		} else {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, COMM, "[-]  Couldn't send message to user-mode file, status 0x%X\n", status);
#endif
		}

		* pReply = Reply;

	}
	__finally {

		return status;

	}
}

VOID K_COMMUNICATION::Unload()
/**
 * @brief		Unload the communication. It is called by Filter::Unload/.
 *
 * @param		None.
 *
 * @return		None.
*/
{
	if (globalDriverData.pClientPort != nullptr) 
		FltCloseCommunicationPort(globalDriverData.pClientPort);

	if (this->_pServerPort != nullptr) 
		FltCloseCommunicationPort(this->_pServerPort);
}