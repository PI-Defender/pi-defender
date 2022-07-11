/**
 * @file		SharedDefs.hpp
 * @brief		Header file which contains the structures, type definitions, constants, global variablesand function prototypes that are shared between kerneland user mode. 
 * @author		Berenger BRAULT, Nicolas JALLET
 * @version		1.0
 * @date		02/06/2022
 * @copyright	LGPLv3
*/

#ifndef SHARED_DEFS_HPP
#define SHARED_DEFS_HPP
#pragma once


#define TRIPLE_MAX_PATH		780
#define DRIVER_ALTITUDE		TEXT("325556")


typedef struct _DATA_TRANSMIT
{
	//
	// Name of the file to be checked
	//

	WCHAR wFileName[TRIPLE_MAX_PATH];

} DATA_TRANSMIT, * PDATA_TRANSMIT;


typedef struct _DATA_REPLY 
{
	//
	// Is the exe provided is signed ?
	//

	BOOLEAN bIsSigned;

} DATA_REPLY, *PDATA_REPLY;


#endif // !SHARED_DEFS_HPP