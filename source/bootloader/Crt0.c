/************************************************************************************
* This module contains some basic init code.
*
* Note! Basic code is adapted (start08.c) from some code developed by Metrowerks.
* Not used code has been deleted. Code has been cleaned up and rearranged.
* 
* Note! The init code in this file can be used with a D18/Application with or without the
*       BootLoader. #define FOR_BOOTLOADER_ONLY MUST be specificed for use with BootLoader
*
* Author(s):  Michael V. Christensen
*
* (c) Copyright 2004, Freescale, Inc.  All rights reserved.
*
* Freescale Confidential Proprietary
* Digianswer Confidential
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 29-03-01
* Last Tested:
************************************************************************************/


/**********************************************************************/
// Includes

#include "DigiType.h"
#include "Crt0.h"
#include "Gb60_io.h"
#include "NV_Data.h"

#ifdef FLASH_LIB_ENABLED
    #include "Update_NVM.h"
#endif FLASH_LIB_ENABLED

/**********************************************************************/

#pragma MESSAGE DISABLE C20001 // Warning C20001: Different value of stackpointer depending on control-flow
#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration

/**********************************************************************/

// Define the STARTUP data segement. Start code must be allocated to this segment.
#pragma DATA_SEG FAR _STARTUP

// Define a _startup_Data structure. The linker searches for the name "_startup_Data". The linker
// initialize the structure and the structure is allocated in ROM

// Store for init code stored in flash
_startupData_t _startupData;

// **************************************************************************

#if !defined BOOTLOADER_ENABLED || defined FOR_BOOTLOADER_ONLY
#pragma CONST_SEG NV_REGISTERS // Section to store

// HCS08 NV register struct with values.
const volatile NV_REG_t none_volatile_struct =
{   // DO NOT CHANGE
	NV_BACKKEY_VALUE,
	NV_RESERVED_VALUE,
	NV_PROT_VALUE,
	NV_ICG_TRIM_VALUE,
	NV_OPT_VALUE
};

#pragma CONST_SEG DEFAULT

/**********************************************************************/

#ifdef FOR_BOOTLOADER_ONLY

	// Startup structure in RAM
	#pragma DATA_SEG STARTUP_STRUCT_DATA
		_startupData_t _startupData_var;
	#pragma DATA_SEG default

#endif FOR_BOOTLOADER_ONLY

/**********************************************************************/
// Local prototypes

static void Copy_Down(void);
static void Finish(void);

/**********************************************************************/
// External prototypes

extern void _COPY_L(void); // Function from RTSHC08.C.o (ansiis.lib)
extern char __SEG_END_SSTACK[];

extern void main_interface(void); // function in main.c

/****************************************************************************************************/
/* Function name:		loadByte()   									                            */
/* Description:                                                                                     */
/* Input parameter:                                                                                 */
/* Output parameter:                                                                                */
/****************************************************************************************************/

static void near loadByte(void)
{
asm
{
             PSHH
             PSHX

             LDHX    5,SP
             LDA     0,X
             AIX     #1
             STHX    5,SP
             PULX
             PULH
             RTS
} // ASM end
}

/****************************************************************************************************/
/* Function name:	Init() 						                                                    */
/* Description:         1) zero out RAM-areas where data is allocated                               */
/*                      2) init run-time data                                                       */
/*                      3) copy initialization data from ROM to RAM                                 */
/* Input parameter:                                                                                 */
/* Output parameter:                                                                                */
/****************************************************************************************************/

void Init(void)
{
int i;
int *far p;

asm
{
ZeroOut:     ;
             LDA    STARTUP_DATA.nofZeroOuts:1 ; nofZeroOuts
             INCA
             STA    i:1                        ; i is counter for number of zero outs
             LDA    STARTUP_DATA.nofZeroOuts:0 ; nofZeroOuts
             INCA
             STA    i:0
             LDHX   STARTUP_DATA.pZeroOut      ; *pZeroOut
             BRA    Zero_5
Zero_3:    ;
            ; CLR    i:1 is already 0
Zero_4:    ;
             ; { HX == _pZeroOut }
             PSHX
             PSHH
             ; { nof bytes in (int)2,X }
             ; { address in (int)0,X   }
             LDA    0,X
             PSHA
             LDA    2,X
             INCA
             STA    p                  ; p:0 is used for high byte of byte counter
             LDA    3,X
             LDX    1,X
             PULH
             INCA
             BRA    Zero_0
Zero_1:    ;
           ;  CLRA   A is already 0, so we do not have to clear it
Zero_2:    ;
             CLR    0,X
             AIX    #1
Zero_0:    ;
             DBNZA  Zero_2
Zero_6:
             DBNZ   p, Zero_1
             PULH
             PULX                           ; restore *pZeroOut
             AIX    #4                      ; advance *pZeroOut
Zero_5:    ;
             DBNZ   i:1, Zero_4
             DBNZ   i:0, Zero_3
             ;
CopyDown:    JSR    Copy_Down;

} // ASM end
}

/****************************************************************************************************/
/* Function name:	Copy_Down() 						                                            */
/* Description:     _startupData.toCopyDownBeg  --->                                                */
/*                  {nof(16) dstAddr(16) {bytes(8)}^nof} Zero(16)                                   */
/* Input parameter:	None                                                                            */
/* Output parameter:    None                                                                        */
/* Error handling:      None                                                                        */
/****************************************************************************************************/

static void Copy_Down(void)
{
int i;

asm 
{
             LDHX   STARTUP_DATA.toCopyDownBeg:toCopyDownBegOffs
             PSHX  
             PSHH  
Loop0:             
             JSR    loadByte  ; load high byte counter
             TAX              ; save for compare
             INCA  
             STA    i
             JSR    loadByte  ; load low byte counter
             INCA  
             STA    i:1
             DECA
             BNE    notfinished
             CBEQX  #0, finished
notfinished:

             JSR    loadByte  ; load high byte ptr
             PSHA  
             PULH  
             JSR    loadByte  ; load low byte ptr
             TAX              ; HX is now destination pointer
             BRA    Loop1
Loop3:             
Loop2:             
             JSR    loadByte  ; load data byte
             STA    0,X
             AIX    #1
Loop1:
             DBNZ   i:1, Loop2
             DBNZ   i:0, Loop3
             BRA    Loop0

finished:
             AIS #2

		     JSR    Finish;

} // ASM end
}

/****************************************************************************************************/
/* Function name:	Finish() 			                                                            */
/* Description:     optimized asm version. Some bytes (ca 3) larger than C version                  */
/*                  (when considering the runtime routine too), but about 4 times                   */
/*                  faster.                                                                         */
/* Input parameter:	None                                                                            */
/* Output parameter:    None                                                                        */
/* Error handling:      None                                                                        */
/****************************************************************************************************/

static void Finish(void)
{
asm
{
             LDHX   STARTUP_DATA.toCopyDownBeg:toCopyDownBegOffs
next:
             LDA   0,X    ; list is terminated by 2 zero bytes
             ORA   1,X
             BEQ copydone
             PSHX         ; store current position
             PSHH
             LDA   3,X    ; psh dest low
             PSHA
             LDA   2,X    ; psh dest high
             PSHA
             LDA   1,X    ; psh cnt low
             PSHA
             LDA   0,X    ; psh cnt high
             PSHA
             AIX   #4
             JSR  _COPY_L ; copy one block
             PULH
             PULX       
             TXA
             ADD   1,X    ; add low 
             PSHA 
             PSHH
             PULA
             ADC   0,X    ; add high
             PSHA
             PULH
             PULX
             AIX   #4
             BRA next
copydone:

} // ASM end
}

/****************************************************************************************************/
/* Function name:	_Startup() 				                                                        */
/* Description:     Entry point for the SW. DO NOT CHANGE FUNCTION NAME.                            */
/*                  It will initialize the stack and the ZI and RW memory.                          */
/* Input parameter:	None                                                                            */
/* Output parameter:None                                                                            */
/* Error handling:  None                                                                            */
/****************************************************************************************************/

void _Startup (void) // DO NOT CHANGE FUNCTION NAME
{
	// Disable interrupts - HCS08 should have interrupts disable from reset.
	DisableInterrupts;

	// Setup SIM options	
	SIMOPT = SYSTEMS_OPTION_REG_VALUE;

	// Setup stack
	if (!(_startupData.flags & STARTUP_FLAGS_NOT_INIT_SP))
	{
		// Initialize the stack pointer */
		__asm LDHX @__SEG_END_SSTACK;
		__asm TXS;
	}                 

	Find_NV_RAM();

	// Change default SCM (mode 2) bus frequency from ~4 MHz to ~8 MHz
	// SCM is only temporary until Abel is ready. No precise frequency is required.
	// Note! FLL is bypassed in SCM mode.

	// Store ICG filter values to change bus frequency for self clock mode
	// Filter values are copied from NV RAM - Search for valid NV RAM
	
	// Check NV RAM section 0
	if(NV_RAM_ptr != NULL)
	{
		ICGFLTL = NV_RAM_ptr->NV_ICGFLTL;
		ICGFLTH = NV_RAM_ptr->NV_ICGFLTU;
	}
	else
	{ 
		// Both NV RAM sections are empty -> no application available?
		// Use default values from code
		ICGFLTL = ICG_FILTER_LSB; // LSB value must be written first
		ICGFLTH = ICG_FILTER_MSB;
	}

	// A few nops for clock to settle.
	__asm nop
	__asm nop
	__asm nop
	__asm nop

	CALL_MAIN_INTERFACE
}

// **************************************************************************


#if !defined FLASH_LIB_ENABLED && !defined BOOTLOADER_ENABLED 
void Find_NV_RAM(void)
{
	// Setup pointer to NV RAM section 0
	NV_RAM_ptr = (NV_RAM_Struct_t*)NV_RAM0_ADDRESS;

	// Check NV RAM in section 0
	if(NV_RAM_ptr->NV_RAM_Version != (uint16_t)0xFFFF)
		// NV RAM Found in section 0
		return;

	// NV RAM section 0 was empty - set pointer to NV RAM section 1
	NV_RAM_ptr = (NV_RAM_Struct_t*)NV_RAM1_ADDRESS;

	// Check NV RAM in section 1
	if(NV_RAM_ptr->NV_RAM_Version != (uint16_t)0xFFFF)
		// NV RAM Found in section 1
		return;

	// NV RAM NOT FOUND
	NV_RAM_ptr = NULL;
}

#endif !defined FLASH_LIB_ENABLED && !defined BOOTLOADER_ENABLED

// **************************************************************************

#ifdef FOR_BOOTLOADER_ONLY

void Copy_startup_structure(_startupData_t * copy_structure)
{
		// Copy structure to init structure
		_startupData_var.flags = copy_structure->flags;
		_startupData_var.main = copy_structure->main;  					// Top level procedure of user program
		_startupData_var.stackOffset = copy_structure->stackOffset; 	// Initial value of the stack pointer
		_startupData_var.nofZeroOuts = copy_structure->nofZeroOuts;		// Number of zero out ranges
		_startupData_var.pZeroOut = copy_structure->pZeroOut;			// Vector of ranges with nofZeroOuts elements
		_startupData_var.toCopyDownBeg = copy_structure->toCopyDownBeg;	// ROM-address where copydown-data begins

#if INCLUDE_ROM_LIBRARIES
		_startupData_var.nofLibInits = copy_structure->nofLibInits; 	// number of library startup descriptors
		_startupData_var.libInits = copy_structure->libInits;         	// Vector of pointers to library startup descriptors
#endif INCLUDE_ROM_LIBRARIES

}

#endif FOR_BOOTLOADER_ONLY

// **************************************************************************

#endif !defined BOOTLOADER_ENABLED || defined FOR_BOOTLOADER_ONLY

// **************************************************************************

#if defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY

// Just a dummy function - but must exist.
void _Startup (void) // DO NOT CHANGE FUNCTION NAME
{
	
}

#endif defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY

// **************************************************************************

