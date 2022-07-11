/**
 * @file		K_GlobalHeader.hpp
 * @brief		Header used by several .cpp files
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/


#ifndef GLOBAL_HEADER_HPP
#define GLOBAL_HEADER_HPP
#pragma once
#pragma comment(lib, "FltMgr.lib") // set in the project properties, else this isn't working.
#pragma comment(lib, "ksecdd.lib") // set in the project properties, else this isn't working.

#include <ntifs.h>
#include <wdm.h>
#include <fltKernel.h>
#include <ntstrsafe.h>

// Enable debugging
#define _DEBUG


#ifdef _DEBUG
#include "K_WppTracing.hpp"
#endif

#include "K_Helper.hpp"


#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define DRIVER_TAG 'D-IP'	// 'PI-D'


struct DRIVER_DATA {
    //  The filter handle that results from a call to FltRegisterFilter.
    PFLT_FILTER pFilter;
    // The port handle that results from a client connection (Connection callback).
    PFLT_PORT pClientPort;
     // The obcallback handle that results from a call to ObCallbackRegister.
    PVOID pObCallback;
};


#endif // !GLOBAL_HEADER_HPP