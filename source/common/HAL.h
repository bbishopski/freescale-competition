#ifndef __HAL_H

#include "common_def.h"

// 13192 prescale values for system clock. This is used
// to determine the resolution of the system clock.
// a resolution of 2MHZ means a timer tick every
// .5 uSeconds
#define MC13192_tmr_prescale_2MHZ     0 // .5uSeconds
#define MC13192_tmr_prescale_1MHZ     1 //  1uSeconds
#define MC13192_tmr_prescale_500khz   2 //  2uSeconds
#define MC13192_tmr_prescale_250khz   3 //  4uSeconds
#define MC13192_tmr_prescale_125khz   4 //  8uSeconds
#define MC13192_tmr_prescale_62khz    5 //  16uSeconds
#define MC13192_tmr_prescale_31khz    6 //  32uSeconds
#define MC13192_tmr_prescale_15khz    7 //  64uSeconds
#define MAX_TIME_VALUE  0xFFFFFF
#define TIME_PRESCALE   MC13192_tmr_prescale_250khz

// Fixed System clock 64 TPM clock source divide-by
// for 4MHz bus clock causes 62.5 ticks every ms
// (4MHz/64 = 62500; 1/62500 = .000016
#define MCU_TIMER_REGISTER_VAL  0x0E

// 62 ticks/ms at 4mhz bus clock with 64 divide-by
// actual value is 62.5
#define MCU_TIMER_TICKS_MS      62

// We don't use acoma mode because we need the timer
// on the 13192 for it's accuracy.  Acoma mode disables
// the timer.  Even when dozing, the timer will run!
#define NO_ACOMA_MODE              0
#define ACOMA_MODE                 1

// ******************************************
// CLOCK DEPENDANCIES and LOW POWER OPERATION
// ******************************************
// The 13192 clock is used as an external clock to wake
// up the MCU from stop mode.  When going to stop mode
// the 13192 clock is set to it's doze value which allows 
// for a greater range of wakeup times (from 31ms to 2 seconds).
// When not in stop mode the MCU uses the external
// 13192 clock with freq set to MC13192_clock_val_run.
//
// The HAL uses the 13192 external clock as throughout.
// The only time the MCU internal clock is used is when
// the 13192 clock is set to 16khz right before stop mode
// is entered.  Once the MCU wakes up, the external clock
// is cranked back up to it's run speed and the external
// clock is switched on as the MCU clock source.
//
// It is very important that the application does not try
// to switch clock sources outside of the HAL!
//
// The RTI via external clock is controlled by the SRTISC
// register.  The wake up time is controlled by RTIS2:RTIS1:RTIS0
// Those 3 bits determine the divisor factor for the external
// clock.  The factor is listed in this table:
//
//RTIS2:RTIS1:RTIS0 Using External Clock Source Delay (crystal frequency)
//0:0:0 Disable periodic wakeup timer Disable periodic wakeup timer
//0:0:1 divide by 256
//0:1:0 divide by 1024
//0:1:1 divide by 2048
//1:0:0 divide by 4096
//1:0:1 divide by 8192
//1:1:0 divide by 16384
//1:1:1 divide by 32768
//
// For example if external clock = 16khz and 0:1:0 is used then
// RTI will be generated every 62Msec (16 times a second).
//
// Therefore the range needed for timeouts determines which
// clock frequency to use.  For example a clock frequency of
// 16.393khz (lowest setting) gives a range of 15ms-2seconds.
// 16mhhz (highest setting) gives a range of 16us-2.048ms
//
// 16mhz external clock when not in stop mode
#define MC13192_clock_val_run    0  

// external clock when cpu is in stop mode
// 0 = 16MHz		 (16us - 2.048ms)
// 1 = 8MHz			 (32us - 4ms)
// 2 = 4MHz			 (64us - 8ms)
// 3 = 2MHz      (128us - 16.384msec)
// 4 = 1MHz			 (256us - 32ms)
// 5 = 62.5kHz	 (4.096ms - .5 second)
// 6 = 32.786kHz (8ms - 1 second)
// 7 = 16.393kHz (16ms - 2 second)
#define MC13192_clock_val_doze   5

// Value copied to SRTISC (Real Time Interrupt).  These values
// are intended to be used with RTI with external interrupt while
// the MCU is in stop mode.  The following values are dependent
// on the external clock frequency while in stop mode.  For this
// application the HAL uses an external clock frequency of 16khz
// which is supplied by the 13192 clock. That value was chosen
// because it offers a wide range of wakeup timings. 
#define RTI_EXT_DOZE_4_MSEC    0x31
#define RTI_EXT_DOZE_16_MSEC   0x32
#define RTI_EXT_DOZE_32_MSEC   0x33
#define RTI_EXT_DOZE_62_MSEC   0x34// close enough for this app
#define RTI_EXT_DOZE_64_MSEC   0x34
#define RTI_EXT_DOZE_128_MSEC  0x35
#define RTI_EXT_DOZE_256_MSEC  0x36
#define RTI_EXT_DOZE_512_MSEC  0x37

#define T_4_MS_SAMPLE_RATE     250 // 250 times/second

/*
#define MC13192_clock_val_doze   6

#define RTI_EXT_DOZE_8_MSEC   0x31
#define RTI_EXT_DOZE_32_MSEC  0x32
#define RTI_EXT_DOZE_62_MSEC  0x33
#define RTI_EXT_DOZE_125_MSEC 0x34
#define RTI_EXT_DOZE_250_MSEC 0x35
#define RTI_EXT_DOZE_500_MSEC 0x36
#define RTI_EXT_DOZE_1_SEC    0x37

#define T_8_MS_SAMPLE_RATE   12  //1/8
*/

/*
#define MC13192_clock_val_doze   7 
#define RTI_EXT_DOZE_16_MSEC  0x31
#define RTI_EXT_DOZE_62_MSEC  0x32
#define RTI_EXT_DOZE_125_MSEC 0x33
#define RTI_EXT_DOZE_250_MSEC 0x34
#define RTI_EXT_DOZE_500_MSEC 0x35
#define RTI_EXT_DOZE_1_SEC    0x36
#define RTI_EXT_DOZE_2_SEC    0x37

#define T_16_MS_SAMPLE_RATE   62  //1/16
*/

/*
#define MC13192_clock_val_doze   3 
#define RTI_EXT_DOZE_128_uSEC   0x31
#define RTI_EXT_DOZE_512_uSEC   0x32
#define RTI_EXT_DOZE_1_MSEC     0x33
#define RTI_EXT_DOZE_2_MSEC     0x34
#define RTI_EXT_DOZE_4_MSEC     0x35
#define RTI_EXT_DOZE_8_MSEC     0x36
#define RTI_EXT_DOZE_16_MSEC    0x37
*/
//
// These values are tied to the MC13192_clock_val_doze above
// and are dependent on a 16.393khz external clock
/*
#define MC13192_clock_val_doze   5 
#define RTI_EXT_DOZE_4_MSEC    0x31
#define RTI_EXT_DOZE_16_MSEC   0x32
#define RTI_EXT_DOZE_32_MSEC   0x33
#define RTI_EXT_DOZE_64_MSEC   0x34
#define RTI_EXT_DOZE_128_MSEC  0x35
#define RTI_EXT_DOZE_256_MSEC  0x36
#define RTI_EXT_DOZE_512_MSEC  0x37
*/

// The baud rate value depends on the RUN clock frequency
#define baud38400	0x0D	


typedef unsigned long t_time;

void HAL_getTicks(t_time *time);
void HAL_MCU_init(void);
void HAL_RF_init(void);
void HAL_RF_lowpower(void);
void HAL_MCU_sleep(UINT8 time_val, int deep);
void HAL_MCU_wake(void);
void HAL_RF_wake_wait(void);
BOOL HAL_KB_poll_s1(void);  // poll for s1 
BOOL HAL_KB_poll_s2(void);  // poll for s2
void HAL_KB_clear(void);    // clear kb flags
void MCU_delay (UINT16 delayMS);



#endif
