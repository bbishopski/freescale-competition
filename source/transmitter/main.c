/****************************************************************************
* main.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	main.c
* 
* This is the main file for the Remote Control Wah Pedal Transmitter.
* The transmitter is strapped to the foot of the musician and transmits
* accelerometer readings to the receiver which controls the wah pedal.
*
* This file contains the main loop and state machine/event driver.  It
* also contains the timeout functions.
*
****************************************************************************/
#include <hidef.h> 
#include <MC9S08GT60.h> 
#include "simple_mac.h"
#include "mcu_hw_config.h"
#include "MC13192_hw_config.h"
#include "SCI.h"
#include "drivers.h"
#include "sard_board.h"
#include "HAL.h"
#include "accelerometer.h"
#include "net.h"
#include "timer.h"
#include "statemach.h"

// Global data used by all applications
// Cross-application data block
volatile t_CADB GlobalData;

void main(void) 
{
  t_AppStates appState=IDLE_STATE;

  // Create event object
  t_Event event;
  memset(&GlobalData, '\0', sizeof(GlobalData));
  event.pCADB = &GlobalData;

  // Init RF and MCU hardware
  HAL_RF_init();
  HAL_MCU_init();

  // Initialize accelerometers and movement sensor system 
  ACC_init();
  ACC_MovementInit();

  // Initialize state machine  
  event.eventId = SYSTEM_INIT;    
  appState = stMachHandlers[IDLE_STATE](&event);

  for (;;) {
    // *******************************************************
    // Low power handling    
    // To achieve lowest power setting we don't want to
    // continuously process, so we'll create a sleep event
    // each time through the loop.  The state machine can
    // determine how to handle low power mode because it
    // may be state specific.
    // *******************************************************

    // This event must be handled by each state and must return
    // an event to process - or NIL_EVENT if nothing to process
    event.eventId = IDLE_LOOP_WAIT;    
    appState = stMachHandlers[appState](&event);
    // End Low Power handling

    // ********************************************************
    // Increment timers, don't process timeouts if movement
    // sample is ready, just increment timers.  We'll get back
    // to expired timers after checking for movement.    
    // ********************************************************
    handleLoopTimer(&event, event.eventId != MVMT_SAMPLE_READY);  

    // ********************************************************
    // Send the event to the state machine
    // ********************************************************
    appState = stMachHandlers[appState](&event);
  }
}

/**************************************************************
 * TIMER PROCEDURES
 *
 * The timer procedures are responsible for setting the 
 * appropriate event to be processed.  In general a TIMER_EXPIRED
 * message with timerId set to the specific timer will be set
 * in the event data. The application is then responsible for
 * determining if a timeout occured and how to handle the timeout.
 * The applicaiton is also responsible for restarting timers if
 * needed.
 *
 **************************************************************/

/*********************************************************
 * Called when READY_FLASH_TIMER expires.  This allows the
 * ready mode LED to flash.
 *********************************************************/
int readyFlashTimer(t_Event *event)
{
  event->eventId=TIMER_EXPIRED;
  event->timerId=READY_FLASH_TIMER;
  return 0;
}

/*********************************************************
 * Called when ACK_WAIT_TIMER timer expires.  When this
 * timer expires the receiver did not respond to keep
 * alive message in time. 
 *********************************************************/
int ackWaitTimer(t_Event *event)
{
  // ack wait timer popped, poll the status of the ack
  if (event->pCADB->ackReceived) {
    event->eventId=ACK_RECEIVED;
    event->pCADB->ackReceived=FALSE;
  } else {
    event->eventId=TIMER_EXPIRED;
    event->timerId=ACK_WAIT_TIMER;
  }
  return 0;
}

/*********************************************************
 * Called when KEEPALIVE_SEND_TIMER expires.  When this 
 * timer expires it's time to send the keepalive message
 * to the receiver.
 *********************************************************/
int keepAliveTimer(t_Event *event)
{
  event->eventId=TIMER_EXPIRED;
  event->timerId=KEEPALIVE_SEND_TIMER;
  return 0;
}

/*********************************************************
 * Called when IDLE_TIMER expires.  When this 
 * timer expires the system has been motionless for a while
 * so transition to idle mode.
 *********************************************************/
int idleTimer(t_Event *event)
{
  event->eventId=TIMER_EXPIRED;
  event->timerId=IDLE_TIMER;
  return 0;
}

/*********************************************************
 * Called when debounce timer expires.
 *********************************************************/
int kbDebounceTimer(t_Event *event)
{
  HAL_KB_clear();
  event->eventId=KB_EVENT;
  return 0;
}

/*********************************************************
 * Called when GESTURE_DEBOUNCE_TIMER expires.  When this 
 * timer expires it's time to start looking for gestures
 * via accelerometer.  The gestures turn the system on/off.
 * The debounce timer is used to allow the accelerometers
 * to settle down after a jolt is detected before looking
 * for gestures.
 *********************************************************/
int gestureDebounceTimer(t_Event *event)
{
  event->eventId=TIMER_EXPIRED;
  event->timerId=GESTURE_DEBOUNCE_TIMER;
  return 0;
}

/*********************************************************
 * Called when kb poll timer expires.  Polls for kb press.
 *********************************************************/
int kbPollTimer(t_Event *event) 
{
  event->eventId=NIL_EVENT;

  if (HAL_KB_poll_s1()) {
    event->eventId=KB_PRESS;
    event->pCADB->s1Pressed = TRUE;  
  }

  if (HAL_KB_poll_s2()) {
    event->eventId=KB_PRESS;
    event->pCADB->s2Pressed = TRUE;  
  }
  return 0;
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

/*********************************************************
 * Flashes the ready LED
 *********************************************************/
void readyLedFlash()
{
  LED2^=1;
}
