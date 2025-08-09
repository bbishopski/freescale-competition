#ifndef _STATEMACH_H
#define _STATEMACH_H

// State machine states
typedef enum 
{ IDLE_STATE,       //
  KEEPALIVE_STATE,  // received keepalive, send ack
  WAH_ON_STATE,     // wah is turned on
  WAH_MOVE_STATE,   // wah is moving
  WAH_OFF_STATE,    // wah turned off
  MAX_STATES
} t_AppStates;


#endif