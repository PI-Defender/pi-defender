/**
 * @file		K_Driver.cpp
 * @brief		Entrypoint of the driver, unload routine
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version		1.0
 * @date		02/06/2022
 * @copyright	©Naval Group SA.
 *				This document in its content and form is the property of Naval Group SA and/or third parties.
 *				This project is released under the LGPLv3 license.
*/


#include "K_Driver.hpp"
#ifdef _DEBUG
#include "K_Driver.tmh"
#endif


DRIVER_DATA globalDriverData = { 0 };				// Global structure of the driver (Filter Object, Client port, ObCallback)
K_CONFIGURATION Configuration;						// Needed to unload the configuration in DriverUnload routine
K_OB ObCallback;									// Needed to unload the obcallback in DriverUnload routine


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING szRegistryPath)
/**
 * @brief		Main Driver Entry Routine
 *
 * @param[in]	"pDriverObject"	- Pointer to DRIVER_OBJECT structure
 * @param[in]	"szRegistryPath"- Pointer to UNICODE_STRING structure that specifies the path to the driver in the Registry Key
 *
 * @return		STATUS_SUCCESS	- No error occurs.
 *				Other status	- Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
#ifdef _DEBUG
	WPP_INIT_TRACING(pDriverObject, szRegistryPath);					// Initialize WPP Tracing
#endif

	NTSTATUS status = STATUS_SUCCESS;
	K_FILTER Filter;

	pDriverObject->DriverUnload = DriverUnload;

	__try {
#ifdef _DEBUG
		TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Driver init START ****************");
#endif

		// Get the configuration of the driver set by the user mode service
		status = Configuration.Initialize(szRegistryPath);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, CONF, "[-] Configuration failed to initialize");
#endif
			__leave;
		}

		// Initialize Filter
		status = Filter.Initialize(pDriverObject);
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_ERROR, FLT, "[-] Filter failed to initialize");
#endif
			__leave;
		}

		// Initialize ObCallback
		status = ObCallback.Initialize();
		if (!NT_SUCCESS(status))
		{
#ifdef _DEBUG
			TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "[-] ObCallback failed to initialize");
#endif
		}
		
#ifdef _DEBUG
		TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Driver init END ****************");
#endif

	}
	__finally {

		if (!NT_SUCCESS(status))
		{
			Filter.Unload(NULL);
			DriverUnload(pDriverObject);
		}

		return status;

	}
}


extern "C" VOID DriverUnload(_In_ PDRIVER_OBJECT pDriverObject)
/**
 * @brief		Driver Unloading Routine.\n
 *				[i] FilterUnload is called automatically by the system before DriverUnload
 *
 * @param[in]	"pDriverObject" - Pointer to DRIVER_OBJECT structure.
 *
 * @return		None.
*/
{
#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Driver unload START ****************");
#endif

	Configuration.Unload();
	
	ObCallback.Unload();

#ifdef _DEBUG
	TraceEvents(TRACE_LEVEL_INFORMATION, INIT, "**************** Driver unload END ****************");
#endif

	WPP_CLEANUP(pDriverObject);

	return;
}