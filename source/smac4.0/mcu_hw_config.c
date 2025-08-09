/**************************************************************
*	Includes
**************************************************************/
//#include "device_header.h" /* include peripheral declarations */
#include "pub_def.h"
#include "MC13192_regs.h"
#include "MC13192_hw_config.h"
#include "mcu_hw_config.h"
#include "drivers.h"

/**************************************************************
*	Globals
**************************************************************/
extern byte rtx_mode;

/**************************************************************
*	Function: 	Initialize the MCU-to-MC13192 GPIO direction and data.
*	Parameters: None
*	Return:		None
**************************************************************/
void init_gpio()
{
 MC13192_RESET_PULLUP = 0;
 MC13192_CE = 1;					
 MC13192_ATTN = 1;
 MC13192_RTXEN = 0;
 MC13192_RESET = 0;				/* Do not initially reset MC13192 */
 MC13192_CE_PORT = 1;
 MC13192_ATTN_PORT = 1;
 MC13192_RTXEN_PORT = 1;
 MC13192_RESET_PORT = 1;
 MC13192_CE = 1;					
 MC13192_ATTN = 1;
 MC13192_RTXEN = 0;
 MC13192_RESET = 0;				/* Do not initially reset MC13192 */
 
 #if defined (ANTENNA_SWITCH)
 	
 		MC13192_ANT_CTRL2_PORT = 1;		/* Output for antenna port RX */
 		MC13192_ANT_CTRL_PORT = 1;		/* Output for antenna port TX */
 		MC13192_ANT_CTRL2 = 1;		/* Signal to turn on RX antenna */
 		MC13192_ANT_CTRL = 1;		/* Signal to turn on TX antenna */
 
 #endif
 
 #if defined (LNA)
    MC13192_LNA_CTRL = LNA_OFF;		  // Turn off the LNA out of reset
    MC13192_LNA_CTRL_PORT  = 1;		  // Enable the port for OUTPUT
 #endif
 
 #if defined (PA)
    MC13192_PA_CTRL = PA_OFF;       // Turn off the PA out of Reset
    MC13192_PA_CTRL_PORT = 1;       // Enable the port for OUTPUT
 #endif
 
}

/**************************************************************
*	Function: 	Switch the MCU from internal to MC13192 supplied clock.
*				The MCU FLL is not engaged.
*	Parameters: None
*	Return:		None
**************************************************************/
void use_external_clock()
{
 ICGC1 = 0x50;         
 while (!ICGS1_ERCS); /* Wait for external reference to be stable. */
 ICGC2_LOLRE = 1;
 ICGC2_MFD = 0x00;		/* Mult factor of 4. */
}

/**************************************************************
*	Function: 	Switch the MCU from external to internal clock.
*	Parameters: None
*	Return:		None
**************************************************************/
void use_mcu_clock()
{
 ICGC1_RANGE = 1;	/* Range High. */
 ICGC1_OSCSTEN = 1;	/* Osc enabled in off mode. */
 ICGC1_CLKS = 0x00;	/* Self clocked. */			
}

/**************************************************************
*	Function: 	Restart the MC13192.
*	Parameters: None
*	Return:		None
**************************************************************/
void MC13192_restart()
{
 rtx_mode = SYSTEM_RESET_MODE;
 IRQSC = 0x14; /* Turn on the IRQ pin. */
 MC13192_RESET = 1; /* Take MC13192 out of reset */
 while (IRQSC_IRQF == 0) /* Poll waiting for MC13192 to assert the irq (i.e. ATTN). */
 {
 }
 drv_read_spi_1(0x24);	/* Clear MC13192 interrupts */
 IRQSC |= 0x04;	/* ACK the pending IRQ interrupt */
 IRQSC = 0x16; /* Pin Enable, IE, IRQ CLR, negative edge. */
}

/**************************************************************
*	Function: 	Reset (continuous) the MC13192.
*	Parameters: None
*	Return:		None
**************************************************************/
void MC13192_cont_reset()
{
 rtx_mode = SYSTEM_RESET_MODE;
 IRQSC = 0x00; /* Set for negative edge. */
 MC13192_RESET = 0; /* Place the MC13192 into reset */
}

/**************************************************************
*	Function: 	Initialize the MCU COP, GPIO, SPI and IRQ.
*				Set the desired MC13192 clock frequency here.
*	Parameters: None
*	Return:		None
**************************************************************/
void mcu_init(void)
{
	__uint16__ irq_reg =0;
	__uint8__ attn_irq = FALSE, timer_hi, timer_lo;
	
	SOPT = 0x73;	/* Turn off the watchdog. */	
	
	rtx_mode = RESET_DELAY;
	
	/* Add a delay to debouce the reset switch on development boards ~200ms */
	TPM1SC = 0x0D;	/* Set the Timer module to use BUSCLK as reference with Prescalar at / 32 */
	
	do 
	{
		timer_hi = TPM1CNTH;	/* Get the value of the timer register (hi byte) */
		timer_lo = TPM1CNTL;	/* Get the value of the timer register (lo byte) */
	}
	while (timer_lo <= 0x80); /* Poll for TIMER LO to be greater than 0x80 at 4MHz/32*/
	
	TPM1SC = 0x00;	/* Return to reset values */

 rtx_mode = SYSTEM_RESET_MODE;
 init_gpio();
 SPI1C1 = 0x50; /* Init SPI */
 SPI1C2 = 0x00;
 SPI1BR = 0x00;
 IRQSC = 0x14; /* Turn on the IRQ pin. */
 rtx_mode = MC13192_RESET_MODE;
 MC13192_RESET = 1; /* Take MC13192 out of reset */

  while (attn_irq == FALSE) {
  	if (IRQSC_IRQF == 1) { /* Check to see if IRQ is asserted */
			irq_reg = drv_read_spi_1(0x24);	/* Clear MC13192 interrupts and check for ATTN IRQ from 13192*/
			irq_reg &= 0x400;
			if (irq_reg == 0)
				attn_irq = FALSE;
			else
				attn_irq = TRUE;
		}
	}
 IRQSC |= 0x04;	/* ACK the pending IRQ interrupt */
 IRQSC = 0x16; /* Pin Enable, IE, IRQ CLR, negative edge. */
 rtx_mode = MC13192_CONFIG_MODE;
}