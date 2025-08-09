/************************************************************************************
* This file contains Platform specific implentations of BTA functions.
* For the Windows Simulator Platform.
*
* Author(s): Thomas O. Jensen
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
typedef enum ePhyEnums_t phyTxRxState_t;

/************************************************************************************
* Includes
************************************************************************************/
#include "DigiType.h"
#include "phy_spi.h"
#include "AbelReg.h"
#include "gb60_io.h"
#include "MacPhy.h"
#include "Crt0.h"
#include "ICG.h"
#include "NV_Data.h"
#include "PhyMacMsg.h"
#include "Debug.h"
#include "FreeLoader_inf.h"

#define ToBigEndianL2S(pLongIn) (*(((short*)pLongIn)+1) | *((short*)pLongIn))
#define ToNativeEndianS2L(pLongIn) (*(((short*)pLongIn)+1) | *((short*)pLongIn))

// Prototypes to be used by phy
void IrqCcaComplete(bool_t channelInUse);
//void RxEof(bool_t crcValid);
//void TxEof(void);
extern void DoFastRxEof(void);
extern void DoFastTxEof(void);
void RxTimeout(void);
extern uint16_t gIsrMask;
extern bool_t gStopAsyncRx;
extern bool_t gSeqInErrorState;
extern void SeqIsrCompletedInErrorState(void);
extern void ReStartRx(void);
extern void SeqGenerateWakeInd(void);
extern uint8_t gIsrEntryCounter;
#pragma PLACE_DATA_SEG(SLEEP_VARIABLE)
extern uint8_t gSeqPowerSaveMode;
#pragma RESTORE_DATA_SEG
extern phyTxRxState_t mPhyTxRxState;

extern void GenerateTc2SyncLossIndication(void);
#define GenerateSyncLoss GenerateTc2SyncLossIndication

// Needed to initialize Timer1 vector
void DummyISR(void)
{
  ;
}
void (*pTimer1Isr)(void) = DummyISR; // Makes it possible to dynamically change interrupt vector for testing!
extern void (*gpTimer2Isr)(void);
extern  void (*mpfPendingSetup)(void);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
uint8_t StreamingRxError; //Counts up when a StreamingRx interrupt is received, but not
                          //serviced by gIsrFastAction as expected
uint8_t StreamingTxError; //Counts up when a StreamingTx interrupt is received, but not
                          //serviced by gIsrFastAction as expected

/*************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*  SPI configured as ABEL interface                                                  *
************************************************************************************/
void InitAbelSPI(void)
{
  // SPI Bit Clock Rate = BUSCLK / 2
  PTED = 0x04;	//PTED.2=SS = LOW  : 0x00 for autoselect
  PTEPE = 0x00;
  PTEDD |= 0x34;

  SPIC1 = 0x50; //SPIC1.1 = SSOE   : 0x52 for autoselect
  SPIC2 = 0x00; //SPIC2.4 = MODEEN : 0x10 for autoselect
  SPIBR = 0x00;
}

/************************************************************************************
* All the complete systems I/O port definitions are described here                  *
************************************************************************************/
void InitAllProjectsIO(void)
{
#ifdef I_AM_A_SNIFFER
  TPM1SC = 0x0c; // Use BUSCLK and prescale with factor 16 (to get 1 us)
  TPM1C1SC = 0x04; // Disable interrupt, Input capture, Capture on rising edge
#endif I_AM_A_SNIFFER

  // Setup port C
  mSETUP_PORT_C

  // Setup port A
  mSETUP_PORT_A

  ENABLE_TIMING_PINS          //Target.h macro using PORT A
  ENABLE_ANTENNESWITCH_PINS   //Target.h macro using PORT A
  ENABLE_ISR_TIMING_PINS      //Target.h macro using PORT A
  RX_ANTENNE_ENABLED
  
// Setup port D
  mSETUP_PORT_D
   // Setup Port B
  mSETUP_PORT_B

  RxTxEnable                   // RxTxEnable static = 1;
  HWAssertAbelReset            // keep abel in reset for now

}

/*******************************************************************
* Make a pulse on hardware reset PIN at ABEL                       *
*******************************************************************/
void HWResetAbel(void)
{
  HWAssertAbelReset     //To enshure time goes....
  HWAssertAbelReset
  HWAssertAbelReset
  HWAssertAbelReset

  HWDeAssertAbelReset
}

/*******************************************************************
* Now use SPI to initialize ABEL                                   *
*******************************************************************/
void AbelRegisterSetup(void)
{
  uint16_t reg9;
  uint16_t retReg;

  //ffj init
  ABEL_WRITE(0x04,NV_RAM_ptr->Abel_HF_Calibration);  //MSB=CCA Energy detect threshold: abs(power in dBm)*2, 0x96=-75dBm
                            //LSB=Power compensation Offset added to RSSI. Typical 0x74 FFJ/JK 13/01/04 (Abel 013)
  ABEL_WRITE(0x08,0xFFe7);  //R0x08: 0xFFE7 (if you can't receive packets try: FFE5)

#define MHZ_2405
#ifdef MHZ_2405
  ABEL_WRITE(0x0F,0x0F95);  //0x0F8F for 2305Mhz, 0x0F95 for 2405Mhz
  ABEL_WRITE(0x10,0x5000);  //0x1000 for 2305MHz, 0x5000 for 2405MHz
#endif

#ifdef MHZ_2305
  ABEL_WRITE(0x0F,0x0F8F);  //0x0F8F for 2305Mhz, 0x0F95 for 2405Mhz
  ABEL_WRITE(0x10,0x1000);  //0x1000 for 2305MHz, 0x5000 for 2405MHz
#endif

  ABEL_WRITE(0x11,0x80ff);
  ABEL_WRITE(0x12,0x00Bc);  //V1.2:0x80CC, default V2.0:0x000C :     PA level
  ABEL_WRITE(0x13,0x1843);  //V1.2:0x1803, default V2.0:0x1863 : Sigma-delta fade detector differs

  ABEL_WRITE(0x31,0xA000);

#ifdef I_AM_A_SNIFFER
  ABEL_WRITE(0x38,0x0020);   //Correlator VT
#else
  ABEL_WRITE(0x38,0x0008);   //Correlator VT, 0x0000=Abel 1.2
#endif I_AM_A_SNIFFER

{
  // Write Abel Mask registers ("Control2" and "Mask")  
  uint16_t ctrl2val,maskval;
#if gAspCapability_d
  maskval = cATTN_MASK | cTMR4_MASK | cTMR2_MASK | cDOZE_MASK;
#else
  maskval = cATTN_MASK | cTMR4_MASK | cTMR2_MASK;
#endif
  
#ifdef USE_INTERRUPT_EOF
  ctrl2val=0x7c00 | cABEL2SPI_MASK | cTX_DONE_MASK | cRX_DONE_MASK | cUSE_STRM_MODE;
#else
  ctrl2val=0x7c00 | cABEL2SPI_MASK | cUSE_STRM_MODE;
#endif
#ifdef LO_LOCK_DETECT_HACK
  ctrl2val |= cHG_BIAS_EN;               // Enable Thermometer circuitry (must be enabled at all times, when using thermometer) 
  maskval  |= cLO1_LOCK_MASK | cHG_MASK; // Enable Lo1Lock and Thermometer irqs
#endif

  maskval  |= cLO1_LOCK_MASK;
/*
#ifdef I_AM_A_SNIFFER
    maskval |= cLO1_LOCK_MASK;
#endif I_AM_A_SNIFFER
*/

  ABEL_WRITE(ABEL_CONTROL2_REG, ctrl2val);
  ABEL_WRITE(ABEL_MASK_REG, maskval);
}
#ifdef LO_LOCK_DETECT_HACK
    // Configure Thermometer timer to approx 144us
  ABEL_WRITE(0x0D, 0x1004); 
  ABEL_WRITE(0x0E, 0x8002);
#endif /*LO_LOCK_DETECT_HACK*/


    
  ABEL_READ(ABEL_reg9, reg9);
  reg9 &= ~cTMR_PRESCALEmask;
  reg9 |= 5 << cTMR_PRESCALEshift;    //5=62.5kHz, 3=250kHz(default)
  reg9 |= 0x80;                       // Alternate GPIO enable
  ABEL_WRITE(ABEL_reg9, reg9);

  ABEL_WRITE(ABEL_TMR1_HI_REG, 0x8000);    //stop all wrap around interrupts
  ABEL_WRITE(ABEL_TMR2_HI_REG, 0x8000);
  ABEL_WRITE(ABEL_TMR3_HI_REG, 0x8000);
  ABEL_WRITE(ABEL_TMR4_HI_REG, 0x8000);
  ABEL_READ(ABEL_reg24, retReg);

  ABEL_WRITE(ABEL_TMR2_HI_REG, 0x0000);

  ABEL_WRITE(ABEL_regB, 0x01fc);

  IRQSC = 0x17; // Enable external interrupt from Abel, low level and edge - triggered
}

/*************************************************************************************
* Used by JT to make PHY speed test (Header calculation)                             *
*************************************************************************************/

#if gAspCapability_d

void WakeUpIsr(void)
{
  Dbg_SetPortA0(0);

  SeqGenerateWakeInd();
  gSeqPowerSaveMode = 0;

#ifdef LO_LOCK_DETECT_HACK
  ABEL_WRITE(ABEL_CONTROL2_REG, 0x7c00 | cABEL2SPI_MASK | cTX_DONE_MASK | cRX_DONE_MASK | cUSE_STRM_MODE | cHG_BIAS_EN);
#else
  ABEL_WRITE(ABEL_CONTROL2_REG, 0x7c00 | cABEL2SPI_MASK | cTX_DONE_MASK | cRX_DONE_MASK | cUSE_STRM_MODE);
#endif

  // Setup clock again
 #if defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
	// Call version in bootloader
	FL_ICG_Setup();
#else
	ICG_Setup(); // Reuse code in function to reduce code size
#endif defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
}
#endif // gAspCapability_d

/*************************************************************************************
* Must signal to gIsrAsyncTailFunction/Header calculation, that the calculation must *
* stop now, or receiver to be restarted.                                             *
*************************************************************************************/
void LO_LOCK_IRQ(void)
{

#ifdef I_AM_A_SNIFFER 
//extern uint16_t LockISRCounter;
//  LockISRCounter++;
extern uint16_t gPhyCurrentiDiv;

  ReStartRx(); // Will not suffice, since LO_LOCK has been observed on CCA!

  mRestartLockTimer

  {uint16_t iDiv = (0x8000 | gPhyCurrentiDiv);ABEL_WRITE_INT(ABEL_LO1DIV_REG, iDiv);}

  
  //TOGGLE_RED_LED;

#else
    ReStartRx(); 
    if (mPhyTxRxState!=cBusy_Rx) GenerateSyncLoss();
#endif I_AM_A_SNIFFER 

}

/*************************************************************************************
*                 *
*************************************************************************************/
#ifdef I_AM_A_SNIFFER 
__interrupt void LO_LOCK_DETECT_ISR(void)
{
extern uint16_t gPhyCurrentiDiv;

  TPM1C0SC &= 0x3f;  // Clear interrupt flag and disable
  ABEL_WRITE_INT(ABEL_LO1DIV_REG, (gPhyCurrentiDiv));

}
#endif I_AM_A_SNIFFER 

#ifdef LO_LOCK_DETECT_HACK
void ForceLo1LockDetectIsr(void)
{
  CLRTIMINGPIN_3
  if (mPhyTxRxState!=cBusy_Rx) return;
  
    gIsrFastAction = IrqStreamingRxFirst;  

  // This is the thermometer interrupt! Used here to time when abel warmup is complete
  // Toggle Loop filter PreCharge on and off to enable Lo1Lock detection
  {
    uint16_t iDiv,iDivTmp;
    ABEL_READ_INT(ABEL_LO1DIV_REG, iDiv);
    iDivTmp= 0x8000 | iDiv;
    ABEL_WRITE_INT(ABEL_LO1DIV_REG, iDivTmp);
    ABEL_WRITE_INT(ABEL_LO1DIV_REG, iDiv);
  }

}
#endif

/*************************************************************************************
* This is where everything happens...  Abel driver runs through here                 *
*************************************************************************************/
__interrupt void AbelInterrupt(void)
{
  uint16_t retReg;
  uint8_t *pTmp;
  register uint8_t tmpStatus;

#ifdef TIME_INTERRUPT
  uint8_t tmpRunning;
  tmpRunning = CHECK_ABEL_ISR_RUNNING_1;
  ABEL_ISR_RUNNING_1;
#endif
  ISR_DISABLED_0;

  if (gIsrSuperFastAction){
    gIsrSuperFastAction();
    gIsrSuperFastAction=NULL;
    mpfPendingSetup = NULL;
  }

  tmpStatus = IRQSC;
  tmpStatus |=  0x04;   // IACK, MUST be done before RxStreaming...
  tmpStatus &=  ~0x02;  // 0x02 = DISABLE ABEL ISR
  IRQSC = tmpStatus;


  SETTIMINGPIN_0
  gIsrEntryCounter++;
  gIsrMask = 0xFFFF;
  ENABLE_ALL_INTERRUPTS;    // Interrupts are always enabled in this system. Disable ABEL only when needed



  (gIsrFastAction)();
  if(IRQSC & 0x08)  //IRQF test
  {
    IRQSC |=  0x04;   //IACK, Could be initiated by new/next edge.

    ABEL_READ_INT(ABEL_reg24, retReg);
    retReg = retReg & gIsrMask;
    if (retReg & 0xFF00)
    {
      if (retReg & cLO_LOCK_IRQ){   LO_LOCK_IRQ(); goto IsrExit;}
#ifdef USE_INTERRUPT_EOF
      if (retReg & cTX_DONE){ GenerateSyncLoss();  goto IsrExit;}
      if (retReg & cRX_DONE){ gStopAsyncRx = TRUE;GenerateSyncLoss(); goto IsrExit;}
#endif
      if (retReg & cTMR1_IRQ)      pTimer1Isr();
#if gAspCapability_d
      if (retReg & cResetATTN_IRQ) WakeUpIsr();
      if (retReg & cDOZE_IRQ)      WakeUpIsr();
#endif // gAspCapability_d

#ifdef LO_LOCK_DETECT_HACK
      if (retReg & cHG_IRQ)        ForceLo1LockDetectIsr();
      if (retReg & cSTRM_DATA_ERR) DummyISR();
#else
      if (retReg & (cSTRM_DATA_ERR | cHG_IRQ | cLO_LOCK_IRQ )) DummyISR();
#endif
    }

    if (retReg & 0x00FC)
    {
      if (retReg & cCCA_IRQ)       IrqCcaComplete((bool_t)(retReg & cCCA)); // If set, channel is busy
      if (retReg & cTMR4_IRQ){     RxTimeout();  goto IsrExit;}
      if (retReg & cTMR2_IRQ)      gpTimer2Isr();
      if (retReg & cTMR3_IRQ)      DummyISR();
      if (retReg & cRX_RCVD_IRQ)   StreamingRxError++;
      if (retReg & cTX_SENT_IRQ)   StreamingTxError++;
    }
  }

  gIsrAsyncTailFunction();
IsrExit:
    // Assume that interrupts are DISABLED at this point!
  pTmp = &gIsrEntryCounter;
  if (!(--*pTmp)){
    if (gSeqInErrorState){ 
      SeqIsrCompletedInErrorState();
    }
  }

  DISABLE_ALL_INTERRUPTS;
  ENABLE_ABEL_INTERRUPTS_FAST;
  CLRTIMINGPIN_0
#ifdef TIME_INTERRUPT
  if (tmpRunning == 0) ABEL_ISR_DONE_1; // Ensure measuring entire time of nested interrupts!
#endif
  ISR_ENABLED_0;
}

/************************************************************************************
*************************************************************************************
* Public function(s)
*************************************************************************************
************************************************************************************/

/*******************************************************************
* Setup everything                                                 *
*******************************************************************/
void HwSetup(void)
{
  InitAllProjectsIO();
  InitAbelSPI();
  HWResetAbel();
#if defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
	// Call version in bootloader
	FL_ICG_Setup();
#else
	ICG_Setup(); // Reuse code in function to reduce code size
#endif defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
  AbelRegisterSetup();
  Dbg_Init(); // Setup serial port for debug if enabled.
}

// **************************************************************************

__interrupt void FLL_Lost_Lock_ISR(void)
{
	// Setup ICG module again to prevent that system hangs forever.
	
	ICGS1 |= 0x01; // Clear FLL lost lock interrupt

#if defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
	// Call version in bootloader
	FL_ICG_Setup();
#else
	ICG_Setup(); // Reuse code in function to reduce code size
#endif defined BOOTLOADER_ENABLED && !defined FOR_BOOTLOADER_ONLY
}

// **************************************************************************
