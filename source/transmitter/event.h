#ifndef _EVENT_H
#define _EVENT_H

#include "common_def.h"

// Events that can occur in this system
typedef enum {
  NIL_EVENT,
  SYSTEM_INIT,
  MVMT_SAMPLE_READY, // a movement sample is ready
  TIMER_EXPIRED,     // timer has popped
  MVMT_OCCURED,      // movement has been detected
  ACK_RECEIVED,      // RF Ack received
  ACK_TIMEOUT,       // RF Ack timed out
  KB_PRESS,           // a KB press has occured
  KB_EVENT,          // debounce finished, process kb now
  IDLE_LOOP_WAIT,    // go into low power mode
  MAX_EVENTS
} t_EventId;

#define MAX_EVENT_DATASIZE 10

// Cross application data block (i.e. global data)
typedef struct {
  volatile UINT8 identifier;
  volatile BOOL  waitingForAck;
  volatile BOOL  ackReceived;
  volatile BOOL  s1Pressed;
  volatile BOOL  s2Pressed;
} t_CADB;


// Event data structure
typedef struct {
  t_EventId eventId;
  short     timerId;    // allows states to get specific timer pops

  char      data[MAX_EVENT_DATASIZE];

  // global data used by all applications
  t_CADB    *pCADB;
} t_Event;

#endif