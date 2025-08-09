/**************************************************************
*	This is the SMAC C source media (i.e. MAC) layer file for the HC(S)08 MCU
*   and MC13192 transceiver.
*   The SMAC MAC is the highest layer of C code for the SMAC.
**************************************************************/


/**************************************************************
*	Includes
**************************************************************/
#include "pub_def.h"
#include "drivers.h"
#include "simple_phy.h"
#include "simple_mac.h"

/**************************************************************
*	Externals
**************************************************************/
extern rx_packet_t *drv_rx_packet;
extern byte rtx_mode;

/**************************************************************
* Version string to put in NVM. Note! size limits
**************************************************************/

// Normally it shoud be enough to change the version numbers.
#define Database_Label_Version	"1.00"
#define MAC_Version		        "1.00"
#define MAC_Label               "SMAC "

#pragma MESSAGE DISABLE C3303  // Warning C3303: Implicit concatenation of strings
#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration

#pragma CONST_SEG BOOTLOADER_MAC_NV_DATA0

// DO NOT CHANGE OR REMOVE

// These strings will be located in the NV RAM0 section.
// Note!!Check that items are location in the same sequence as specified. 
const unsigned char Freescale_Copyright[54] = "(c) Copyright 2004 Freescale Inc. All rights reserved";
const unsigned char Firmware_Database_Label[40] = "DB Label: XXXXXXXXXXXXXXXXXXXX Ver " Database_Label_Version;
const unsigned char SMAC_Version[47] = "MAC " MAC_Label " Ver " MAC_Version " Build: "__DATE__" "__TIME__;

#pragma CONST_SEG DEFAULT


/**************************************************************
*	Function: 	Transmit data packet
*	Parameters: packet pointer
*	Return:		status
**************************************************************/
int MCPS_data_request(tx_packet_t *packet)
{
	__uint8__ status;
	/* Send it to the phy for processing */
	status = pd_data_request(packet);
	return status;
}

/**************************************************************
*	MCPS_data_indication
*	Function: 	Receive data packet indication
*	Parameters: data packet pointer
*	Notes: This function return should be located in the application
**************************************************************/

/**************************************************************
*	Function: 	Hibernate the MC13192 (very low current, no CLKO)
*	Parameters: none
*	Return:		status
**************************************************************/
int MLME_hibernate_request(void)
{
	__uint8__ status = 0;
	status = PLME_hibernate_request();
	return status;
}

/**************************************************************
*	Function: 	Doze the MC13192 (Low current, with CLKO enabled,
*             (CLKO must be <= 1MHz)
*	Parameters: none
*	Return:		status
**************************************************************/
int MLME_doze_request_wClk(int acomaMode)
{
	__uint8__ status = 0;
	status = PLME_doze_request_wClk(acomaMode);
	return status;
}


/**************************************************************
*	Function: 	Doze the MC13192 (Low current, CLKO <= 1MHz)
*	Parameters: none
*	Return:		status
**************************************************************/
int MLME_doze_request(void)
{
	__uint8__ status = 0;
	status = PLME_doze_request();
	return status;
}

/**************************************************************
*	Function: 	Wake the MC13192 from Hibernate or Doze
*	Parameters: none
*	Return:		status
**************************************************************/
int MLME_wake_request(void)
{
	__uint8__ status = 0;
	status = PLME_wake_request();
	return status;
}

/**************************************************************
*	Function: 	Set the MC13192 operating channel
*	Parameters: channel number (0-15)
*	Channel frequencies:
*	0: 2.405GHz
*	1: 2.410GHz
*	2: 2.415GHz
*	3: 2.420GHz
*	4: 2.425GHz
*	5: 2.430GHz
*	6: 2.435GHz
*	7: 2.440GHz
*	8: 2.445GHz
*	9: 2.450GHz
*	10: 2.455GHz
*	11: 2.460GHz
*	12: 2.465GHz
*	13: 2.470GHz
*	14: 2.475GHz
*	15: 2.480GHz
*	Return:		status
**************************************************************/
int MLME_set_channel_request(__uint8__ ch)
{
	__uint8__ status = 0;
	status = PLME_set_channel_request(ch);
	return status;
}

/**************************************************************
*	Function: 	Set the MC13192 receiver ON (with optional timeout)
*	Parameters: packet pointer for received data and timeout
*	Return:		status
*	Notes:		Timeout of 0 disables the timeout.
*				The actual timeout period is the timeout value times
*				the MC13192 timer rate from MLME_set_MC13192_tmr_prescale.
**************************************************************/
int MLME_RX_enable_request(rx_packet_t *rx_packet, __uint32__ timeout)
{
	__uint8__ status = 0;
	__uint32__ current_time;
	drv_rx_packet = rx_packet;	/* Assign the rx_packet to SMAC global. */
	if (timeout == 0) /* Timeout disabled */
	{
		status = PLME_set_trx_state_request(RX_MODE); /* Just enable the receiver */
	}
	else /* Timeout requested. Get the current time and add the timeout value. */
	{	
		PLME_get_time_request(&current_time);
		current_time +=  timeout;
		status = PLME_enable_MC13192_timer1(current_time); /* Set the timeout in TC1 */
  		status = PLME_set_trx_state_request(RX_MODE_WTO);
	}
	return status;
}

/**************************************************************
*	Function: 	Set the MC13192 receiver OFF
*	Parameters: None
*	Return:		status
**************************************************************/
int MLME_RX_disable_request(void)
{
	PLME_disable_MC13192_timer1(); /* In case the timeout is being used, disable it also */
	if (PLME_set_trx_state_request(IDLE_MODE) == SUCCESS) /* Attempt to disable the timer */
	{
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}

/**************************************************************
*	Function: 	Set MC13192 CLKo frequency
*	Parameters: frequency value enumeration (0-7)
*	Return:		status
*	Notes:
*	FREQ 	OUTPUT_FREQ
*	0 		16MHz (Recommended default)
*	1 		8MHz
*	2 		4MHz
*	3 		2MHz
*	4 		1MHz
*	5 		62.5kHz
*	6 		32.786kHz
*	7 		16.393kHz
**************************************************************/
int MLME_set_MC13192_clock_rate(__uint8__ freq)
{
	__uint8__ status;
	status = PLME_set_MC13192_clock_rate(freq);
	return status;
}

/**************************************************************
*	Function: 	Set MC13192 timer frequency
*	Parameters: frequency value enumeration (0-7)
*	Return:		status
*	Notes:
*	FREQ 	OUTPUT_FREQ
*	0 		2MHz
*	1 		1MHz
*	2 		500kHz
*	3 		250kHz (Recommended default)
*	4 		125kHz
*	5 		62.5kHz
*	6 		31.25kHz
*	7 		15.625kHz
**************************************************************/
int MLME_set_MC13192_tmr_prescale (__uint8__ freq)
{
	__uint8__ status;
	status = PLME_set_MC13192_tmr_prescale (freq);
	return status;
}

/**************************************************************
*	Function: 	Measure channel energy
*	Parameters: none
*	Return:		energy
*	Notes: 
*	Actual power returned is: -(power/2)
*	Global calibration required for accuracy (from MLME_MC13192_FE_gain_adjust).
**************************************************************/
__uint8__ MLME_energy_detect (void)
{
  __uint8__ power;
  power = PLME_energy_detect();
  return power;
}

/**************************************************************
*	Function: 	Report energy from last successful RX packet
*	Parameters: none
*	Return:		energy
*	Notes: 
*	Actual power returned is: -(power/2)
*	Global calibration required for accuracy (from MLME_MC13192_FE_gain_adjust).
**************************************************************/
__uint8__ MLME_link_quality (void)
{
  __uint8__ power;
  power = PLME_link_quality ();
  return power;
}

/**************************************************************
*	Function: 	Force the MC13192 into a soft reset condition
*	Parameters: none
*	Return:		status
**************************************************************/
int MLME_MC13192_soft_reset(void)
{
	/* Performs a soft reset of MC13192 via writing to register 0*/
	int status;
	status = PLME_MC13192_soft_reset();
	return status;
}

/**************************************************************
*	Function: 	Adjust the MC13192s crystal trim value
*	Parameters: trim value (0-255)
*	Return:		status
**************************************************************/
int MLME_MC13192_xtal_adjust(__uint8__ req_value)
{
  int status;
  status = PLME_MC13192_xtal_adjust(req_value);
  return status;
}

/**************************************************************
*	Function: 	Adjust the MC13192s gain compensator
*	Parameters: gain compensation value (0 to 255. 128 is center point)
*	Return:		status
**************************************************************/
int MLME_MC13192_FE_gain_adjust(__uint8__ gain_value)
{
  int status;
  status = PLME_MC13192_FE_gain_adjust(gain_value);
  return status;
}

/**************************************************************
*	Function: 	Adjust the Output power of the transmitter
*	Parameters: Course Value
*	Return:		status
**************************************************************/
int MLME_MC13192_PA_output_adjust(__uint8__ pa_value)
{
  int status;
  status = PLME_MC13192_PA_output_adjust(pa_value);
  return status;
}

/**************************************************************
*	Function: 	Reads the version number of the IC
*	Parameters: none
*	Return:		The version number of the IC.
*
**************************************************************/
__uint8__ MLME_get_rfic_version(void)
{
  __uint8__ version;
  version = PLME_get_rfic_version();
  return version;
}

