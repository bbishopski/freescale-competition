/************************************************************************************
* This module contains the IRQ vector table
* 
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

#ifndef WIN32

#include "DigiType.h"
#include "hwdrv_hcs08.h"
#include "FreeLoader_inf.h"

// **************************************************************************

typedef void(*ISR_func_t)(void);

// **************************************************************************

extern __interrupt void Default_Dummy_ISR(void);
extern __interrupt void FLL_Lost_Lock_ISR(void);
extern void _Startup (void);

// **************************************************************************

#pragma CONST_SEG IRQ_VECTOR_SECTION

const ISR_func_t ISR_vectors[] = 
{
        // Interrupt table
        Default_Dummy_ISR,       // vector 31 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 30 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 29 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 28 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 27 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 26 	- NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 25  	Real time interrupt
        Default_Dummy_ISR,       // vector 24	IIC control
        Default_Dummy_ISR,       // vector 23	AD conversion complete
        Default_Dummy_ISR,       // vector 22	Keyboard pins
        Default_Dummy_ISR,       // vector 21	SCI2 transmit
        Default_Dummy_ISR,       // vector 20	SCI2 receive
        Default_Dummy_ISR,       // vector 19	SCI2 error
        Default_Dummy_ISR,       // vector 18	SCI1 transmit
        Default_Dummy_ISR,       // vector 17	SCI1 receive
        Default_Dummy_ISR,       // vector 16	SCI1 error
        Default_Dummy_ISR,       // vector 15	SPI
        Default_Dummy_ISR,       // vector 14	TPM2 overflow
        Default_Dummy_ISR,       // vector 13	TPM2 channel 4
        Default_Dummy_ISR,       // vector 12	TPM2 channel 3
        Default_Dummy_ISR,       // vector 11	TPM2 channel 2
        Default_Dummy_ISR,       // vector 10	TPM2 channel 1
        Default_Dummy_ISR,       // vector 9	TPM2 channel 0
        Default_Dummy_ISR,       // vector 8	TPM1 overflow
        Default_Dummy_ISR,       // vector 7	TPM1 channel 2
        Default_Dummy_ISR,       // vector 6	TPM1 channel 1
        Default_Dummy_ISR,       // vector 5	TPM1 channel 0
        FLL_Lost_Lock_ISR,       // vector 4	ICG (FLL lock of clock)
        Default_Dummy_ISR,       // vector 3	Low voltage detect
        AbelInterrupt,           // vector 2	External IRQ (Abel interrupt)
        Default_Dummy_ISR,       // vector 1	Software interrupt (SWI)
        						 // vector 0	Reset (Watchdog timer, Low voltage detect, external pin, illegal opcode)
};

// **************************************************************************

#ifndef BOOTLOADER_ENABLED

#pragma CONST_SEG RESET_VECTOR_SECTION

const ISR_func_t Reset_vector[] = {_Startup};	 // Vector 0	Reset vector 

#pragma CONST_SEG DEFAULT

#endif BOOTLOADER_ENABLED

#endif WIN32

// **************************************************************************

