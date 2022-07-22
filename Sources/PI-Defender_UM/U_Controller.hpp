/**
 * @file       U_Controller.hpp
 * @brief      Header for U_Controller.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#ifndef SVCCONTROLLER_HPP
#define SVCCONTROLLER_HPP
#pragma once

#pragma comment(lib, "PathCch.lib" )


#include "U_GlobalHeader.hpp"
#include <PathCch.h>
#include "U_Helper.hpp"


#define GROUP			"FSFilter Anti-Virus"
#define DEPEND_ON_SERVICE	"FltMgr"


class U_CONTROLLER
{
	public:

		static BOOLEAN InstallService(_In_ PCWSTR szServiceName, _In_ BOOLEAN bDriver);
		static BOOLEAN IsServiceInstalled(_In_ PCWSTR szServiceName);

		static DWORD GetServiceState(_In_ PCWSTR szServiceName);
		static BOOLEAN LaunchService(_In_ PCWSTR szServiceName);
		static BOOLEAN StopService(_In_ PCWSTR szServiceName);
		static BOOLEAN QueryService(_In_ PCWSTR szServiceName);
		static BOOLEAN DelService(_In_ PCWSTR szServiceName);

		static VOID WINAPI SvcCtrlHandler(_In_ DWORD dwCtrl);
		static VOID SvcReportStatus(_In_ SERVICE_STATUS SvcStatus, _In_ SERVICE_STATUS_HANDLE SvcStatusHandle, _In_ DWORD dwCurrentState, _In_ DWORD dwWin32ExitCode, _In_ DWORD dwWaitHint);
		
	private:

		static SC_HANDLE _GetSCMHandle();
		static SC_HANDLE _GetServiceHandle(_In_ SC_HANDLE hSCManager, _In_ PCWSTR szServiceName);

};

#endif
