#pragma once
#pragma comment (lib, "ntdll.lib")	// for NtQueryObject
#pragma comment (lib, "wintrust.lib")

#include "Main.hpp"
#include <winternl.h>				// for NtQueryObject
#include <TlHelp32.h>

// Lib to verify signature
#include <wincrypt.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <mscat.h>


#define TRIPLE_MAX_PATH				780
#define REPLY_SIZE					(sizeof(FILTER_REPLY_HEADER) + sizeof(BOOLEAN))

// Print
typedef enum class _MESSAGE_TYPE {
	TYPE_ERROR,		// print in red
	TYPE_SUCCESS	// print in green
} MESSAGE_TYPE;

typedef struct _DATA_TRANSMIT
{
	//
	// Name of the file to be checked
	//

	WCHAR wFileName[TRIPLE_MAX_PATH];

} DATA_TRANSMIT, * PDATA_TRANSMIT;

// Data
typedef struct _MESSAGE {
	FILTER_MESSAGE_HEADER messageHeader;	// Required structure header
	DATA_TRANSMIT Data;						// Custom data structure
	OVERLAPPED Ovlp;						// Required structure when dealing with asynchronous message handling
} MESSAGE, * PMESSAGE;

// Thread
typedef struct _THREAD_PARAMETERS {
	HANDLE hPort;
	HANDLE hCompletion;
	PMESSAGE Msg;
} THREAD_PARAMETERS, * PTHREAD_PARAMETERS;

// Reply
typedef struct _DATA_REPLY
{
	//
	// Is the exe provided is signed ?
	//

	BOOLEAN bIsSigned;

} DATA_REPLY, * PDATA_REPLY;

typedef struct _REPLY_MESSAGE
{
	FILTER_REPLY_HEADER ReplyHeader;	// Required structure header
	DATA_REPLY DataReply;				// Is the exe provided is signed ?
} REPLY_MESSAGE, * PREPLY_MESSAGE;



// Display Error
VOID PrintMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...);
VOID DisplayMessage(_In_ MESSAGE_TYPE msgType, _In_ _Printf_format_string_ LPCTSTR lpszFormat, ...);
VOID DisplayError(_In_ LPCTSTR lpszFunction);

// Service Handle & State
SC_HANDLE GetSCMHandle();
DWORD GetServiceState(_In_ LPCTSTR lpszServiceName);

// Connection
BOOLEAN ConnectToDriver(_In_ PWSTR szPortName, _In_ DWORD dwListeningThreads, _Out_ HANDLE* hCompletion, _Out_ HANDLE* hPort);
VOID CreateListenerThread(_In_ HANDLE hPort, _In_ HANDLE hCompletion);
VOID ListenerThread(_In_ LPVOID lpParam);

// Verify Signature
BOOLEAN VerifyEmbeddedSignature(_In_ PCWSTR szFile);
BOOLEAN VerifyCatalogSignature(_In_ PCWSTR szFile, _In_ HANDLE hFile);
_Success_(return != FALSE) BOOLEAN GetHandleFromFileName(_Out_ HANDLE * hFileToCheck, _In_ PCWSTR szFileName);

//Reply
VOID Reply(_In_ HANDLE hPort, _In_ PMESSAGE pMsg, _In_ DATA_REPLY DataToReply);

// Push new message
VOID PumpMessageInQueue(_In_ HANDLE hPort, _In_ PMESSAGE pMsg);