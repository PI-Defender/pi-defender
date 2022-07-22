/**
 * @file       PI-Defender_UM.mc
 * @brief      Define log messages string to display
 * @author     NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version    1.0
 * @date       02/06/2022
 * @copyright  ©Naval Group SA.
 *             This document in its content and form is the property of Naval Group SA and/or third parties.
 *             This project is released under the LGPLv3 license.
*/

LanguageNames=(English=0x409:MSG00409)

; // MESSAGE DEFINITIONS

MessageIdTypedef=DWORD

MessageId=1
SymbolicName=PIDEFENDER_CATEGORY_GENERAL
Language=English
Tasks
.

MessageId=100
SymbolicName=TASK_OK
Language=English
Task %1 completed succesfully (%2).
.

MessageId=200
SymbolicName=TASK_ERROR
Language=English
Task %1 failed to complete (%2).
.

MessageId=300
SymbolicName=TASK_INFO
Language=English
%1 Information: %2.
.
