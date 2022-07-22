/**
 * @file       U_Configuration.hpp
 * @brief      Header for U_Configuration.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP
#pragma once

#pragma comment(lib, "PathCch.lib" )


#include "U_GlobalHeader.hpp"

#include <PathCch.h>
#include <Shlobj_core.h>

#include "U_Registry.hpp"
#include "U_Service.hpp"
#include "U_Logs.hpp"
#include "U_Helper.hpp"


//
// Registry Keys (paths)
//

#define U_CONFIGURATION_DRIVER_HIVE		TEXT("SYSTEM\\CurrentControlSet\\Services\\PI-Defender\\Parameters")
#define U_CONFIGURATION_SERVICE_HIVE		TEXT("SYSTEM\\CurrentControlSet\\Services\\PI-Defender_UM\\Parameters")
#define U_CONFIGURATION_COMMUNICATION_PORT_KEY	TEXT("CommunicationPort")
#define U_CONFIGURATION_MAX_CLIENTS_KEY		TEXT("MaxClients")
#define U_CONFIGURATION_WHITELIST_KEY		TEXT("Whitelist")
#define U_CONFIGURATION_LISTENER_THREADS_KEY	TEXT("ListenerThreads")
#define U_CONFIGURATION_MIN_WORKER_THREADS_KEY	TEXT("MinWorkerThreads")
#define U_CONFIGURATION_MAX_WORKER_THREADS_KEY	TEXT("MaxWorkerThreads")
#define U_CONFIGURATION_CACHE_SIZE_KEY		TEXT("CacheSize")


//
// Default PI-Defender Registry Values
//

#define COMMUNICATIONPORT_REG_DEFEAULT_VALUE	TEXT("\\PIDefenderPort")
#define MAXCLIENTS_REG_DEFAULT_VALUE		1

//
// Default PI-Defender_UM Registry Values
//

#define LISTENER_THREADS_REG_DEFAULT_VALUE	1
#define MIN_WORKER_THREADS_REG_DEFAULT_VALUE	1
#define MAX_WORKER_THREADS_REG_DEFAULT_VALUE	10
#define CACHE_SIZE_REG_DEFAULT_VALUE		100


// Custom WHITELIST structure that contains a dynamically allocated WCHARs* and pointers of WCHARs* arrays and its number of elements
typedef struct _WHITELIST {
	PCWSTR* szBuffer;	// Array of PWCSTRs
	int iLength;		// Number of elements in Buffer
} WHITELIST, * PWHITELIST;




class U_CONFIGURATION
{
public:

	static LSTATUS SetServiceConfiguration();
	static LSTATUS SetDriverConfiguration();
	static VOID PrintConfiguration();

	static LSTATUS DeleteLogsRegistry();
	static DWORD GetListenerThreads();
	static DWORD GetMinWorkerThreads();
	static DWORD GetMaxWorkerThreads();

	static DWORD GetMaxClients();
	static PCWSTR GetCommunicationPort();
	static PWHITELIST GetWhitelist();
	static DWORD GetCacheSize();

private:

	static LSTATUS _CreateRegistryForLogs();

};


#endif // !"CONFIGURATION_HPP"
