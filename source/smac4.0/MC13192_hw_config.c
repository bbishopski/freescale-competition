/**************************************************************
*	Includes
**************************************************************/
#include "device_header.h"
#include "pub_def.h"
#include "drivers.h"
#include "MC13192_hw_config.h"
/**************************************************************
*	Globals
**************************************************************/
extern byte rtx_mode;

/**************************************************************
*	Function: 	Initialize the MC13192 register map.
*	Parameters: None
*	Return:		None
**************************************************************/
void MC13192_init(void)
{

 /* MC13192 v2.2 register settings */
 drv_write_spi_1(0x1B,0x8000); /* Disable TC1. */
 drv_write_spi_1(0x1D,0x8000); /* Disable TC2. */
 drv_write_spi_1(0x1F,0x8000); /* Disable TC3. */
 drv_write_spi_1(0x21,0x8000); /* Disable TC4. */
 drv_write_spi_1(0x07,0x0E00); /* Enable CLKo in Doze */
 drv_write_spi_1(0x0C,0x0300); /* IRQ pull-up disable. */
 drv_read_spi_1(0x25); /* Sets the reset indicator bit */
 drv_write_spi_1(0x04,0xA08D); /* LR ADDED New cal value */
 drv_write_spi_1(0x08,0xFFF7); /* Preferred injection */
 drv_write_spi_1(0x05,0x8351); /* Acoma, TC1, Doze, ATTN masks, LO1, CRC */
 drv_write_spi_1(0x06,0x4720); /* CCA, TX, RX, energy detect */
 
 /* Read the status register to clear any undesired IRQs. */
 drv_read_spi_1(0x24); /* Clear the status register, if set */
 rtx_mode = IDLE_MODE; /* Update global to reflect MC13192 status */

}
