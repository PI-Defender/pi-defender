/**
 * @file		Utilitaire.cpp
 * @author		NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

#include "Utilitaire.hpp"

VOID PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...)
{
	if (!lpszFormat || !lpszFormat[0]) return;

	LPCTSTR lpszPrefix = TEXT("");
	LPVOID lpBuf = nullptr;
	va_list list, copy;
	int nSize = 0;

	__try {

		va_start(list, lpszFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(lpszFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) __leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, lpszFormat, list);

		switch (msgType) {
		case MESSAGE_TYPE::TYPE_ERROR:
			lpszPrefix = TEXT("\n\033[31m[-] ");
			break;
		case MESSAGE_TYPE::TYPE_SUCCESS:
			lpszPrefix = TEXT("\033[32m[+] ");
			break;
		default:
			break;
		}

		_tprintf_s(TEXT("%s%s\033[0m\n"), lpszPrefix, (LPCTSTR)lpBuf);
	}

	__finally {
		va_end(list);
		if (lpBuf) LocalFree(lpBuf);
	}
}

VOID DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...)
{
	if (!lpszFormat || !lpszFormat[0]) return;

	int nSize = 0;
	va_list list, copy;
	LPVOID lpBuf = nullptr;
	LPCTSTR lpszPrefix = TEXT("");
	LONG lIcon = 0L;

	__try {

		va_start(list, lpszFormat);
		va_copy(copy, list);

		nSize = _vsctprintf(lpszFormat, copy) + 1;
		va_end(copy);

		if (nSize <= 0) __leave;

		lpBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(TCHAR));
		if (!lpBuf) {
			_tprintf_s(TEXT("\033[31m[-] LocalAlloc failed with error 0x%X\033[0m\n"), GetLastError());
			__leave;
		}

		_vsntprintf_s((LPTSTR)lpBuf, nSize, _TRUNCATE, lpszFormat, list);

		switch (msgType)
		{
		case MESSAGE_TYPE::TYPE_ERROR:
			lpszPrefix = TEXT("\033[31m[-] ");
			lIcon = MB_ICONERROR;
			break;
		case MESSAGE_TYPE::TYPE_SUCCESS:
			lpszPrefix = TEXT("\033[32m[+] ");
			lIcon = MB_ICONINFORMATION;
			break;
		default:
			break;
		}

		PrintMessage(msgType, (LPCTSTR)lpBuf);
		MessageBox(NULL, (LPCTSTR)lpBuf, TEXT("Error"), MB_OK | lIcon);
	}

	__finally {
		va_end(list);
		if (lpBuf) LocalFree(lpBuf);
	}
}

VOID DisplayError(_In_ LPCTSTR lpszFunction)
{
	LPVOID lpMsgBuf = nullptr;
	LPVOID lpDisplayBuf = nullptr;
	DWORD dwLastErrorCode = GetLastError();

	__try {
		if (!FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwLastErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		)) {
			PrintMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("FormatMessage failed with error 0x%X"), GetLastError());
			__leave;
		}

		DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s failed with error 0x%X\n%s"), lpszFunction, dwLastErrorCode, (LPCTSTR)lpMsgBuf);
	}

	__finally {
		if (lpMsgBuf) LocalFree(lpMsgBuf);
	}
}

SC_HANDLE GetSCMHandle()
/*
Description:
	Get a handle to the Service Controller Database

Parameters:
	None

Return:
	SC_HANDLE - No error occurs.
	NULL - Error occurs.
*/
{
	SC_HANDLE schSCManager = nullptr;

	// Get SCM database handle
	schSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, GENERIC_READ);
	if (!schSCManager) 
		DisplayError(TEXT("OpenSCHManager"));

	return schSCManager;
}

DWORD GetServiceState(_In_ LPCTSTR lpszServiceName)
/*
Description:
	Query the service "lpszServiceName" and return its state

Parameters:
	lpszServiceName - Name of the service

Return:
	serviceStatus.dwCurrentState - "lpszServieceName" current state
	0 - Error
*/
{
	DWORD dwStatus = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS serviceStatus;

	__try {
		// Get SCM database handle
		schSCManager = GetSCMHandle();
		if (!schSCManager) {
			DisplayError(TEXT("GetSCMHandle"));
			__leave;
		}

		// Get Service handle, if the service is installed, we get the handle. Else we raise an error.
		schService = OpenService(schSCManager, lpszServiceName, SERVICE_QUERY_STATUS);
		if (!schService) {
			DisplayMessage(MESSAGE_TYPE::TYPE_ERROR, TEXT("%s is not installed."), lpszServiceName);
			__leave;
		}

		// Get service information
		if (!QueryServiceStatus(schService, &serviceStatus)) {
			DisplayError(TEXT("QueryServiceStatus"));
			__leave;
		}

		dwStatus = serviceStatus.dwCurrentState;
	}

	__finally {
		if (schService) 
			CloseServiceHandle(schService);

		if (schSCManager) 
			CloseServiceHandle(schSCManager);
	}

	return dwStatus;
}

BOOLEAN ConnectToDriver(_In_ PWSTR szPortName, _In_ DWORD dwListeningThreads, _Out_ HANDLE* hCompletion, _Out_ HANDLE* hPort)
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
	BOOLEAN bStatus = FALSE;
	HANDLE hMyPort = INVALID_HANDLE_VALUE;
	HANDLE hMyCompletion = INVALID_HANDLE_VALUE;

	*hCompletion = hMyCompletion;
	*hPort = hMyPort;

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
			_tprintf_s(TEXT("\n[-] FilterConnectCommunicationPort, Unable to initialize the connection to the server"));
			bStatus = FALSE;
			__leave;
		}

		// Completion Port to queue message received from the kernel driver
		hMyCompletion = CreateIoCompletionPort(hMyPort, NULL, 0, dwListeningThreads);
		if (hMyCompletion == INVALID_HANDLE_VALUE)
		{
			_tprintf_s(TEXT("\n[-] CreateIoCompletionPort, Unable to create message queue"));
			bStatus = FALSE;
			__leave;
		}

		bStatus = TRUE;
		*hCompletion = hMyCompletion;
		*hPort = hMyPort;

	}
	__finally {

		return bStatus;

	}
}

VOID CreateListenerThread(_In_ HANDLE hPort, _In_ HANDLE hCompletion)
{
	HANDLE* hThreads = nullptr;
	INT i = 0;
	THREAD_PARAMETERS Parameters = { 0 };

	__try {

		hThreads = (HANDLE*)calloc(1, sizeof(HANDLE));			// "1" hardcoded number of listener
		if (hThreads == nullptr)
		{
			_tprintf_s(TEXT("\n[i] calloc,  Unable to create the dynamic handle array\n"));
			__leave;
		}

		Parameters.hCompletion = hCompletion;
		Parameters.hPort = hPort;

		for (i = 0; i < 1; i++)									// "1" hardcoded number of listener
		{
			*(hThreads + i) = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ListenerThread,
				&Parameters,
				0,
				NULL
			);

			if (*(hThreads + i) == NULL)
			{
				_tprintf_s(TEXT("\n[i] CreateThread,  Unable to create listening thread\n"));
				__leave;
			}

		}

		WaitForMultipleObjects(1, hThreads, TRUE, INFINITE);

	}
	__finally {

		for (i = 0; i < 1; i++)
		{
			if (*(hThreads + i) != NULL)
				CloseHandle(*(hThreads + i));
		}

		return;

	}
	
}


VOID ListenerThread(_In_ LPVOID lpParam)
{
	HRESULT hResult = S_OK;
	BOOL bResult = FALSE;
	PTHREAD_PARAMETERS Parameters = NULL;
	DWORD dwOutsize = 0;
	ULONG_PTR uKey = NULL;
	LPOVERLAPPED pOvlp = NULL;
	SYSTEMTIME sysTime = { 0 };
	BOOLEAN bSigned = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DATA_REPLY DataToReply = { 0 };

	__try {

		Parameters = (PTHREAD_PARAMETERS)lpParam;

		if (Parameters->hCompletion == NULL)
		{
			_tprintf_s(TEXT("\n[-] hCompletion,  Empty\n"));
			__leave;
		}

		if (Parameters->hPort == NULL)
		{
			_tprintf_s(TEXT("\n[-] hPort,  Empty\n"));
			__leave;
		}

		Parameters->Msg = (PMESSAGE)HeapAlloc(GetProcessHeap(), 0, sizeof(MESSAGE));
		if (Parameters->Msg == NULL)
		{
			_tprintf_s(TEXT("\n[-] malloc,  Unable to alloc memory for the message\n"));
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
			DisplayError(TEXT("FilterGetMessage"));
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

				DisplayError(TEXT("GetQueuedCompletionStatus"));

				_tprintf_s(TEXT("\n[-] GetQueuedCompletionStatus,  Unable to dequeue message\n"));

				__leave;
			}

			Parameters->Msg = CONTAINING_RECORD(pOvlp, MESSAGE, Ovlp);
			if (Parameters->Msg == NULL)
			{
				_tprintf_s(TEXT("\n[-] CONTAINING_RECORD failed\n"));
				__leave;
			}

			//
			// HANDLE MESSAGE HERE
			//

			// Check signature
			bSigned = VerifyEmbeddedSignature(Parameters->Msg->Data.wFileName);
			if (!bSigned)
			{
				bResult = GetHandleFromFileName(&hFile, Parameters->Msg->Data.wFileName);
				if (!bResult)
					__leave;

				bSigned = VerifyCatalogSignature(Parameters->Msg->Data.wFileName, hFile);
			}

			GetLocalTime(&sysTime);

			_tprintf_s(TEXT("\n%d-%02d-%02d %02d:%02d:%02d %s %s\n"), sysTime.wYear, sysTime.wMonth, sysTime.wDay,
				sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
				Parameters->Msg->Data.wFileName,
				bSigned ? TEXT("SIGNED") : TEXT("NOT SIGNED")
			);

			// Answer
			DataToReply.bIsSigned = bSigned;
			Reply(Parameters->hPort, Parameters->Msg, DataToReply);

			// Push new message in queue
			PumpMessageInQueue(Parameters->hPort, Parameters->Msg);
		}

		if (Parameters->Msg != NULL)
			HeapFree(GetProcessHeap(), 0, Parameters->Msg);

	}
	__finally {

		if (hFile)
			CloseHandle(hFile);

		return;

	}
	

}

BOOLEAN VerifyEmbeddedSignature(_In_ PCWSTR szFile)
/**
 * @brief		Check if the file "szFile" is signed by a trusted authority.\n
 *				[i] WINTRUST_ACTION_GENERIC_VERIFY_V2:
 *				- Check if the certificate used to sign the file chains up to a root certificate located in the trusted root certificate store.
 *
 * @param[in]	"szFile"	 - Name of the .exe to be verify
 *
 * @return		TRUE - File signed\n
 *				FALSE - File not signed
*/
{
	WINTRUST_FILE_INFO winTrustFileInfo = { 0 };
	WINTRUST_DATA winTrustData = { 0 };
	WINTRUST_SIGNATURE_SETTINGS winTrustSettings = { 0 };
	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	BOOLEAN bSigned = FALSE;

	__try {

		//
		// Initialize WinTrust Data
		//

		winTrustData.cbStruct = sizeof(WINTRUST_DATA);
		winTrustData.pPolicyCallbackData = NULL;				// Default code signing EKU
		winTrustData.pSIPClientData = NULL;						// No data to pass to SIP
		winTrustData.dwUIChoice = WTD_UI_NONE;					// Disable WVT UI
		winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;		// No revocation checking
		winTrustData.dwUnionChoice = WTD_CHOICE_FILE;			// Verify embedded signature
		winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;	// Verify action
		winTrustData.hWVTStateData = NULL;						// Verification sets the value
		winTrustData.pwszURLReference = NULL;					// Not used
		winTrustData.dwUIContext = 0;							// UI Disable so no context

		winTrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
		winTrustFileInfo.pcwszFilePath = szFile;
		winTrustFileInfo.hFile = NULL;
		winTrustData.pFile = &winTrustFileInfo;					// Set pFile

		winTrustSettings.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
		winTrustSettings.dwFlags = WSS_VERIFY_SPECIFIC | WSS_GET_SECONDARY_SIG_COUNT;
		winTrustSettings.dwIndex = 0;
		winTrustData.pSignatureSettings = &winTrustSettings;

		//
		// Verify the primary embedded signature of a file.
		//

		// According to MSDN, WinVerifyTrust return a LONG that cannot be cast into a HRESULT!
		if (WinVerifyTrust(NULL, &guidAction, &winTrustData) != 0)
			__leave;

		bSigned = TRUE;

	}
	__finally {

		winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &guidAction, &winTrustData);

		return bSigned;

	}
}

BOOLEAN VerifyCatalogSignature(_In_ PCWSTR szFile, _In_ HANDLE hFile)
/**
 * @brief		Verify the signature of 'szFile' by looking at catalogs files.
 *
 * @param[in]	"szFile"	- Name of the file to check.
 * @param[in]	"hFile"		- Handle of the file to check
 *
 * @return		TRUE - File signed\n
 *				FALSE - File not signed
*/
{
	BOOLEAN bSigned = FALSE;
	HCATADMIN hCatAdmin = NULL;
	HCATINFO hCatInfo = NULL;
	CATALOG_INFO catalogInfo = { 0 };
	DWORD dwHashLength = 0;
	PBYTE pFileHash;

	__try {

		// Acquire catalog context (signature algorithm, ...)
		if (CryptCATAdminAcquireContext2(&hCatAdmin, NULL, NULL, NULL, 0) == NULL) {
			_tprintf_s(TEXT("[-] Error CryptCATAdminAcquireContext2\n"));
			__leave;
		}

		// First call for the size of the HASH
		if (CryptCATAdminCalcHashFromFileHandle2(hCatAdmin, hFile, &dwHashLength, NULL, NULL) == NULL) {
			_tprintf_s(TEXT("[-] Error CryptCATAdminCalcHashFromFileHandle2\n"));
			__leave;
		}

		// Allocate memory
		pFileHash = (PBYTE)calloc(dwHashLength, sizeof(BYTE));
		if (pFileHash == NULL) {
			_tprintf_s(TEXT("[-] Error allocating memory\n"));
			__leave;
		}

		// Second call to get the hash
		if (CryptCATAdminCalcHashFromFileHandle2(hCatAdmin, hFile, &dwHashLength, pFileHash, NULL) == NULL) {
			_tprintf_s(TEXT("[-] (2) Error CryptCATAdminCalcHashFromFileHandle2\n"));
			__leave;
		}


		//
		// Check if the hash is in a catalog
		//

		// get first catalog containing hash
		hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, pFileHash, dwHashLength, 0, &hCatInfo);
		if (hCatInfo == NULL) {
			// No catalog? --> exit
			__leave;
		}

		catalogInfo.cbStruct = sizeof(CATALOG_INFO);

		// Get catalog info (just for information purpose) --> printf
		if (CryptCATCatalogInfoFromContext(hCatInfo, &catalogInfo, 0)) {
			_tprintf_s(TEXT("[-] Error CryptCATCatalogInfoFromContext\n"));
			__leave;
		}

		bSigned = TRUE;

	}
	__finally {

		if (hCatInfo != NULL)
			CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);

		if (hCatAdmin != NULL)
			CryptCATAdminReleaseContext(hCatAdmin, 0);

		return bSigned;

	}
}

_Success_(return != FALSE) BOOLEAN GetHandleFromFileName(_Out_ HANDLE * hFileToCheck, _In_ PCWSTR szFileName)
/**
 * @brief			Get the handle to a file by providing it's name.
 *
 * @param[out]		"hFileToCheck" - Handle file return to the caller
 * @param[in]		"szFileName" - Name of the file which we need a handle to it.
 *
 * @return			TRUE - No error occurs.\n
 *					FALSE - An error occurs.
*/
{
	BOOLEAN bStatus = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	__try {

		// Open file (DOS path) (Read-only)
		hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			_tprintf_s(TEXT("\t[-]Erreur CreateFile : %d\n"), GetLastError());
			__leave;
		}

		// Return valid handle
		*hFileToCheck = hFile;

		bStatus = TRUE;

	}
	__finally {

		// No free(Path)			--> returned to caller (out parameter)
		// No CloseHandle(hFile)	--> returned to caller (out parameter)

		return bStatus;

	}
}

VOID Reply(_In_ HANDLE hPort, _In_ PMESSAGE pMsg, _In_ DATA_REPLY DataToReply)
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

		hResult = FilterReplyMessage(hPort, &Reply->ReplyHeader, REPLY_SIZE);

		if (IS_ERROR(hResult))
		{
			_tprintf_s(TEXT("\t[-] Reply message error: 0x%X\n"), hResult);
			__leave;
		}


	}
	__finally {

		return;

	}
}

VOID PumpMessageInQueue(_In_ HANDLE hPort, _In_ PMESSAGE pMsg)
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
		DisplayError(TEXT("FilterGetMessage"));
		HeapFree(GetProcessHeap(), 0, pMsg);
	}

	return;
}