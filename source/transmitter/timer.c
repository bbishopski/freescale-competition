/****************************************************************************
* timer.c
* 
* Author: 	Bill Bishop - Sixth Sensor
* Title: 	timer.c
* 
* Simple timer system provides for inaccurate but simple timing functions.
* The handleLoopTimer is to be called from within a loop at regular 
* intervals.  The interval is specified in the setTimerBase() function.
* The application can change the loop timing at run time, but it must
* call setTimerBase() and must restart any active timers.
*
* When a timeout occurs the timeout function will be called.  The application
* is responsible for all timeout handling. 
*
****************************************************************************/
#include "timer.h"
#include <stdtypes.h>

// Timer handler prototypes
extern int ackWaitTimer(t_Event *);
extern int keepAliveTimer(t_Event *);
extern int idleTimer(t_Event *);
extern int kbDebounceTimer(t_Event *);
extern int kbPollTimer(t_Event *);
extern int readyFlashTimer(t_Event *);
extern int gestureDebounceTimer(t_Event *);

// For fast timer stops, no sanity checks
static void stopTimerFast(t_TimerId timerId);

// For optimization, not a counter - just a flag
static int timersRunning=0;

// The order here decides priority.  Care must be taken when defining
// timeout values.  If two timers expire at the same time then only
// one is handled.  The next call to handleLoopTimer() will then
// handle the second timer.  So care must be taken so that a quick
// timer doesn't hog up all the ticks!
t_TmrHandlers tmrHandlers[] = 
{ACK_WAIT_TIMEOUT_MSEC,       0, NIL_TIME, FALSE, &ackWaitTimer,
  KEEPALIVE_SEND_TIMEOUT_MSEC, 0, NIL_TIME, FALSE, &keepAliveTimer,
  IDLE_TIMEOUT_MSEC,           0, NIL_TIME, FALSE, &idleTimer,
  KB_DEBOUNCE_TIMEOUT_MSEC,    0, NIL_TIME, FALSE, &kbDebounceTimer,
  KB_POLL_TIMEOUT_MSEC,        0, NIL_TIME, FALSE, &kbPollTimer,
  READY_FLASH_TIMEOUT_MSEC,    0, NIL_TIME, FALSE, &readyFlashTimer,
  GESTURE_DEBOUNCE_TIMEOUT_MSEC,0, NIL_TIME, FALSE, &gestureDebounceTimer,
  0, 0, NIL_TIME, FALSE, 0
};

/****************************************************************************
 * setTimerBase
 *
 * Description: Sets the number of times per second that the timer loop
 *              will be invoked.  NOTE: all timers will be stopped. You 
 *              must restart timers when you change the timer base.
 *
 * Parms:       ticksPerSecond - number of times the loop timer will be
 *              called per second.
 *
 * Returns:     nothing 
 ***************************************************************************/
void setTimerBase(int ticksPerSecond)
{
  int cnt = 0;

  if (ticksPerSecond > 0) {
    // first stop all timers!  
    stopAllTimers();

    // set the timeoutCount (number of ticks before timeout)
    // for each timer
    for (cnt=0; cnt<MAX_TIMERS; cnt++) {
      tmrHandlers[cnt].timeoutCount = tmrHandlers[cnt].timeoutMs / ticksPerSecond;
    }
  }
}

/****************************************************************************
 * stopAllTimers
 *
 * Description: Stops all active timers
 *
 * Parms:       none
 *
 * Returns:     nothing 
 ***************************************************************************/
void stopAllTimers(void)
{
  int cnt;

  timersRunning=0;
  for (cnt=0; cnt<MAX_TIMERS; cnt++) {
    stopTimerFast(cnt);
  }
}

/****************************************************************************
 * startTimer
 *
 * Description: Starts a given timer
 *
 * Parms:       timerId - which timer to start
 *              restartOnTimeout - whether or not to automatically restart 
 *
 * Returns:     nothing 
 ***************************************************************************/
void startTimer(t_TimerId timerId, BOOL restartOnTimeout)
{
  timersRunning=1;

  // sanity check
  if (timerId > MAX_TIMERS || timerId < 0) {
    return;
  }

  // easy, just start the tick count.  It's up to the application
  // to call handleLoopTimer().  
  tmrHandlers[timerId].tickCount = 0;
  tmrHandlers[timerId].restart = restartOnTimeout;
}

/****************************************************************************
 * stopTimer
 *
 * Description: stop a given timer
 *
 * Parms:       timerId - which timer to stop
 *
 * Returns:     nothing 
 ***************************************************************************/
void stopTimer(t_TimerId timerId)
{
  // sanity check
  if (timerId > MAX_TIMERS || timerId < 0) {
    return;
  }

  // easy, just start the tick count.  It's up to the application
  // to call handleLoopTimer().  
  tmrHandlers[timerId].tickCount = NIL_TIME;
}

/****************************************************************************
 * stopTimerFast
 *
 * Description: stop a given timer (Fast Version)
 *
 * Parms:       timerId - which timer to stop
 *
 * Returns:     nothing 
 ***************************************************************************/
static void stopTimerFast(t_TimerId timerId)
{
  // easy, just start the tick count.  It's up to the application
  // to call handleLoopTimer().  
  tmrHandlers[timerId].tickCount = NIL_TIME;
}

/****************************************************************************
 * tmrStarted
 *
 * Description: determines if timer has been started
 *
 * Parms:       timerId - which timer to check
 *
 * Returns:     True if tmr started, false if not 
 ***************************************************************************/
static int tmrStarted(int tmrIdx)
{
  return(tmrHandlers[tmrIdx].tickCount != NIL_TIME);
}

/****************************************************************************
 * tmrExpired
 *
 * Description: determines if timer is expired
 *
 * Parms:       timerId - which timer to check
 *
 * Returns:     True if tmr expired, false if not 
 ***************************************************************************/
static int tmrExpired(int tmrIdx)
{
  //int loopTimer = LOOP_TIMER;
  int expired = 0;

  // If timer is just now expiring or has expired in the past
  expired = (tmrHandlers[tmrIdx].tickCount >= tmrHandlers[tmrIdx].timeoutCount);

  // only restart timer if restart==true  
  if (expired) {
    if (tmrHandlers[tmrIdx].restart)
      tmrHandlers[tmrIdx].tickCount = 0;
    else
      tmrHandlers[tmrIdx].tickCount = NIL_TIME;
  }
  return(expired);
}

/****************************************************************************
 * incrTimers
 *
 * Description: Increments all started timers
 *
 * Parms:       none
 *
 * Returns:     nothing
 ***************************************************************************/
static void incrTimers()
{
  int cnt;

  for (cnt=0; cnt<MAX_TIMERS; cnt++) {
    if (tmrStarted(cnt)) {
      tmrHandlers[cnt].tickCount++;
    }
  }
}

/****************************************************************************
 * incrTimers
 *
 * Description: Calls appropriate timer handler based on counter.
 *              Counter is assumed to be in increments of LOOP_TIMER.
 *              Updates event only if process is TRUE
 *              otherwise just keeps up with timers
 *              automatically stops timer when expired
 *
 * Parms:       event   - pointer to system event.  Will be updated if expiry
 *              process - if true, then tmr expiry is checked.  If false
 *                        then just the tick is recorded - no timeouts occur.
 *
 * Returns:     nothing
 ***************************************************************************/
void handleLoopTimer(t_Event *event, BOOL process)
{
  int cnt=0;
  //int index;

  // for speed, don't process if no timers running
  if (timersRunning) {
    // keep up with ticks each time through
    incrTimers();

    // Increment each started timer and call tmr handlers if timeout occured
    for (cnt=0; cnt<MAX_TIMERS && process; cnt++) {
      if (tmrStarted(cnt)) {
        // has timer expired
        if (tmrExpired(cnt)) {
          // time to kick this timer handler!
          tmrHandlers[cnt].timerHdlrFunc(event);

          // only one timeout supported at a time
          break;
        }// timer expired
      }// timer started
    }// for
  }// if running timers
}

