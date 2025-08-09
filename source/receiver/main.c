/****************************************************************************
* main.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	main.c
* 
* This is the main file for the Remote Control Wah Pedal Receiver.
* The receiver controls the wah pedal hardware based on values 
* given from transmitter.
*
* The RF channel is constantly monitored.  When a keepalive packet is
* received we send an ack.  When a movement packet is received it is
* translated into a wah pedal setting.  The translation is just a
* trig function to calculate the angle of the pedal based on accelerometer
* values.
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
#include "net.h"
#include "statemach.h"
#include "wahPedal.h"
#include "trigtables.h"

// Number of packets to toss while waiting for
// accelerometer readings to settle down after the
// user turns on the device
#define NUM_TOSS_PACKETS  2

void alarmRFProblem(BOOL alarm);
void runLed(BOOL alarm);
void runLedFlash();
int getWahStep(UINT8 wahStep, const t_NetData accReading, const t_NetData prevAcc);
int fixedArcTangent2(long tanRad1, long tanRad2);

// Prototypes
static t_NetCallback netCallback(t_NetPacket *packet);
static volatile t_AppStates appState=IDLE_STATE;
static volatile int         wahAngle=0;

// If you don't declare some global memory, this whole thing
// doesn't work!  Don't believe me?  Take this out and see what
// happens.  Guess how long it took for me to figure that out?
// I'd love to get in a room with the team who wrote this compiler!
static char dbgbuf[80];
static int  bufcnt;

void main(void) 
{
  // Wah pedal setting
  UINT8 wahStep=0;
  int   topSet =0;
  int   nToss  =0;

  // Init RF and MCU hardware
  HAL_RF_init();
  HAL_MCU_init();

#ifdef MVMT_DEBUG	    

  // if debugging, send data to serial port, in debug
  // mode the PC is used and not a modem
  SCITransmitStr("DEBUG: \r\n");
#endif

  // Delay for a few ms while wah hardware is stabilizing
  MCU_delay(100);
  initWahPedal(ARCTANGENT_MIN_ANGLE, ARCTANGENT_MAX_ANGLE);

  // State machine loop.
  for (;;) {
    //  
    // constantly monitor RF data
    //
    // if keep alive is received, send ack
    //
    // otherwise the RF data is an accelerometer reading
    //
    rcvRFData(netCallback);
    _asm wait;

    switch (appState) {
    // Transmitter sent a keepalive packet
    case KEEPALIVE_STATE:
      // send acknowledgement
      if (!sendRFMessage(WAH_ACK)) {
        alarmRFProblem(TRUE);
      } else {
        alarmRFProblem(FALSE);
      }
      appState=IDLE_STATE;
      break;

    case WAH_OFF_STATE:
      wahStep = WAH_POT_POWERONVALUE;
      setWahPedal(wahStep);
      runLed(FALSE);
      appState = IDLE_STATE;
      break;

    case WAH_ON_STATE:
      // so that we're in sync with wah hardware when
      // user turns on with gesture
      wahStep = WAH_POT_POWERONVALUE;
      setWahPedal(wahStep);
      runLed(TRUE);
      appState = IDLE_STATE;
      topSet   = 0;
      wahAngle = 0;
      nToss    = 0;
      break;

    case WAH_MOVE_STATE:
      if (nToss > NUM_TOSS_PACKETS) {
        // call on first movement after wah is turned on
        if (!topSet && wahAngle > 0) {
          // don't set top until we have a sane value
          setWahTop(wahAngle);
          topSet = 1;
        }

        if (wahAngle > 0) {
          setWahPedalAngle(wahAngle);
        }

#ifdef MVMT_DEBUG
        itoa(wahAngle, dbgbuf, 20);  
        SCITransmitStr(dbgbuf);
        SCITransmitStr("\r\n");
#endif
      } else {
        nToss++;
      }

      break;
    }// switch

  }
}

static t_NetCallback netCallback(t_NetPacket *packet)
{
  long y,z;

#ifdef MVMT_DEBUG

  // if debugging, send data to serial port

  // display all data up to the debug data  
  SCITransmitArray((char *)packet, sizeof(t_NetPacket) - MAX_NET_DATA);

  // display additional debug data
  for (bufcnt=0;bufcnt<3;bufcnt++) {
    itoa(packet->netData[bufcnt], dbgbuf, 20);  
    SCITransmitStr(dbgbuf);
    SCITransmitStr(",");
  }
  SCITransmitStr("\r\n");
#endif

  switch (packet->msgType) {
  case KEEPALIVE:
    appState = KEEPALIVE_STATE;
    break;

  case WAH_ON:
    appState = WAH_ON_STATE;
    break;

  case WAH_OFF:
    appState = WAH_OFF_STATE;
    break;

  case WAH_MVMT:
    appState = WAH_MOVE_STATE;

    // Calculate angle of wah pedal from accelerometer reading
    y = (long)packet->netData[1];
    z = (long)packet->netData[2];
    wahAngle = fixedArcTangent2(y, z);
    break;

  default:
    break;
  }
}


// Returns atan of parm1/parm2.  Returns angle in
// fixed point degress to a precision of tenth of degree
int fixedArcTangent2(long tanRad1, long tanRad2)
{
  static long tanRadians;
  int tanAngle = 0;

  tanRad1 *= ARCTANGENT_PRECISION;
  //tanRad2 *= ARCTANGENT_PRECISION;

  if (tanRad2>0) {
    tanRadians = tanRad1/tanRad2;
    tanRadians -= ARCTANGENT_NORMALIZE;
    if (tanRadians >= 0 && tanRadians < ARCTANGENT_NUMENTRIES) {
      tanAngle = fixedArcTangentTable[tanRadians];
    }
  }

  return tanAngle;
}


/*********************************************************
 * Turns on/off the RF Problem LED
 *********************************************************/
void alarmRFProblem(BOOL on_off)
{
  LED1 = on_off == TRUE ? LED_ON : LED_OFF;
}

/*********************************************************
 * Turns on/off the RUN Mode LED
 *********************************************************/
void runLed(BOOL on_off)
{
  LED2 = on_off == TRUE ? LED_ON : LED_OFF;
}




