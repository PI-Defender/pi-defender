/**
 * @file        K_ObCallback.cpp
 * @brief       Handle request callback
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


#include "K_ObCallback.hpp"
#ifdef _DEBUG
#include "K_ObCallback.tmh"
#endif


extern DRIVER_DATA globalDriverData;
K_COMMUNICATION Communication;


NTSTATUS K_OB::Initialize()
/**
 * @brief   Register the ObCallback routine.\n
 *          /!\ In order to register the callback you must have:
 *             - a signed driver
 *             - /integritycheck parameter added in the linker
 *          Else, you'll get '0xC0000022' error
 *
 * @param   None.
 *
 * @return  STATUS_SUCCESS - No error occurs.
 *          Other status   - Error status. For more information, see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
*/
{
    NTSTATUS status = STATUS_SUCCESS;
    OB_CALLBACK_REGISTRATION ObRegistration = { 0 };
    OB_OPERATION_REGISTRATION ObOperationRegistrations[NumberOfOperations] = { {0}, {0} };
    UNICODE_STRING szAltitude = { 0 };

    __try {

        ObOperationRegistrations[0].ObjectType = PsProcessType;
        ObOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
        ObOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
        ObOperationRegistrations[0].PreOperation = _PreObCallback;
        ObOperationRegistrations[0].PostOperation = NULL;

        ObOperationRegistrations[1].ObjectType = PsThreadType;
        ObOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_CREATE;
        ObOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
        ObOperationRegistrations[1].PreOperation = _PreObCallback;
        ObOperationRegistrations[1].PostOperation = NULL;

        RtlInitUnicodeString(&szAltitude, OB_ALTITUDE);

        ObRegistration.Version = OB_FLT_REGISTRATION_VERSION;
        ObRegistration.OperationRegistrationCount = NumberOfOperations;
        ObRegistration.Altitude = szAltitude;
        ObRegistration.RegistrationContext = NULL;
        ObRegistration.OperationRegistration = ObOperationRegistrations;


        status = ObRegisterCallbacks(&ObRegistration, &globalDriverData.pObCallback);
        if (!NT_SUCCESS(status))
        {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_ERROR, FLT, "[!] Error ObRegisterCallbacks 0x%x.", status);
#endif
            __leave;
        }

    }
    __finally {

        return status;

    }
}


OB_PREOP_CALLBACK_STATUS K_OB::_PreObCallback(_In_ PVOID pRegistrationContext, _In_ POB_PRE_OPERATION_INFORMATION pPreOperationInfo)
/**
 * @brief       We get notify when a process want a handle.\n
 *              We communicate with the user-mode service to know if this process is signed.\n
 *              If the process is signed, we don't filter.\n
 *              Else, we filter it and remove 'PROCESS_VM_WRITE' and 'PROCESS_VM_OPERATION' to the desired access.
 *
 * @param[in]   "pRegistrationContext"   - Context that the driver want to pass to this callback.
 * @param[in]   "pPreOperationInfo"      - Pointer to OB_PRE_OPERATION_INFORMATION structure, that receive information about the handle created.
 *
 * @return      OB_PREOP_SUCCESS - Pre Operation callback finished.
*/
{
    DBG_UNREFERENCED_PARAMETER(pRegistrationContext);

    NTSTATUS status = STATUS_SUCCESS;;
    HANDLE hTargetPID = NULL;
    PACCESS_MASK pDesiredAccessMask = nullptr;
#ifdef _DEBUG
    ACCESS_MASK GrantedAccessMask = { 0 };
#endif
    UNICODE_STRING szNtPath = { 0 }, szDosPath = { 0 };
    PDATA_TRANSMIT pData = nullptr;
    DATA_REPLY Reply = { 0 };
    BOOLEAN bResult, bIsSigned = FALSE;
    PVOID pHash = nullptr;
    ULONG ulHashSize = 0;

    __try {
        
        // Basic checks: IRQL, Client connected, Kernel events
        if (_Checks(pPreOperationInfo) != TRUE)
            __leave;

        // Filter processes only
        if (pPreOperationInfo->ObjectType != *PsProcessType)
            __leave;

        // Don't filter process doing stuff on theirselves
        if (pPreOperationInfo->Object == PsGetCurrentProcess())
            __leave;


        hTargetPID = PsGetProcessId((PEPROCESS)pPreOperationInfo->Object);

        switch (pPreOperationInfo->Operation)
        {
            case OB_OPERATION_HANDLE_CREATE:
                pDesiredAccessMask = &pPreOperationInfo->Parameters->CreateHandleInformation.DesiredAccess;
                break;

            case OB_OPERATION_HANDLE_DUPLICATE:
                pDesiredAccessMask = &pPreOperationInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
                break;

            default:
                pDesiredAccessMask = nullptr;
        }

        if (pDesiredAccessMask == nullptr)
            __leave;


        // Leave if desired access mask does not contain watched access rights (we don't care)
        if ((*pDesiredAccessMask & ACCESS_BITS_REMOVED) == 0)
            __leave;

        //
        // Check Whitelist
        //

        // Get Process Name
        szNtPath.Length = 0;
        szNtPath.MaximumLength = TRIPLE_MAX_PATH;
        szNtPath.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, TRIPLE_MAX_PATH, DRIVER_TAG);

        status = K_HELPER::GetFullImageNameByHandle(PsGetCurrentProcessId(), &szNtPath);
        if (!NT_SUCCESS(status))
        {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_ERROR, INIT, "GetFullImageNameByHandle failed");
#endif
            __leave;
        }


        // Check whitelist
        status = K_CONFIGURATION::IsInWhitelist(&szNtPath, &bResult);
        if (!NT_SUCCESS(status))
        {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_ERROR, WLIST, "[-] CONFIGURATION::IsInWhitelist error: 0x%8.8X", status);
#endif
            __leave;
        }

        if (bResult != FALSE) {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_INFORMATION, WLIST, "[!] Whitelist contains %wZ --> stop", &szNtPath);
#endif
            __leave;
        }

        //
        // Check Cache
        //

        // Compute file hash
        // /!\ GetFileHash Allocate hash memory ! Don't forget to free after use!
        status = K_HELPER::GetFileHash(&szNtPath, &pHash, &ulHashSize);
        if (!NT_SUCCESS(status))
        {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_INFORMATION, OBCB, "GetFileHash failed: 0x%8.8X", status);
#endif
            __leave;
        }

        // Check if cache does contains hash
        if (K_CONFIGURATION::IsInCache(ulHashSize, pHash, &bIsSigned))
        {
#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_INFORMATION, CACH, "[!] Cache contains %wZ (%s)", &szNtPath, bIsSigned ? "SIGNED" : "NOT SIGNED");
#endif
        }
        else
        {

            //
            // Send data to user mode (signature)
            //

            // Convert NT path to DOS path
            status = K_HELPER::ConvertNtPathToDosPath(szNtPath, &szDosPath);
            if (!NT_SUCCESS(status))
            {
#ifdef _DEBUG
                TraceEvents(TRACE_LEVEL_INFORMATION, COMM, "ConvertNtPathToDosPath failed: 0x%8.8X", status);
#endif
                __leave;
            }

            // Allocate buffer
            pData = (PDATA_TRANSMIT)ExAllocatePoolWithTag(PagedPool, sizeof(DATA_TRANSMIT), DRIVER_TAG);
            if (pData == nullptr)
            {
#ifdef _DEBUG
                TraceEvents(TRACE_LEVEL_ERROR, COMM, "[-] Unable to allocate memory for the Data structure that will be send.");
#endif
                __leave;
            }

            // Fill data information
            RtlFillMemory(pData, sizeof(DATA_TRANSMIT), 0);

            RtlCopyMemory(pData->wFileName, szDosPath.Buffer, szDosPath.Length);

            if (TRIPLE_MAX_PATH < szDosPath.Length)
            {
#ifdef _DEBUG
                TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "[-] Buffer overflow: %d is higher than %d", szDosPath.Length, TRIPLE_MAX_PATH);
#endif
                __leave;
            }

            // Send data to user-mode service
            Communication.Send(pData, &Reply);

            // Check response to see if executable is signed
            bIsSigned = Reply.bIsSigned;

#ifdef _DEBUG
            TraceEvents(TRACE_LEVEL_INFORMATION, COMM, "[+] Response from User: %s.", bIsSigned ? "SIGNED" : "NOT SIGNED");
#endif

            // Add app hash in cache
            K_CONFIGURATION::AddToCache(ulHashSize, pHash, bIsSigned); // VOID

        }

        //
        // Filter not signed application
        //
        
        // Leave if app is signed
        if (bIsSigned != FALSE)
            __leave;


        // Else, filter access mask!

#ifdef _DEBUG

        GrantedAccessMask = *pDesiredAccessMask;
        GrantedAccessMask &= ~ACCESS_BITS_REMOVED;

        // Reuse DosPath to get target process name (NT Path)
        szDosPath.Length = 0;
        szDosPath.MaximumLength = TRIPLE_MAX_PATH;
        szDosPath.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, TRIPLE_MAX_PATH, DRIVER_TAG);

        status = K_HELPER::GetFullImageNameByHandle(hTargetPID, &szDosPath);
        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, INIT, "GetFullImageNameByHandle failed");
            __leave;
        }
       
        // Only display access mask with watched bits (see ACCESS_BITS_REMOVED)
        TraceEvents(TRACE_LEVEL_INFORMATION, OBCB,
            "[+] ObCallback:\t"
            "Source: %wZ (PID:%u)\t"
            "Dest: %wZ (PID:%u)\t"
            "DesiredAccess (in): 0x%8.8X\t"
            "GrantedAccess (out): 0x%8.8X",
            &szNtPath, PtrToUlong(PsGetCurrentProcessId()),
            &szDosPath, PtrToUlong(hTargetPID),
            *pDesiredAccessMask,
            GrantedAccessMask
        );

        *pDesiredAccessMask = GrantedAccessMask; // Apply filter!

#else

        *DesiredAccessMask &= ~ACCESS_BITS_REMOVED; // Apply filter!

#endif

    }

    __finally {

        if (pData) 
            ExFreePool(pData);

        if (szNtPath.Buffer) 
            ExFreePool(szNtPath.Buffer);

        if (pHash) 
            ExFreePool(pHash);

        return OB_PREOP_SUCCESS;

    }
}


BOOLEAN K_OB::_Checks(_In_ PVOID pOperationInfo)
/**
 * @brieF       Checks if :\n
 *                  - A client is connected
 *                  - The IRQL is at PASSIVE_LEVEL
 *                  - We are dealing with kernel handle
 *              If 'bFlags' is TRUE: Pre Operation Callback
 *              If 'bFlags' is FALSE: Post Operation Callback
 * 
 * @param[in]   "pOperationInfo" - Pointer to a structure that will be cast depending on the bFlags value (see description above)
 *
 * @return      TRUE    - Pass the checks
 *              FALSE   - Blocked
*/
{
    BOOLEAN bResult = FALSE;

    __try {

        // First, check if a client is connected to the server
        if (globalDriverData.pClientPort == NULL)
            __leave;

        // Then, check if we are at PASSIVE_LEVEL
        if (KeGetCurrentIrql() != PASSIVE_LEVEL)
            __leave;

        // Don't filter kernel handle
        if (((POB_PRE_OPERATION_INFORMATION)pOperationInfo)->KernelHandle == TRUE)
            __leave;
       
        bResult = TRUE;

    }
    __finally {

        return bResult;

    }
}


VOID K_OB::Unload()
/**
 * @brief   Routine used to unload ObCallback.
 *
 * @param   None.
 *
 * @return  None.
*/
{
#ifdef _DEBUG
    TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "**************** ObCallback unload START ****************");
#endif

    if (globalDriverData.pObCallback != NULL) 
        ObUnRegisterCallbacks(globalDriverData.pObCallback);

#ifdef _DEBUG
    TraceEvents(TRACE_LEVEL_INFORMATION, FLT, "**************** ObCallback unload END ****************");
#endif

    return;
}