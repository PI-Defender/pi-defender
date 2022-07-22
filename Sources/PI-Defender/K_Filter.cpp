/**
 * @file		K_Filter.cpp
 * @brief		Initialize filter object used by the communication
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/

#include "K_Filter.hpp"
#ifdef _DEBUG
#include "K_Filter.tmh"
#endif


extern DRIVER_DATA globalDriverData;
extern K_CONFIGURATION Configuration;
extern K_COMMUNICATION Communication;


NTSTATUS K_FILTER::Initialize(_In_ PDRIVER_OBJECT pDriverObject)
/**
 * @brief		Initialize the filter
 *
 * @param[in]	"pDriverObject" - Pointer to DRIVER_OBJECT structure
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	FLT_REGISTRATION Registration = { 0 };
	
	// Set Registration data structure
	Registration.Size = sizeof(FLT_REGISTRATION);
	Registration.Version = FLT_REGISTRATION_VERSION;
	Registration.Flags = 0;
	Registration.ContextRegistration = NULL;
	Registration.OperationRegistration = NULL;
	Registration.FilterUnloadCallback = this->Unload;
	Registration.InstanceSetupCallback = NULL;
	Registration.InstanceQueryTeardownCallback = NULL;
	Registration.InstanceTeardownStartCallback = NULL;
	Registration.InstanceTeardownCompleteCallback = NULL;
	Registration.GenerateFileNameCallback = NULL;
	Registration.NormalizeNameComponentCallback = NULL;
	Registration.NormalizeContextCleanupCallback = NULL;
	Registration.TransactionNotificationCallback = NULL;
	Registration.NormalizeNameComponentExCallback = NULL;
	Registration.SectionNotificationCallback = NULL;

	__try {

		//
		// Register the Driver Object to the FltMgr
		//
		
		status = FltRegisterFilter(pDriverObject, &Registration, &globalDriverData.pFilter);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, FLT, "[-] Failed to register the Driver to the FltMgr.");
#endif
			__leave;
		}

		status = Communication.Initialize(Configuration.GetCommunicationPort(), Configuration.GetMaxClients());
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, FLT, "[-] Communication failed to initialize");
#endif
			__leave;
		}
		
	}
	__finally {

		return status;
	
	}
}


NTSTATUS K_FILTER::Unload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
/**
 * @brief		Unload the communication & the filter
 *
 * @param[in]	Flags - Not used
 *
 * @return		STATUS_SUCCESS - No error occurs.
 *				Other status - Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
	DBG_UNREFERENCED_PARAMETER(Flags);

	NTSTATUS status = STATUS_SUCCESS;

#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Filter unload START ****************");
#endif

	Communication.Unload();

	if (globalDriverData.pFilter != nullptr) 
		FltUnregisterFilter(globalDriverData.pFilter);

#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Filter unload END ****************");
#endif

	return status;
}