/****************************************************************************
* statemach.c
* 
* Author: 	Bill Bishop - Sixth Sensor
* Title: 	statemach.c
* 
* This is the state machine for the transmitter.  It is responsible for 
* handling system events based on the current state of the system. 
* Currently the following states are defined:
*
*
* Idle state  - initial state after power up, or no movement detected 
*               for extended period of time. RF is dozed, MCU is
*               in low power.  Only waking up to detect movement.
* Ready state - movement has been detected, RF is powered up in idle mode,
*               RF keepalive is taking place.
* Run state -   RF is in IDLE or TX mode most of the time and accelerometer
*               readings are being sent to the receiver on a periodic basis.
*               When a gesture off is detected, go back to ready state.
* 
* State transitions: Idle->Ready->Run->Ready->Idle
*
****************************************************************************/
#include "statemach.h"
#include "event.h"
#include "timer.h"
#include "accelerometer.h"
#include "HAL.h"
#include "sard_board.h"
#include "net.h"
#include "common_def.h"
#include <stdtypes.h>

// Do software filtering on accelerometer samples to remove noise.
// Every 8th sample is averaged to achieve filtering.
#define RUN_SAMPLE_FILTER_VALUE   8

// Shift 3 for fast divide by 8 - must match value above
#define RUN_SAMPLE_FILTER_SHIFT   3

// The gesture off acceleration depends on the sample rate
// so we'll define it in this module.  This is the acceleration
// of the x axis for the off gesture.
#define GESTURE_OFF_ACCELERATION  65

// Prototypes for state machine handlers
t_AppStates idleStateHandler(t_Event *);
t_AppStates readyStateHandler(t_Event *);
t_AppStates runStateHandler(t_Event *);

// Entry procs for each state
t_AppStates idleStateEnter(t_Event *pEvent);
t_AppStates readyStateEnter(t_Event *pEvent);
t_AppStates runStateEnter(t_Event *pEvent);

// State machine handlers
t_stMachFuncPtr stMachHandlers[MAX_STATES] = 
{ &idleStateHandler, 
  &readyStateHandler,
  &runStateHandler};

// Prototypes for doing work in this module  
void                lowPowerHandler(UINT8 nDozeValue, int nDozeMs, BOOL timerOn);
t_NetCallback       netCallback(t_NetData data);
extern volatile     t_CADB GlobalData;
void processKBEvent (t_Event *pEvent, int *handled);

// Packet used to send data to receiver
static t_NetPacket packet;

// Globals needed for run state
static int sampleNum,avgX,avgY,avgZ;
static BOOL gestureOffDetect;

/****************************************************************************
 * commonStateHandler
 *
 * Description: Handles events that are common to all states.
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     True if the event was handled, false if not. 
 ***************************************************************************/
static int commonStateHandler(t_Event *pEvent)
{
  int handled=0;

  switch (pEvent->eventId) {
  case ACK_RECEIVED:
    // Clear any alarms, we got an ack!
    alarmRFProblem(FALSE);

    // Stop the timer
    stopTimer(ACK_WAIT_TIMER);
    break;

  case KB_PRESS:
    // When keyboard debounce timer pops then we'll process
    // the keyboard press.
    //
    // Start the debounce timer
    startTimer(KB_DEBOUNCE_TIMER, FALSE);
    handled = 1;
    break;

  case KB_EVENT:
    // Debounce timer expired
    processKBEvent(pEvent, &handled);
    break;

  case TIMER_EXPIRED:
    {
      switch (pEvent->timerId) {
      case ACK_WAIT_TIMER:
        // Generate RF alarm
        alarmRFProblem(TRUE);
        break;

        // time to send another packet to receiver
      case KEEPALIVE_SEND_TIMER:
        // Always send a packet to the receiver regardless
        // of the alarm status. This allows the detector and
        // receiver to generate LED status if they are not
        // in range of each other - or if channel is not
        // configured properly.

        // clear ack received flag
        pEvent->pCADB->ackReceived = FALSE;

        if (!sendRFMessage(KEEPALIVE)) {
          alarmRFProblem(TRUE);
        } else {
          // Turn on receiver and set ack timer
          rcvRFData(netCallback);
          startTimer(ACK_WAIT_TIMER, FALSE);
        }
        break;
      }// timerid
      break;
    }// timer expired
    break;
  }

  return handled;  
}

/****************************************************************************
 * idleStateEnter
 *
 * Description: Setup for the idle state.
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     IDLE_STATE 
 ***************************************************************************/
t_AppStates idleStateEnter(t_Event *pEvent)
{
  // Initialize the movement system, not sampling too fast in this state
  // because it is not required, and we want to sleep as much as possible.
  movementInit(ACC_SAMPLES_PER_SECOND_SLOW);

  // No LEDs in idle state  
  runLed(FALSE);
  alarmRFProblem(FALSE);

  // Put RF Device into low power mode until we need it
  HAL_RF_lowpower();

  // The timer loop will get invoked approximately 62 times
  // per second in this state  
  setTimerBase(ACC_SAMPLE_FREQUENCY_SLOW);

  // Start polling the keyboard
  startTimer(KB_POLL_TIMER, TRUE);

  return IDLE_STATE;

}

/****************************************************************************
 * idleStateHandler
 *
 * Description: Idle state processor.
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     next state to enter, or IDLE_STATE if no change. 
 ***************************************************************************/
t_AppStates idleStateHandler(t_Event *pEvent)
{
  t_AppStates state=IDLE_STATE;

  if (commonStateHandler(pEvent)) {
    return state;
  }

  switch (pEvent->eventId) {
  case SYSTEM_INIT:
    state = idleStateEnter(pEvent);
    break;

    // Special event that should determine if other events
    // are to be processed by this state
  case IDLE_LOOP_WAIT:
    pEvent->eventId = NIL_EVENT;  

    // **********************************************************
    // MCU STOP MODE!!!!
    // **********************************************************

    // Let the MCU sleep to conserve battery life, TRUE means that
    // we don't want the lowPowerHandler to return until full timeslice
    // is up.  This will keep our timers in sync.
    lowPowerHandler(ACC_DOZE_FREQ_SLOW, ACC_SAMPLE_FREQUENCY_SLOW, TRUE);

    // **********************************************************
    // MCU back from STOP mode.  
    // **********************************************************

    // Each time through the loop take an accelerometer 
    // movement reading.  A movement sample requires quite
    // a bit of processing because of the math involved.
    if (movementSample()) {
      // Next time through the loop, process this event
      pEvent->eventId = MVMT_SAMPLE_READY;
    }
    break;

  case MVMT_SAMPLE_READY:
    // if movement detected transition to ready mode
    if (movementDetected()) {
      state = readyStateEnter(pEvent);
    }

    // reinitialize the movement system for next sample period
    movementInit(ACC_SAMPLES_PER_SECOND_SLOW);
    break;

  case NIL_EVENT:
    break;
  }
  return state;
}

/****************************************************************************
 * readyStateEnter
 *
 * Description: Ready state entry.  In this state the RF is powered up
 *              in IDLE mode, but MCU is still sleeping periodically.
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     READY_STATE
 ***************************************************************************/
t_AppStates readyStateEnter(t_Event *pEvent)
{
  // Initialize the movement system, crank it up to fast mode
  movementInit(ACC_SAMPLES_PER_SECOND_FAST);

  // Start looking for on gesture
  gestureInit();

  // Flash LED in ready mode
  runLed(FALSE);
  readyLedFlash();

  // The timer loop will get invoked approximately 256 times
  // per second in this state  
  setTimerBase(ACC_SAMPLE_FREQUENCY_FAST);

  // Start timer to keep monitoring movement.  We'll transition
  // back to idle if no movement detected  
  startTimer(IDLE_TIMER, FALSE);
  startTimer(KEEPALIVE_SEND_TIMER, TRUE);
  startTimer(READY_FLASH_TIMER, FALSE);
  startTimer(KB_POLL_TIMER, TRUE);

  // wake up the RF device, which will take ~ 323uSeconds for 13192. 
  // we do this because we want instant response once the gesture
  // to turn on wah pedal is recognized
  HAL_RF_wake_wait();

  // Make sure receiver knows the pedal is off
  sendRFMessage(WAH_OFF);
  return READY_STATE;
}

/****************************************************************************
 * readyStateHandler
 *
 * Description: Ready state processor.  
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     new state or READY_STATE if no change
 ***************************************************************************/
t_AppStates readyStateHandler(t_Event *pEvent)
{
  t_AppStates state=READY_STATE;

  if (commonStateHandler(pEvent)) {
    return state;
  }

  switch (pEvent->eventId) {
  case NIL_EVENT:
    break;

    // Special event that should determine if other events
    // are to be processed by this state
  case IDLE_LOOP_WAIT:
    pEvent->eventId = NIL_EVENT;  

    // We are spending all available processing power 
    // to sample at 256 times per second along with
    // all of the math involved.  Therefore, in this
    // mode there is no low power setting.


    // In this mode, we are monitoring for WAH-ON gesture.
    // If we get it, we enter RUN mode.  The WAH-ON gesture
    // requires movement samples which are math intensive. 
    // However, we are not monitoring for general movement
    // so we don't need an entire sample window, just look
    // at one sample to the next.      
    //
    // However, we also need to monitor for general movement
    // because we need to turn off automatically if no movement
    // occurs within window.
    if (movementSample()) {
      // Next time through the loop, process this event
      pEvent->eventId = MVMT_SAMPLE_READY;
    }

    // Determine if gesture ON has occured
    if (gestureOnDetected()) {
      state = runStateEnter(pEvent);
    }
    break;

  case TIMER_EXPIRED:
    {
      switch (pEvent->timerId) {
      // idle timer
      case IDLE_TIMER:
        // when this timer pops, the system has been
        // motionless for quite some time, transition
        // back to idle
        state = idleStateEnter(pEvent);
        break;

      case READY_FLASH_TIMER:
        readyLedFlash();
        startTimer(READY_FLASH_TIMER, FALSE);
        break;

      }// switch timer id

    }// timer expired event
    break;

  case MVMT_SAMPLE_READY:
    // if movement detected remain in this mode
    if (movementDetected()) {
      // restart idle timer
      stopTimer(IDLE_TIMER);
      startTimer(IDLE_TIMER, FALSE);
    }

    // reinitialize the movement system
    movementInit(ACC_SAMPLES_PER_SECOND_FAST);
    break;
  }// switch

  return state;
}

/****************************************************************************
 * runStateEnter
 *
 * Description: run state entry
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     RUN_STATE
 ***************************************************************************/
t_AppStates runStateEnter(t_Event *pEvent)
{
  // Set timer base to the sample frequency in this state
  setTimerBase(T_4_MS_SAMPLE_RATE);

  // Wait before checking for off gesture 
  startTimer(GESTURE_DEBOUNCE_TIMER, FALSE);

  // LED to notify user that run mode is entered
  runLed(TRUE);

  gestureOffDetect = FALSE;
  gestureInit();

  // Make sure receiver is off (from ack receive), we're going to start
  // transmitting at this point
  stopReceive();

  // We use a global here only to allow for easy debugging
  // mode where we send raw data to receiver
  strcpy(packet.idString, NET_ID_STRING);

  // Make sure receiver knows the pedal is off
  sendRFMessage(WAH_ON);
  sampleNum=avgX=avgY=avgZ=0;

  pEvent->eventId = NIL_EVENT;
  return RUN_STATE;
}

/****************************************************************************
 * runStateHandler
 *
 * Description: run state processor
 *
 * Parms:       pEvent - pointer to currently processing event.
 *
 * Returns:     new state or RUN_STATE if no change
 ***************************************************************************/
t_AppStates runStateHandler(t_Event *pEvent)
{
  int state=RUN_STATE;
  static tAccSample sampleX, sampleZ, sampleY;

  if (commonStateHandler(pEvent)) {
    return state;
  }

  switch (pEvent->eventId) {
  case NIL_EVENT:
    break;

  case IDLE_LOOP_WAIT:

    // Sample 250 times/second...
    // sleep MCU for 4 ms, then sample
    lowPowerHandler(RTI_EXT_DOZE_4_MSEC, 4, TRUE);

    // sample all axes
    sampleNum++;
    ACC_read_x(&sampleX);
    ACC_read_y(&sampleY);
    ACC_read_z(&sampleZ);

    // remove noise from the sampled data (software filtering)
    if (sampleNum >= RUN_SAMPLE_FILTER_VALUE) {
      avgX>>=RUN_SAMPLE_FILTER_SHIFT;
      avgY>>=RUN_SAMPLE_FILTER_SHIFT;
      avgZ>>=RUN_SAMPLE_FILTER_SHIFT;

      // Send accelerometer averages to receiver
      packet.msgType = WAH_MVMT;
      packet.netData[0] = avgX;
      packet.netData[1] = avgY;
      packet.netData[2] = avgZ;
      if (!sendRFPacket(&packet)) {
        alarmRFProblem(TRUE);
      } else {
        alarmRFProblem(FALSE);
      }

      // Use averaged X sample to detect off gesture
      if (gestureOffDetect) {
        if (gestureOffDetected(avgX, GESTURE_OFF_ACCELERATION)) {
          state = readyStateEnter(pEvent);
        }
      }// if ok to look for off gesture

      // Reset averages        
      sampleNum=avgX=avgY=avgZ=0;
    }// if avg sample ready
    else {
      // keep running sum of sample values until ready to avg
      avgX+=sampleX;
      avgY+=sampleY;
      avgZ+=sampleZ;
    }

    pEvent->eventId = NIL_EVENT;      
    break;

  case TIMER_EXPIRED:
    {
      switch (pEvent->timerId) {
      // idle timer
      case GESTURE_DEBOUNCE_TIMER:
        // start checking for gesture off now, if detected
        // it will take us out of this state
        gestureOffDetect = TRUE;
        break;
      }
    }
    break;
  }
  return state;
}

/****************************************************************************
 * processKBEvent
 *
 * Description: handles kb press
 *
 * Parms:       pEvent - pointer to currently processing event, handled.
 *
 * Returns:     nothing
 ***************************************************************************/
void processKBEvent(t_Event *pEvent, int *handled)
{
  if (pEvent->pCADB->s1Pressed) {
    // selects new RF channel.
    selectNextRFChannel();
    *handled=1;

    // don't come back here again until user presses again
    pEvent->pCADB->s1Pressed = FALSE;
  }
}

/****************************************************************************
 * netCallback
 *
 * Description: callback procedure that gets invoked when a packet arrives
 *
 * Parms:       *data - pointer to received packet
 *
 * Returns:     nothing
 ***************************************************************************/
static t_NetCallback netCallback(t_NetPacket *data)
{
  // We got an acknowledgement from receiver, set global
  // flag
  if (data->msgType == WAH_ACK) {
    GlobalData.ackReceived = TRUE;
  }
}


/****************************************************************************
 * lowPowerHandler
 *
 * Description: The low power handler for the state machine.  Puts the 
 *              MCU into STOP3 mode.  If timerOn=TRUE then this procedure
 *              will not return until the alloted time has elapsed.  This
 *              allows for the timer to continue to run on schedule without
 *              having a separate interrupt for the timer.
 *
 * Parms:       nDozeValue - HAL defined doze value
 *              nDozeMs    - number of MS to doze
 *              timerOn    - if true, delays if STOP exited prematurely
 *
 * Returns:     nothing
 ***************************************************************************/
void lowPowerHandler(UINT8 nDozeValue, int nDozeMs, BOOL timerOn)
{
  static t_time startTime, endTime, asleepMs;
  static UINT16 delayMs;

  if (timerOn) {
    // Get current system time (timer ticks)
    HAL_getTicks(&startTime);
  }

  // put MCU into stop mode 
  HAL_MCU_sleep(nDozeValue, FALSE);

  if (timerOn) {
    // Get current system time (timer ticks)
    // and determine how long we were in low power sleep
    // mode.  Eat up any leftover time so that the timer
    // loop is called at an accurate rate
    HAL_getTicks(&endTime);
    HAL_difftime(startTime, endTime, &asleepMs);

    // Eat up any leftover time, the low power wait may
    // have been interrupted.  By delaying here we keep
    // the timer loop consistent.    
    if (asleepMs < nDozeMs) {
      delayMs = nDozeMs - asleepMs;
      MCU_delay(delayMs);
    }
  }// if timerOn
}
