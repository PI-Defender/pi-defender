 // MESSAGE DEFINITIONS
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: PIDEFENDER_CATEGORY_GENERAL
//
// MessageText:
//
// Tasks
//
#define PIDEFENDER_CATEGORY_GENERAL      ((DWORD)0x00000001L)

//
// MessageId: TASK_OK
//
// MessageText:
//
// Task %1 completed succesfully (%2).
//
#define TASK_OK                          ((DWORD)0x00000064L)

//
// MessageId: TASK_ERROR
//
// MessageText:
//
// Task %1 failed to complete (%2).
//
#define TASK_ERROR                       ((DWORD)0x000000C8L)

//
// MessageId: TASK_INFO
//
// MessageText:
//
// %1 Information: %2.
//
#define TASK_INFO                        ((DWORD)0x0000012CL)

