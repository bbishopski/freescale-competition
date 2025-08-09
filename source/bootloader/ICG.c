/************************************************************************************
* This module contains code for the ICG (Internal Clock Generator) module.
* 
* Note! Support for selecting external crystal and external clock source from Abel 2.0
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
* Last Inspected:
* Last Tested:
************************************************************************************/

#include "DigiType.h"
#include "ICG.h"
#include "FreeLoader_inf.h"

#if !defined BOOTLOADER_ENABLED || !defined FOR_BOOTLOADER_ONLY
  #pragma PLACE_DATA_SEG(SLEEP_VARIABLE)
  extern uint8_t gSeqPowerSaveMode;
  #pragma RESTORE_DATA_SEG
#else
	#include "Application_interface.h"
#endif FOR_BOOTLOADER_ONLY 

// **************************************************************************

#if !defined BOOTLOADER_ENABLED || defined FOR_BOOTLOADER_ONLY

// **************************************************************************

#ifdef PLATFORM_GB60
#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration
#pragma DATA_SEG BOOTLOADER_VARIABLES
#endif PLATFORM_GB60

	// Global variables
	volatile bool_t init_mode = TRUE;

#ifdef PLATFORM_GB60
#pragma DATA_SEG DEFAULT
#endif PLATFORM_GB60

// **************************************************************************

void ICG_Setup(void)
{
uint8_t loop_counter;

	SETUP_LED_PORT
	
	SET_LED1 // Show entry in function

	SET_LED2 // Cleared first time in loop
	CLEAR_LED3
	CLEAR_LED4

	if(gSeqPowerSaveMode == 0)
	{
		// Not in power save mode

		// Loop until clock is locked
		for(;;)
		{
			TOGGLE_LED2

			if((ICGS1 & ICG_IRQ_PENDING) == ICG_IRQ_PENDING)
				ICGS1 |= ICG_IRQ_PENDING; // Clear FLL lost lock interrupt

			if(init_mode == FALSE)
			{
				TOGGLE_LED3
				
				// Is filter value drifting?
				if(ICGFLTH > ICGFLTH_VALUE_8MHZ)
				{
					TOGGLE_LED4
					
					SCM_8MHZ_DEFAULT_ICG_FILTER
					WAIT_FOR_STABLE_CLOCK
				}

				SELECT_SCM_MODE // DO NOT CALL THIS CODE UNDER INIT

				WAIT_FOR_STABLE_CLOCK
			}
			else
				init_mode = FALSE; // Only one time under init

			SETUP_ABEL_CLOCK

			WAIT_FOR_STABLE_CLOCK

			SETUP_ICG_MODULE

			WAIT_FOR_STABLE_CLOCK

			loop_counter = LOOP_COUNTER_VALUE;

			// Wait for clock to lock
			while(((ICGS1 & ICG_FLL_LOCKED) != ICG_FLL_LOCKED) && loop_counter-- > 0);

			// Check exit condition
			if((ICGS1 & ICG_FLL_LOCKED) == ICG_FLL_LOCKED)
				break; // Clock is locked - get out
		}
	}
	else
	{
		// In power save mode - set SCM clock.
		SCM_2MHZ_ICG_FILTER
	}
	
	MEASURING_BUS_CLOCK
	
	CLEAR_LED1 // Show exit of function
}

// **************************************************************************

#endif !defined BOOTLOADER_ENABLED || defined FOR_BOOTLOADER_ONLY

// **************************************************************************

