/********************************************************
application calls for Bootloader.


Author: LTR
File: bootloader user api.c
********************************************************/

#include "bootloader user api.h"
#include "NV_data.h"

  /**********************************************************************/

// NOTE!!! This code is only added to get the symbols in the build. The linker will remove these symbols if not referenced.
// Hope to find a better way 21.10.03 MVC 

void boot_init() {
  
  #ifdef BOOTLOADER_ENABLED

    if((Freescale_Copyright[0] == 0x00) || (Firmware_Database_Label[0] == 0x00) ||
       (SMAC_Version[0] == 0x00) || (SPHY_Version[0] == 0x00) ||	(NV_RAM_ptr->Freescale_Copyright[0] == 0x00) ||
       (NV_RAM0.MAC_Address[0] == 0x00) || (NV_RAM1.MAC_Address[0] == 0x00)) 
    {
      DisableInterrupts;
      NV_Data_Init();
    }
    if(boot_loader_control == ((uint8_t)0x00))
      DisableInterrupts;
    if(boot_loader_flag == ((uint8_t)0x00))
      DisableInterrupts;

  #endif BOOTLOADER_ENABLED

  /**********************************************************************/
   
  #ifdef BOOTLOADER_ENABLED
    BootLoader_Interface_Init();
  #endif BOOTLOADER_ENABLED

}

void boot_call() {
  
 #ifdef BOOTLOADER_ENABLED
		// Push all 4 buttons from reset to active BootLoader
	// Next time the board is reset it will start up in BootLoader mode
	if(PB0 == PRESSED && PB1 == PRESSED && 
		PB2 == PRESSED && PB3 == PRESSED)
		{
		
		  LED1 = 0;
		  LED2 = 0;
		  LED3 = 0;
		  LED4 = 0;
      Enable_Download_Firmware(ALL_BIT_ENABLED, DO_UPDATE_FIRMWARE);
            
      // Do a soft reset (execute illegal instruction
      Hard_Reset();
            
      // Wait for manual reset
      // for(;;);
		}

  #endif BOOTLOADER_ENABLED
  
}