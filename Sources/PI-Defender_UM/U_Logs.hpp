/**
 * @file       U_Logs.hpp
 * @brief      Header for U_Logs.cpp
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#ifndef LOGS_HPP
#define LOGS_HPP
#pragma once

#include "U_GlobalHeader.hpp"
#include "U_Registry.hpp"
#include "U_Helper.hpp"

// Messages File for ETW (see PI-Defender_UM.mc)

//
// MessageId: 1
// Message Text: 
// Language: English
// PIDEFENDER_CATEGORY_GENERAL
//

#define PIDEFENDER_CATEGORY_GENERAL 1

//
// MessageId: 100
// Language: English
// Message Text: Task % 1 (% 2) completed succesfully.
// TASK_OK
//

#define TASK_OK 100


//
// MessageId: 200
// Language: English
// Message Text: Task % 1 (% 2) failed to complete due to error "%3".
// TASK_ERROR
//

#define TASK_ERROR 200

//
// MessageId: 300
// Language: English
// Message Text: Task Information : % 1
// TASK_INFO
//

#define TASK_INFO 300


class U_LOGS
{
	public:

		static BOOLEAN Initialize(_In_ PCWSTR szSourceName);
		static BOOLEAN Unload();
		static VOID SvcReportEvent(_In_ PCWSTR szFunction, _In_ INT iEventType, _In_ DWORD dwEventId, _In_ PCWSTR szComment);

	private:

		static HANDLE _hEventLog;

};


#endif // !LOGS_HPP
