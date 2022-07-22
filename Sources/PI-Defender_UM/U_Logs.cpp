/**
 * @file       U_Logs.cpp
 * @brief      Report Event to ETW
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/


#include "U_Logs.hpp"


HANDLE U_LOGS::_hEventLog = NULL;


BOOLEAN U_LOGS::Initialize(_In_ PCWSTR szSourceName)
/**
 * @brief      Initialize the service eventlog.
 *
 * @param[in]  "szSourceName" - Name of the source event (service name)
 *
 * @return     TRUE  - No error occurs.
 *             FALSE - Error occurs
*/
{
	_hEventLog = RegisterEventSource(NULL, szSourceName);

	return _hEventLog != NULL;
}

BOOLEAN U_LOGS::Unload()
/**
 * @brief   Close the service eventlog.
 *
 * @param   None.
 *
 * @return  TRUE  - No error occurs.
 *          FALSE - Error occurs.
*/
{
	BOOLEAN bResult = TRUE;
	
	if (_hEventLog != NULL)
		bResult = DeregisterEventSource(_hEventLog);

	return bResult;
}

VOID U_LOGS::SvcReportEvent(_In_ PCWSTR szFunction, _In_ INT iEventType, _In_ DWORD dwEventId, _In_ PCWSTR szComment)
/**
 * @brief      Send the reported event in the Event Viewer (Application -> PI-Defender_UM).
 *
 * @param[in]  "szFunction" - Function causing the error
 * @param[in]  "iEventType" - Type of event (EVENTLOG_ERROR_TYPE, EVENTLOG_AUDIT_FAILURE, EVENTLOG_AUDIT_SUCCESS, EVENTLOG_INFORMATION_TYPE, EVENTLOG_WARNING_TYPE)
 * @param[in]  "szComment"  - Comment to be added to the log.
 *
 * @return     None.
*/
{
	PCWSTR szStrings[2] = { 0 };

	if (_hEventLog != INVALID_HANDLE_VALUE && _hEventLog != NULL)
	{
		szStrings[0] = szFunction;
		szStrings[1] = szComment;

		ReportEvent(_hEventLog, iEventType, PIDEFENDER_CATEGORY_GENERAL, dwEventId, NULL, 2, NULL, szStrings, NULL);
	}
}
