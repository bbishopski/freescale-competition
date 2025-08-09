/****************************************************************************
* HAL.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	HAL.c
* 
* This module abstracts some of the hardware specifics from the application.
* Mainly in the area of timers, peripherals, RF components and low power modes.
*
****************************************************************************/
#include <hidef.h> /* for EnableInterrupts macro */
#include <MC9S08GT60.h> /* include peripheral declarations */
#include "simple_mac.h"
#include "mcu_hw_config.h"
#include "MC13192_hw_config.h"
#include "SCI.h"
#include "drivers.h"
#include "sard_board.h"
#include "HAL.h"

// Used to convert system ticks in microseconds to milliseconds.
// This table is indexed by the 13192 prescale value.
__uint32__ prescaleMSDivisor[] = {2000,1000,500,250,125,62,31,15};

static void waitForRTIextClk(UINT8, int deep);

// Keyboard press flags
volatile static int s101_pressed=0;
volatile static int s102_pressed=0;

/****************************************************************************
* HAL_MCU_init
*
* Description: Application calls to initialize the MCU.
*
* Parms:       none
*
* Returns:     none
***************************************************************************/
void HAL_MCU_init(void)
{
#ifndef SIM_MODE

  // Initialize the mcu, setup GPIOs, SPI, clock scheme, etc.
  //mcu_init();

  // Set 13192 clock rate.  This clock will drive the MCU
  // as an external clock 
  MLME_set_MC13192_clock_rate(MC13192_clock_val_run); 

  // use external clock - for SARD board it's supplied by the
  // 13192 via CLKo
  use_external_clock();

  // Initialize push buttons, etc. for demo mode on 13192 board  
#ifdef MC13192SARD  
  sard_board_init();
#endif

  // Setup timer register, used for delay
  TPM1SC = MCU_TIMER_REGISTER_VAL; 

  // Initialize SCI communications
  SCIInit(baud38400); 
#endif

  EnableInterrupts;

  // Make sure channel and power levels are initialized  
  setRFChannel();

  // Turn off the watchdog	
  SOPT = 0x73;  
}

/****************************************************************************
* HAL_RF_init
*
* Description: Application calls to initialize the RF components.
*
* Parms:       none
*
* Returns:     none
***************************************************************************/
void HAL_RF_init(void)
{
#ifndef SIM_MODE

  // Initialize the mcu, setup GPIOs, SPI, clock scheme, etc.
  mcu_init();     

  // Initialize 13192
  MC13192_init();

  // Make sure channel and power levels are initialized  
  setRFChannel();

  // Reset the system clock, the 13192 is used for system clock
  // because of precise accuracy
  PLME_set_time_request(0);

  // The 13192 time base will tick every 4uSeconds
  MLME_set_MC13192_tmr_prescale(TIME_PRESCALE);

  MC13192_IRQ_IE_BIT = 1;

#endif
}

/****************************************************************************
* HAL_RF_lowpower
*
* Description: Implementation of low power handler.
*
* Parms:       none
*
* Returns:     none
***************************************************************************/
void HAL_RF_lowpower(void)
{
#ifndef SIM_MODE

  // If 13192 Register 7, bit9=1 when entering
  // Doze mode, it will keep generating clock
  // if clock freq <= 1mhz
#ifndef MVMT_DEBUG   

  // don't doze if debugging, we need to leave the 13192 on
  // for sending debug information to receiver

  // Doze the 13192, but leave CLKo running for an external
  // clock to the MCU.
  MLME_doze_request_wClk(NO_ACOMA_MODE);  
#endif// MVMT_DEBUG

#endif
}

/****************************************************************************
* HAL_RF_wake_wait
*
* Description: Call to wake up the RF components - will wait until completion.
*
* Parms:       none
*
* Returns:     none
***************************************************************************/
void HAL_RF_wake_wait(void)
{
#ifndef SIM_MODE

  // wake up 13192, you sleepy head...
  MLME_wake_request();

  // bring 13192 into idle mode
  MC13192_init();

  // Make sure we're on proper RF channel
  setRFChannel();

  // Set 13192 (external) clock to run speed
  MLME_set_MC13192_clock_rate(MC13192_clock_val_run); 

  // ok, now we can use the external clock source (13192)
  use_external_clock();

  // turn off rti	
  SRTISC = 0;

#endif
}


/****************************************************************************
* HAL_MCU_wake
*
* Description: Should be called after MCU low power mode is exited.
*
* Parms:       none
*
* Returns:     none
***************************************************************************/
void HAL_MCU_wake(void)
{
#ifndef SIM_MODE

  // Set 13192 (external) clock to run speed
  MLME_set_MC13192_clock_rate(MC13192_clock_val_run); 

  // Now we can use the external clock source (13192) since
  // it is back up to full speed
  use_external_clock();

#endif
}


/****************************************************************************
* HAL_difftime
*
* Description:  Pass start ticks, end ticks, will return difference in MS.
*
* Note: using a pointer rather than return value because I've learned
* my lesson about passing 32 bit values on the stack with this compiler!
*
* Parms:       start - start ticks
*              end   - end ticks
*
* Returns:     diffMs
***************************************************************************/
void HAL_difftime(__uint32__ start, __uint32__ end, __uint32__ *diffMs)
{
  static __uint32__ divisor;
  static __uint32__ diffTicks;

  // get MS divisor for conversion from timer ticks to MS  
  divisor = prescaleMSDivisor[TIME_PRESCALE];

  // Calculate time asleep in MS (don't worry about timer wrap)
  // the math works itself out (as long as the variables are unsigned).
  diffTicks = end - start;
  diffTicks &= 0x00FFFFFF;  
  *diffMs = diffTicks / divisor;
}

/****************************************************************************
* HAL_getTicks
*
* Description:  Gets number of ticks from 13192 system clock
*
* Parms:       time - number of ticks returned
*
* Returns:     time
***************************************************************************/
void HAL_getTicks(t_time *time)
{
  // Get current system ticks	
  PLME_get_time_request(time);
}

/****************************************************************************
* drv_read_tmr_1
*
* Description:  reads internal (MCU) clock tmr
*
* Parms:       none
*
* Returns:     number of ticks
***************************************************************************/
static UINT16 drv_read_tmr_1()
{
  // w[0] is MSB, w[1] is LSB
  UINT16  w; 

  ((UINT8*)&w)[0] = TPM1CNTH; /* MSB */
  ((UINT8*)&w)[1] = TPM1CNTL; /* LSB */
  return w;
}

/****************************************************************************
* MCU_delay
*
* Description:  MCU_delay.  Uses the MCU clock which is not as
*               accurate as the 13192 clock, but I didn't want
*               to read from SPI in a dead loop.
*
* Parms:       delayMS - number of MS to delay
*
* Returns:     nothing
***************************************************************************/
void MCU_delay (UINT16 delayMS)
{
  UINT16 MCU_oldtime;
  UINT16 MCU_newtime;
  UINT16 MCU_delayticks;

  // get number of timer ticks/ms  
  MCU_delayticks = delayMS * MCU_TIMER_TICKS_MS;

  MCU_oldtime = drv_read_tmr_1 ();
  MCU_newtime = MCU_oldtime;
  while ((MCU_newtime-MCU_oldtime) < MCU_delayticks) {
    MCU_newtime = drv_read_tmr_1 ();
  }
}

/****************************************************************************
* MCU_delay
*
* Description:  puts MCU into stop mode with RTI wakeup at given interval
*
* Parms:       delayMS - number of MS to delay
*
* Returns:     nothing
***************************************************************************/
void HAL_MCU_sleep(UINT8 time_val, int deep)
{
#ifndef SIM_MODE

  // Do not remove this!  This is the only time that the
  // MCU clock is needed because the external clock freq
  // is getting ready to go REALLY LOW!
  use_mcu_clock(); 

  // set 13192 clock speed.  clock will continue to run while dozing
  // regardless of whether 13192 is dozing, we want to set the clock
  // rate to the slow rate so that the MCU is woken up in the appropriate
  // time.
  MLME_set_MC13192_clock_rate(MC13192_clock_val_doze);

  // wait for RTI with external clock
  waitForRTIextClk(time_val, deep);
#endif
}

/****************************************************************************
* KEYBOARD ROUTINES
***************************************************************************/
void HAL_KB_clear(void)
{
  s101_pressed=s102_pressed=0;
}

BOOL HAL_KB_poll_s1(void) 
{
  return s101_pressed;
}
BOOL HAL_KB_poll_s2(void) 
{
  return s102_pressed;
}

interrupt void KBD_ISR()
{
  if (PTAD_PTAD2 == 0)
    s101_pressed = 1;

  // don't support both pressed at same time!  S1 overrides
  else if (PTAD_PTAD3 == 0)
    s102_pressed = 1;

  // ack the interrupt (new interrupts are disabled until ack)
  KBI1SC_KBACK = 1;
}

interrupt void RTI_ISR() 
{
  // acknowledge the RTI
  SRTISC_RTIACK = 1;
}

/****************************************************************************
* waitForRTIextClk
*
* Description:  puts MCU into stop mode and sets RTI to wakeup at specified
*               amount of time.  For simplicity STOP3 is used because the
*               application can pick up where it left off - no special
*               initialization code is needed when exiting.
*
* Parms:       SRTISC_val - will be written to SRTISC register - controls
*              the amount of time in stop mode.
*              deep - does nothing.  In future could be used to select
*              stop1, stop2 or stop3
*
* Returns:     nothing
***************************************************************************/
static void waitForRTIextClk(UINT8 SRTISC_val, int deep)
{
  // RTI control register
  SRTISC = SRTISC_val;

  // Enable keyboard interrupts, only S101,S102 will wake us up
  // on SARD board S101 is mapped to KBI2 (PTA2)
  // S102 is mapped to KBI3 (PTA3)
  KBI1PE = KBI1PE_KBI1PE2_MASK | KBI1PE_KBI1PE3_MASK;  
  KBI1SC_KBACK = TRUE;    // clear pending keyboard interrupts
  PTAPE_PTAPE2 = TRUE;    // enable Pullup for Keyboard pin (S101)
  PTAPE_PTAPE3 = TRUE;    // enable Pullup for Keyboard pin (S102)


  IRQSC_IRQPE = TRUE;     // pull-up and enable IRQ

  // Disable keyboard interrupts then re-enable after setting
  // registers. Otherwise we may get a false keyboard interrupt
  // when setting these registers.
  KBI1SC_KBI1E = FALSE;       

  // Make All unused I/O Outputs Driving low   
  PTADD = PTADD_PTADD_MASK;   // set all PTA to output (driving low)
  PTADD_PTADD2 = FALSE;       // PTA2 is input (S101)
  PTADD_PTADD3 = FALSE;       // PTA3 is input (S102)

  PTBDD = PTBDD_PTBDD_MASK;   // set all PTB to output
  PTCDD = PTCDD_PTCDD_MASK;   // set all PTC to output
  PTDDD = PTDDD_PTDDD_MASK;   // set all PTD to output
  PTEDD = PTEDD_PTEDD_MASK;   // set all PTE to output
  PTGDD = PTGDD_PTGDD_MASK;   // set all PTG to output
  KBI1SC_KBI1E = TRUE;        // enable keyboard interrupts

  // Disable LVD while in stop
  // HINT:
  //                 %00010100 ;Disable LVD in stop 
  //                  ||||||       
  //                  ||||||
  //                  ||||||
  //                  |||||+---LVDE Enable LVD 
  //                  ||||+----LVDSE Disable LVD in stop 
  //                  |||+-----LVDRE Enable LVD reset protection
  //                  ||+------LVDIE
  //                  |+-------LVDACK
  //                  +--------LVDF  
  // 
  // set LVDE and LVDRE only (LVDSE disabled)
  SPMSC1 = SPMSC1_LVDE_MASK | SPMSC1_LVDRE_MASK;

  // Disable COP and enable STOP and BDM
  // HINT:              %01100010 ;COP and STOP enable controls
  //                     |||   |
  //                     |||   |
  //                     |||   +--BKGDPE -- BKGD pin enabled
  //                     ||+------STOPE --- STOP allowed
  //                     |+-------COPT ---- long timeout 2^18
  //                     +--------COPE ---- COP off
  SOPT = /*SOPT_BKGDPE_MASK | */SOPT_STOPE_MASK | SOPT_COPT_MASK;

  // Enter STOP3
  SPMSC2_PDC = FALSE;
  // SPMSC2_PPDC - don't care for stop3

  // good night, zzzzzzzzzzzz...
  _asm stop;

  // turn off rti	
  SRTISC = 0;
}
