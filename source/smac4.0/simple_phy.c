/**************************************************************
*	This is the SMAC C source physical layer file for the HC(S)08 MCU
*   and MC13192 transceiver.
*   The SMAC phy is the second lowest layer of C code.
**************************************************************/


/**************************************************************
*	Includes
**************************************************************/
#include "MC13192_regs.h"
#include "MC13192_hw_config.h"
#include <hidef.h>
#include "pub_def.h"
#include "drivers.h"
#include "simple_phy.h"
#include "simple_mac.h"
#include "mcu_hw_config.h"

/**************************************************************
*	Globals
**************************************************************/
extern rx_packet_t *drv_rx_packet;
extern byte rtx_mode;

/**************************************************************
* Version string to put in NVM. Note! size limits
**************************************************************/

// Normally it shoud be enough to change the version numbers.
#define PHY_Version		"1.00"
#define PHY_Label       "SPHY "

#pragma MESSAGE DISABLE C3303  // Warning C3303: Implicit concatenation of strings
#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration
#pragma CONST_SEG BOOTLOADER_PHY_NV_DATA0

// DO NOT CHANGE OR REMOVE

// This string will be located in the NV RAM0 section.
// Note!! Check that item is location in the sequence as specified. 
const unsigned char SPHY_Version[47] = "PHY " PHY_Label " Ver " PHY_Version " Build: "__DATE__" "__TIME__;

#pragma CONST_SEG DEFAULT

/**************************************************************
*	Function: 	Transmit data packet
*	Parameters: packet pointer
*	Return:		status
**************************************************************/
int pd_data_request(tx_packet_t *packet)
{
	if (rtx_mode == IDLE_MODE)
	{ 
		drv_write_tx_ram(packet); /* Load the data into packet RAM */
		PLME_set_trx_state_request(TX_MODE); /* transmit it */
		/* Wait for the state to return to idle. (finish transmitting) */
		while (rtx_mode != IDLE_MODE)
		{
			MCU_LOW_POWER_WHILE;
		}
		return SUCCESS;
	}
	else
	{
		return RX_ON;
	}
}

/**************************************************************
*	Function: 	Receive data packet indication
*	Parameters: none
*	Return:		MCPS data indication
**************************************************************/
void pd_data_indication()
{
	/* Read the Data only if it is a good packet. */
	if (drv_rx_packet->status == SUCCESS)
	{
		drv_read_rx_ram(drv_rx_packet); /* read data from MC13192, check status */
	}
	EnableInterrupts; /* Allow system interrupts within the IRQ handler */
	MCPS_data_indication(drv_rx_packet);
}

/**************************************************************
*	Function: 	Hibernate the MC13192 (very low current, no CLKO)
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_hibernate_request(void)
{
	__uint16__ current_value;
	rtx_mode = HIBERNATE_MODE;
	current_value = drv_read_spi_1(MODE2_ADDR);	/* Read MC13192 Hiberate register. */
	current_value &= 0xFFFC;
	current_value |= 0x0002; /* Hiberate enable */
	drv_write_spi_1(MODE2_ADDR, current_value);	/* Write back to MC13192 to enable hibernate mode. */
	return SUCCESS;		/* Call back to MAC layer indicating success. */
}

/**************************************************************
*	Function: 	Doze the MC13192 (Low current, CLKO <= 1MHz)
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_doze_request(void)
{
	__uint16__ current_value;
	rtx_mode = DOZE_MODE;
	current_value = drv_read_spi_1(MODE2_ADDR);	/* Read MC13192 Doze register. */
	current_value &= 0xFFFC;
	current_value |= 0x0001; /* Doze (acoma) enable */
	drv_write_spi_1(MODE2_ADDR, current_value);	/* Write back to MC13192 to enable hibernate mode. */
	return SUCCESS;		/* Call back to MAC layer indicating success. */
}


/**************************************************************
*	Function: 	Doze the MC13192 (Low current, with CLKO enabled,
*             (CLKO must be <= 1MHz)
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_doze_request_wClk(int acomaMode)
{
	__uint16__ current_value;
	rtx_mode = DOZE_MODE;
	current_value = drv_read_spi_1(MODE2_ADDR);	/* Read MC13192 Doze register. */
	
	// bit 9=1, leaves the clk running in doze mode
	current_value &= 0xFDFC;
	
	// set bit 9 and bit 0 to 1
	current_value |= 0x0200; 
	
	if (acomaMode)
	{
	  current_value |= 0x0001; /* Doze (acoma) enable */
	}
	drv_write_spi_1(MODE2_ADDR, current_value);	/* Write back to MC13192 to enable hibernate mode. */
	return SUCCESS;		/* Call back to MAC layer indicating success. */
}


/**************************************************************
*	Function: 	Wake the MC13192 from Hibernate or Doze
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_wake_request(void)
{
	__uint16__ current_value;
	Wake_MC13192(); /* Wake up the device */
	while (rtx_mode != IDLE_MODE_ATTN && rtx_mode != MC13192_RESET_MODE)
	{
		MCU_LOW_POWER_WHILE; /* Wait until ATTN */
	}
	current_value = drv_read_spi_1(MODE2_ADDR);	/* Read MC13192 Hiberate/Doze register. */
	current_value &= 0xFFFC; /* Hiberate and Doze disable */
	drv_write_spi_1(MODE2_ADDR, current_value);	/* Write back to MC13192 to disable hibernate and doze mode. */
	rtx_mode = IDLE_MODE;
	return SUCCESS;
}

/**************************************************************
*	Function: 	Set the MC13192 operating channel
*	Parameters: channel number
*	Return:		status
**************************************************************/
int PLME_set_channel_request(__uint8__ ch)
{
	switch (ch)
	{
	case 0x00:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F95);
		drv_write_spi_1(LO1_NUM_ADDR,0x5000);
	break;
	case 0x01:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F95);
		drv_write_spi_1(LO1_NUM_ADDR,0xA000); 
	break;
	case 0x02:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F95);
		drv_write_spi_1(LO1_NUM_ADDR,0xF000); 
	break;
	case 0x03:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F96);
		drv_write_spi_1(LO1_NUM_ADDR,0x4000); 
	break;
	case 0x04:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F96);
		drv_write_spi_1(LO1_NUM_ADDR,0x9000); 
	break;
	case 0x05:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F96);
		drv_write_spi_1(LO1_NUM_ADDR,0xE000); 
	break;
	case 0x06:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F97);
		drv_write_spi_1(LO1_NUM_ADDR,0x3000); 
	break;
	case 0x07:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F97);
		drv_write_spi_1(LO1_NUM_ADDR,0x8000); 
	break;
	case 0x08:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F97);
		drv_write_spi_1(LO1_NUM_ADDR,0xD000); 
	break;
	case 0x09:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F98);
		drv_write_spi_1(LO1_NUM_ADDR,0x2000); 
	break;
	case 0x0A:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F98);
		drv_write_spi_1(LO1_NUM_ADDR,0x7000); 
	break;
	case 0x0B:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F98);
		drv_write_spi_1(LO1_NUM_ADDR,0xC000); 
	break;
	case 0x0C:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F99);
		drv_write_spi_1(LO1_NUM_ADDR,0x1000); 
	break;
	case 0x0D:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F99);
		drv_write_spi_1(LO1_NUM_ADDR,0x6000); 
	break;
	case 0x0E:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F99);
		drv_write_spi_1(LO1_NUM_ADDR,0xB000); 
	break;
	case 0x0F:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F9A);
		drv_write_spi_1(LO1_NUM_ADDR,0x0000); 
	break;
	default:
		drv_write_spi_1(LO1_IDIV_ADDR,0x0F97);
		drv_write_spi_1(LO1_NUM_ADDR,0xD000); 
		return ERROR;
	}
	return SUCCESS;
}

/**************************************************************
*	Function: 	Set the MC13192 transceive operation
*	Parameters: operational mode
*	Return:		status
**************************************************************/
int PLME_set_trx_state_request(__uint8__ req_mode)
{
	__uint16__ reg;
	DeAssertRTXEN(); 
	reg = drv_read_spi_1(MODE_ADDR);
	reg &= 0xFFF8; /* Clear mode. */
	switch (req_mode)
	{
		case IDLE_MODE: /* Write Idle */
		  #if defined (LNA)
		    MC13192_LNA_CTRL = LNA_OFF;
		  #endif
		  
		  #if defined (PA)
		    MC13192_PA_CTRL = PA_OFF;
		  #endif
		  
			reg |= IDLE_MODE;
			rtx_mode = IDLE_MODE;
			drv_write_spi_1(MODE_ADDR, reg);
			break;
		case RX_MODE: /* Write RX */
		
		  #if defined (LNA)
		    MC13192_LNA_CTRL = LNA_ON;
		  #endif

		  #if defined (PA)
		    MC13192_PA_CTRL = PA_OFF;
		  #endif
		
			#if defined (ANTENNA_SWITCH)
				MC13192_ANT_CTRL2 = ANT_CTRL_ON;	/* Turn on the RX antenna */
				MC13192_ANT_CTRL = ANT_CTRL_OFF;	/* Turn off the TX antenna */
			#endif
			
			rtx_mode = RX_MODE;
			reg |= RX_MODE;
			drv_write_spi_1(MODE_ADDR, reg);
			AssertRTXEN(); 
			break;
		case RX_MODE_WTO: /* Write RX, but set rtx_mode to timeout */
		
			#if defined (LNA)
		    MC13192_LNA_CTRL = LNA_ON;
		  #endif
		
		  #if defined (PA)
		    MC13192_PA_CTRL = PA_OFF;
		  #endif

			#if defined (ANTENNA_SWITCH)
				MC13192_ANT_CTRL2 = ANT_CTRL_ON;	/* Turn on the RX antenna */
				MC13192_ANT_CTRL = ANT_CTRL_OFF;	/* Turn off the TX antenna */
			#endif
			
			rtx_mode = RX_MODE_WTO;
			reg |= RX_MODE;
			drv_write_spi_1(MODE_ADDR, reg);
			AssertRTXEN();
			break;
		case TX_MODE: /* Write Tx. Note: force LO lock not used */

		  #if defined (PA)
  	    MC13192_PA_CTRL = PA_ON;
		  #endif
		
		  #if defined (LNA)
		    MC13192_LNA_CTRL = LNA_OFF;
		  #endif
		
			#if defined (ANTENNA_SWITCH)
				MC13192_ANT_CTRL2 = ANT_CTRL_OFF;	/* Turn off the RX antenna */
				MC13192_ANT_CTRL = ANT_CTRL_ON;	/* Turn on the TX antenna */
			#endif
						
			reg |= TX_MODE;
			rtx_mode = TX_MODE;
			drv_write_spi_1(MODE_ADDR, reg);
			AssertRTXEN();
			break;
		default:
			return ERROR;	
	}
	return SUCCESS;
}

/**************************************************************
*	Function: 	Measure channel energy
*	Parameters: none
*	Return:		energy
**************************************************************/
__uint8__ PLME_energy_detect (void)
/* Note: Actual power returned is: -(power/2) */
/* Global calibration required for accuracy. */
{
  __uint16__ reg;
  __uint8__ power;
  rtx_mode = CCA_MODE; /* Write energy detect mode */
  reg = drv_read_spi_1(MODE_ADDR);
  reg &= 0xFFF8;
  reg |= CCA_MODE;
  drv_write_spi_1(MODE_ADDR, reg);
  AssertRTXEN();
  while (rtx_mode != IDLE_MODE) /* Wait for energy detect to complete */
  {
	MCU_LOW_POWER_WHILE;
  }
  reg = (drv_read_spi_1(CCA_RESULT_ADDR) & 0xFF00);
  power = (reg >> 8);
  return power;
}

/**************************************************************
*	Function: 	Report energy from last successful RX packet
*	Parameters: none
*	Return:		energy
**************************************************************/
__uint8__ PLME_link_quality (void)
/* Note: Actual power returned is: -(power/2) */
/* Global calibration required for accuracy. */
{
  __uint16__ reg;
  __uint8__ power;
  reg = drv_read_spi_1(CCA_RESULT_ADDR);
  power = ((reg & 0xFF00) >> 8);
  return power;
}

/**************************************************************
*	Function: 	Get MC13192 timer value
*	Parameters: none
*	Return:		timer value
**************************************************************/
void PLME_get_time_request(__uint32__ *time)
{
	__uint32__ upperword, lowerword;
	__uint32__ current_time;
	upperword = drv_read_spi_1(TIMESTAMP_HI_ADDR);
	lowerword = drv_read_spi_1(TIMESTAMP_LO_ADDR);
	upperword &= TIMESTAMP_HI_MASK;	/* Clears TS_HELD bit. */
	current_time = (__uint32__) (upperword << 16) | lowerword;
  *time = current_time;	
}

/**************************************************************
*	Function: 	Set MC13192 CLKo frequency
*	Parameters: frequency value
*	Return:		status
**************************************************************/
int PLME_set_MC13192_clock_rate(__uint8__ freq)
{
	volatile __uint16__ current_value;
	current_value = drv_read_spi_1(CLKS_ADDR); /* Read register and re-write */
	current_value &= 0xFFF8;
	current_value |= freq;
	drv_write_spi_1(CLKS_ADDR, current_value);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Set MC13192 timer frequency
*	Parameters: frequency value
*	Return:		status
**************************************************************/
int PLME_set_MC13192_tmr_prescale (__uint8__ freq) 
{
	volatile __uint16__ current_value;
	current_value = drv_read_spi_1(PRESCALE_ADDR);
	current_value &= 0xFFF8;
	current_value |= freq;
	drv_write_spi_1(PRESCALE_ADDR, current_value);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Set MC13192 timer value (i.e. initialize)
*	Parameters: timer value
*	Return:		none
**************************************************************/
void PLME_set_time_request(__uint32__ requested_time)
{
	__uint16__ upperword, lowerword, mode2_reg_val;
	/* Split 32 bit input into 2 16 bit values */
	upperword = (__uint16__) (requested_time >> 16) & 0x000000FF;
	lowerword = (__uint16__) requested_time & 0x0000FFFF;
	/* Program Time1 comparator with the desired value */	
	drv_write_spi_1(T1_HI_ADDR, upperword);
	drv_write_spi_1(T1_LO_ADDR, lowerword);
	/* Get current state of the MODE2 MC13192 register */
	mode2_reg_val = drv_read_spi_1(MODE2_ADDR);
	/* Set the Tmr_load bit */
	mode2_reg_val |= 0x8000;
	/* Now write the value back to MC13192 register MODE2 */
	drv_write_spi_1(MODE2_ADDR, mode2_reg_val);
	/* Clear the tmr_load bit */
	mode2_reg_val &= 0x7FFF;
	/* Clr the tmr_load bit to prepare for next set_time_request. */
	drv_write_spi_1(MODE2_ADDR, mode2_reg_val);	
	return;
}

/**************************************************************
*	Function: 	Set MC13192 timer compare value
*	Parameters: timer value
*	Return:		status
**************************************************************/
int PLME_enable_MC13192_timer1(__uint32__ counter_value)
{
	/* Load the timeout value into T1 with Timer disabled. */
	drv_write_spi_1(T1_HI_ADDR, (__uint16__) ((counter_value >> 16) & 0x000000FF) | 0x000080FF);
	drv_write_spi_1(T1_LO_ADDR, (__uint16__) (counter_value & 0x0000FFFF));
	/* Turn Timer1 mask on. */
	drv_write_spi_1(T1_HI_ADDR, (__uint16__) ((counter_value >> 16) & 0x000000FF));
	drv_write_spi_1(T1_LO_ADDR, (__uint16__) (counter_value & 0x0000FFFF));
	return SUCCESS;
}

/**************************************************************
*	Function: 	Disable MC13192 timer comparator TC1
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_disable_MC13192_timer1(void)
{
	/* Load the timeout value into T1 with Timer disabled. */
	/* Clear Timer1 if in RX_MODE_WTO */
	drv_write_spi_1(T1_HI_ADDR, 0x8000);
	drv_write_spi_1(T1_LO_ADDR, 0x0000);
//	irq_mask_reg = drv_read_spi_1(IRQ_MASK);
//	irq_mask_reg &= ~TIMER1_IRQMASK_BIT;
//	drv_write_spi_1(IRQ_MASK, irq_mask_reg);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Indicate a MC13192 reset condition
*	Parameters: none
*	Return:		none
**************************************************************/
void PLME_MC13192_reset_indication (void)
{
	MLME_MC13192_reset_indication();
}

/**************************************************************
*	Function: 	Force the MC13192 into a soft reset condition
*	Parameters: none
*	Return:		status
**************************************************************/
int PLME_MC13192_soft_reset(void)
{
	drv_write_spi_1(RESET, 0x00);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Adjust the MC13192s crystal trim value
*	Parameters: trim
*	Return:		status
**************************************************************/
int PLME_MC13192_xtal_adjust(__int8__ trim_value)
{
	__uint16__ reg;
	__uint16__ reg_value;
	reg_value = (trim_value << 8);	/* Shift the req value into the higher half word */
	reg = drv_read_spi_1(XTAL_ADJ_ADDR);	/* Read the current value of XTAL Reg */
	reg = ((reg & 0x00FF) | reg_value);
	drv_write_spi_1(XTAL_ADJ_ADDR, reg);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Adjust the MC13192s gain compensator
*	Parameters: gain compensation
*	Return:		status
**************************************************************/
int PLME_MC13192_FE_gain_adjust(__int8__ gain_value)
{
	__uint16__ reg;
	reg = drv_read_spi_1(FEGAIN_ADDR);	/* Read the current value of GAIN Reg */
	reg = ((reg & 0xFF00) | gain_value);
	drv_write_spi_1(FEGAIN_ADDR, reg);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Adjust the MC13192s Output power
*	Parameters: PA Output adjust
*	Return:		status
**************************************************************/
int PLME_MC13192_PA_output_adjust(__uint8__ requested_pa_value)
{
	__uint16__ reg;
	__uint8__ pa_value;
	int status = SUCCESS;
	switch (requested_pa_value)
	{
		case MAX_POWER:	/* Sets the PA drive level and PA gain to MAX. */
			pa_value = 0xFF;
			break;
		case MIN_POWER:
			pa_value = 0x00; //Sets the PA drive level and PA gain to min.
			break;
		default:
			if (requested_pa_value > 15)
			{
				return OVERFLOW;
			}
			else
			{
				pa_value = requested_pa_value;
			}
			break;
	}
	
	reg = drv_read_spi_1(PA_ADJUST_ADDR);	/* Read the current value of GAIN Reg */
	reg &= 0xFF00;
	
	if ((requested_pa_value == MAX_POWER) || (requested_pa_value == MIN_POWER))
		reg |= pa_value;
	else {
		reg |= ((pa_value << 4) | 0x000C);
	}
	drv_write_spi_1(PA_ADJUST_ADDR, reg);
	return SUCCESS;
}

/**************************************************************
*	Function: 	Returns the RFIC version number.
*	Parameters: none
*	Return:		version number
**************************************************************/
__uint8__ PLME_get_rfic_version(void)
{
	__uint16__ reg;
	reg = drv_read_spi_1(VERSION_REG);	/* Read the version register version[12:10] */
	reg &= VERSION_MASK;			/* Shift to generate accurate number */
	
	reg = reg >> 10;				/* Hard coded to shift */
	return (__uint8__) reg;
}

