#ifndef _TIMER_H
#define _TIMER_H

#include "event.h"
#include "accelerometer.h"
#include "common_def.h"

// 
// Raw timer values
//
#define KB_DEBOUNCE_TIMEOUT_MSEC    180
#define KB_POLL_TIMEOUT_MSEC        900

// How long to wait for receiver to acknowledge a sent packet
#define ACK_WAIT_TIMEOUT_MSEC       100

// Send a status packet every 5 seconds to keep in touch with
// receiver and light LED if out of range.
#define KEEPALIVE_SEND_TIMEOUT_MSEC 5000

// If no movement detected in this amount of time, revert from
// ready to idle mode
#define IDLE_TIMEOUT_MSEC           30000

// Flash the ready LED
#define READY_FLASH_TIMEOUT_MSEC    1000

// Don't check for gestures until acclerometers have
// settled down
#define GESTURE_DEBOUNCE_TIMEOUT_MSEC 1000

//
// END Raw timer values
//

// List of timers that can be started/stopped.  This list
// must match the order in timer.c
typedef enum {
  ACK_WAIT_TIMER,         // A movement sample is ready
  KEEPALIVE_SEND_TIMER,   // send a keepalive message to receiver
  IDLE_TIMER,             // monitor movement to transition to idle
  KB_DEBOUNCE_TIMER,      // debounce the keyboard
  KB_POLL_TIMER,          // poll keyboard interval
  READY_FLASH_TIMER,
  GESTURE_DEBOUNCE_TIMER, // debounce gesture detection
  MAX_TIMERS
} t_TimerId;



// Timer handlers: pointers to functions
typedef int (*t_TimerHdlrFunc)(t_Event *);

// For simplicity this array of timer handlers is just a flat
// structure based on the timing of the accelerometer sample.
// The application is expected to run timing in accordance
// with this requirement.
typedef struct {
  int timeoutMs;       // timer duration
  int timeoutCount;    // static timeout value
  int tickCount;       // incremented each timer tick
  BOOL restart;        // restart timer when times out
  t_TimerHdlrFunc timerHdlrFunc;
} t_TmrHandlers;

#define NIL_TIME  -1   

// Call this from within your main for loop every LOOP_TIMER mseconds
void handleLoopTimer(t_Event *event, BOOL process);
void startTimer(t_TimerId timerId, BOOL restartOnTimeout);
void stopTimer(t_TimerId timerId);
void stopAllTimers(void);


#endif
