/****************************************************************************
* sard_board.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	sard_board.c
* 
* Contains initialization procedure for the 13192 sard board.
*
****************************************************************************/
#include "sard_board.h"
#include <MC9S08GT60.h> /* include peripheral declarations */

void sard_board_init(void) 
{
  
  // Init LED's
	LED1 = LED_OFF; /* Default is off */
	LED2 = LED_OFF;
	LED3 = LED_OFF;
	LED4 = LED_OFF;

	// Set the DDIR register as Outputs for the LED pins
	LED1DIR = 1; 
	LED2DIR = 1;
	LED3DIR = 1;
	LED4DIR = 1;
	
  /* Pushbutton directions and pull-ups */
  // PU=1 = internal pullup enabled	
  // DIR=0 = INPUT
  
  // This shouldn't matter since we're setting KBI values below
  // which overrides PTA (port a) settings
  PB0PU = 1; 
  PB0DIR = 0;
  PB1PU = 1;
  PB1DIR = 0;
  PB2PU = 1;
  PB2DIR = 0;
  PB3PU = 1;
  PB3DIR = 0;

  // Enable pushbuttons, setting to 1 makes the PTAn an input
  // for keyboard rather than general purpose I/O pin not
  // associated with KBI
  //
  // For the SARD, S101 is KB2 (PTA2) - S102 is KB3 (PTA3)
  // 
  KBI1PE_KBI1PE2 = 1;  // s101 interrupt enable
  KBI1PE_KBI1PE3 = 1;  // s102 interrupt enable
  
  // edge only operation
  KBI1SC_KBIMOD = 0;
  
  // enable keyboard interrupts
  KBI1SC_KBI1E =1;

}

