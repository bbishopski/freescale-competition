/****************************************************************************
* sci.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	sci.c
* 
* File for controlling the serial (SCI) interface.  Used for debugging only.
*
* This file contains the main loop and state machine/event driver.  It
* also contains the timeout functions.
*
****************************************************************************/
#include "device_header.h"
#include "simple_mac.h"
#include "SCI.h"
#include <string.h>

#ifdef BOOTLOADER_ENABLED
    #include "NV_Data.h"
#endif BOOTLOADER_ENABLED

char SCIdata[2];
int SCIdata_flag; 
extern int app_status;

void SCIInit(UINT8 baud)
{

#ifdef BOOTLOADER_ENABLED
	SCIBDH = NV_RAM_ptr->NV_SCI1BDH;
	SCIBDL = NV_RAM_ptr->NV_SCI1BDL;
#else
	SCIBDH = 0x00;
	SCIBDL = baud;
#endif BOOTLOADER_ENABLED

	SCIC2 = initSCI2C2;
}

interrupt void Vscirx()
{	

	__uint8__ status, dummy;


	// ensure Rx data buffer is full
	do {
		status = SCIS1;
	}
	while ((status & 0x20) == 0);
	
	//Check for Errors (Framing, Noise, Parity)
	if ((status & 0x07) != 0) {
		dummy = SCID;
		return;
	}

	//Good Data.
	SCIdata[0] = SCID; // load SCI register to data
	SCIdata[1] = 0; 
	SCIdata_flag = 1;
}

void SCIStartTransmit(char cData)
{
	while (!SCIS1_TDRE) { // ensure Tx data buffer empty
	}
	SCID = cData; // load data to SCI2 register
	while (!SCIS1_TC) { // wait for Tx complete
	}
}

void SCITransmitStr(char *pStr)
{
	int i;
	int nStrLen=strlen(pStr);
	
	for (i=0; i<nStrLen; i++)
	{
		SCIStartTransmit(pStr[i]);
	}
}


void SCITransmitArray(char *pStr, UINT8 length)
{
	int i;
	
	for (i=0; i<length; i++)
	{
		SCIStartTransmit(pStr[i]);
	}
}
