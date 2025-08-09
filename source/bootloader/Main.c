/************************************************************************************
* Main file for PTC (Test). Includes Main routine.
*
* Author(s): Jakob Koed
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
#include "MacPhy.h"
#if !defined( INCLUDE_802_15_4 )
#include "PhyMacMsg.h"
#include "../Mac/Support/MacConst/MacConst.h"
#include "../Mac/Support/FunctionLib/FunctionLib.h"
#include "../Ptc/TestParser.h"
#include "../Ptc/TestParserNwkMac.h"
#endif //!INCLUDE_802_15_4
#include "NV_Data.h"

#ifdef BOOTLOADER_ENABLED
	#include "FreeLoader_inf.h"
#endif BOOTLOADER_ENABLED

extern void MPIB_Init(void);
#if gAspCapability_d
extern void PTC_APPASP_AppMainLoop(void);
extern void PTC_APPASP_AppInit(void);
#else
#define PTC_APPASP_AppMainLoop()
#define PTC_APPASP_AppInit()
#endif // gAspCapability_d


#ifdef PLATFORM_WINDOWS
/************************************************************************************
*************************************************************************************
* Version for PC (Host) based debugging via socket interface
*************************************************************************************
************************************************************************************/

#include <windows.h>

/* OLD code

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	PTC_InitSystem();

	// MainLoop
	for(;;) {
		if ( !PTC_TestInterfaceCheckRx() ) break;
	}
}

*/

NV_RAM_Struct_t gWinSimNvRam;

DWORD WINAPI PTC_Main(void *dummy)
{
    // Setup pointer to NV RAM section 0
  gWinSimNvRam.MAC_Address[0] = 0x55;
  gWinSimNvRam.MAC_Address[1] = 0x55;
  gWinSimNvRam.MAC_Address[2] = 0x55;
  gWinSimNvRam.MAC_Address[3] = 0x55;
  gWinSimNvRam.MAC_Address[4] = 0x55;
  gWinSimNvRam.MAC_Address[5] = 0x55;
  gWinSimNvRam.MAC_Address[6] = 0x55;
  gWinSimNvRam.MAC_Address[7] = 0x55;
	NV_RAM_ptr = &gWinSimNvRam;
  FLib_MemCpy(aExtendedAddress, (NV_RAM_Struct_t *)NV_RAM_ptr->MAC_Address, 8);

	MPIB_Init();
  InitializePhy();
	InitializeMac();

	PTC_InitSystem();
	PTC_NWKMAC_NwkInit(); // Make sure queue is initialized
  PTC_APPASP_AppInit(); // Make sure queue is initialized
	// MainLoop
	for(;;) {
		PTC_NWKMAC_NwkMainLoop();	// Do NWK-similar polling
    PTC_APPASP_AppMainLoop(); // Do APP-similar polling
    Sleep(10);
		if ( !PTC_TestInterfaceCheckRx() ) break;
	}
	return(0);
}

/************************************************************************************
*************************************************************************************
* Version for embedded debugging via UART or BDM interface
*************************************************************************************
************************************************************************************/

#else // not WIN32

#include "Gb60_io.h" /* include peripheral declarations */
#include "hwdrv_hcs08.h"

// NOTE!!! This include line is only added to get the symbols in the build. The linker will remove these symbols if not referenced.
// Hope to find a better way 21.10.03 MVC 
#include "..\Sys\Crt0.h"

#if !defined( INCLUDE_802_15_4 )
#include "../Ptc/TestUart.h"
#endif // ! INCLUDE_802_15_4

/**********************************************************************/

// NOTE!!! This code is only added to get the symbols in the build. The linker will remove these symbols if not referenced.
// Hope to find a better way 21.10.03 MVC 

typedef void(*ISR_func_t)(void);
extern const ISR_func_t ISR_vectors[]; 
extern const ISR_func_t Reset_vector[];	 // The system reset vector

#ifdef BOOTLOADER_ENABLED
  extern const uint8_t interface_status;
  extern const uint8_t boot_loader_flag;
#endif BOOTLOADER_ENABLED

#if defined( INCLUDE_802_15_4 )
  // Dont use FunctionLib.h when releasing MAC/PHY as libs
extern void FLib_MemCpy(void*, void*, uint8_t);
  // Dont use MacConst.h when releasing MAC/PHY as libs
extern uint8_t aExtendedAddress[];
#endif INCLUDE_802_15_4

/**********************************************************************/
extern void SeqTestMain(void);

/**********************************************************************/

/**********************************************************************/
/**********************************************************************/
// If the code is build as a stand-alone application, this function is
// the main entry point. If the code is build as libs, this function is
// the main initialization procedure which will be called from the 
// startup code of the project in which the libs are included.
// No PTC or other test facilities are included if building as libs.
#if defined( INCLUDE_802_15_4 )

void Init_802_15_4(void) {

#else

void main(void) {

#endif INCLUDE_802_15_4

/**********************************************************************/

// NOTE!!! This code is only added to get the symbols in the build. The linker will remove these symbols if not referenced.
// Hope to find a better way 21.10.03 MVC 

#ifndef BOOTLOADER_ENABLED
  if(none_volatile_struct.nv_backkey[0] == 0)
    DisableInterrupts;
  if(Reset_vector[0] == (ISR_func_t)0x0000)
    DisableInterrupts;
#endif BOOTLOADER_ENABLED
  if(ISR_vectors[0] == (ISR_func_t)0x0000)
    DisableInterrupts;
  if((Freescale_Copyright[0] == 0x00) || (Firmware_Database_Label[0] == 0x00) ||
     (MAC_Version[0] == 0x00) || (PHY_Version[0] == 0x00) ||	(NV_RAM_ptr->Freescale_Copyright[0] == 0x00) ||
     (NV_RAM0.MAC_Address[0] == 0x00))
  {
    DisableInterrupts;
    NV_Data_Init();
  }
  // Insert correct IEEE address from NV RAM
  FLib_MemCpy(aExtendedAddress, (NV_RAM_Struct_t *)NV_RAM_ptr->MAC_Address, 8);

#ifdef BOOTLOADER_ENABLED
  if(boot_loader_control == ((uint8_t)0x00))
    DisableInterrupts;
  if(boot_loader_flag == ((uint8_t)0x00))
    DisableInterrupts;
#endif BOOTLOADER_ENABLED

/**********************************************************************/
 
#ifdef BOOTLOADER_ENABLED
  BootLoader_Interface_Init();
#endif BOOTLOADER_ENABLED

  HwSetup();
  MPIB_Init();
  InitializePhy();
  InitializeMac();

#if defined( INCLUDE_802_15_4 )
  
} // End of Init_802_15_4()
#else 

  PTC_InitSystem();
  PTC_NWKMAC_NwkInit(); // Make sure queue is initialized
  PTC_APPASP_AppInit(); // Make sure queue is initialized

  EnableInterrupts; /* enable interrupts */
  /* include your code here */
  SeqTestMain();

#pragma DISABLE_WARNING(UnusedResultOfFunction_c)
  
  for(;;) {
       
#ifdef I_AM_A_SNIFFER
    poll_data_from_PHY();
#endif I_AM_A_SNIFFER

#ifdef FTDI_USB
    poll_ftdi_rx();
    poll_ftdi_tx();
#endif 
	        
    PTC_NWKMAC_NwkMainLoop();	// Do NWK-similar polling
    PTC_APPASP_AppMainLoop(); // Do APP-similar polling
    PTC_TestInterfaceCheckRx(); // return value ignored - always stays in mainloop

  } /* loop forever */

#pragma RESTORE_WARNING(UnusedResultOfFunction_c)
}

#endif INCLUDE_802_15_4 // End of main()

/***********************************************************************************/

#endif PLATFORM_WINDOWS
