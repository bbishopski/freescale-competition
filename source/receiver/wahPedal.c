/****************************************************************************
* wahPedal.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	wahPedal.c
* 
* This file abstracts the wah pedal hardware from the application.  The 
* application is responsible for intializing and setting the angle of the
* wah pedal.
*
****************************************************************************/
#include "MC13192_hw_config.h"
#include "wahPedal.h"

// The pot setting is the current setting of the pot
// somewhere between min/max
static UINT8 potSetting=WAH_POT_POWERONVALUE;
static long  absoluteMinWahAngle = 0;
static long  minWahAngle = 0;  // calibrated per session
static long  absoluteMaxWahAngle = 0;
static long  maxWahAngle = 0;  // calibrated per session
//static long  stepsPerDegree = 0;


// Prototypes
static void incrPotSetting(int nIncrement);
static void decrPotSetting(int nDecrement);
static void moveWiper(int nIncrement);
static void calibrateWah(void);


#ifdef MVMT_DEBUG
static char debugbuf[20];
#endif


/****************************************************************************
* initWahPedal
*
* Description: Initializes the wah pedal system
*
* Parms:       minAngle - minimum angle (in tenths, eg. 200 = 20 degrees
*              maxAngle - maximum angle of pedal in tenths
*
* Returns:     nothing
***************************************************************************/
void initWahPedal(int minAngle, int maxAngle)
{
  minWahAngle         = minAngle;
  absoluteMinWahAngle = minAngle;
  maxWahAngle         = maxAngle;
  absoluteMaxWahAngle = maxAngle;

  calibrateWah();

  // Setup hardware ports
  WAH_PARALLEL_PORT = WAH_PORT_DIRECTION;   

  // Slew rate disabled because POT has no minimum
  // rise/fall times... MAX rise/fall = 500us.  With
  // slew rate enabled  HCS08=3ns
  // slew rate disabled HCS08=30ns
  //
  // Therefore we will enable to keep timings low.
  WAH_PORT_SLEW = WAH_SLEW_VALUE;

  // Don't rely on pot setting at power up.  Set to a known
  // value
  setWahPedal(WAH_POT_POWERONVALUE);
}

/****************************************************************************
* setWahTop
*
* Description: Call this when the pedal is at top
*
* Parms:       topAngle - top angle (in tenths, eg. 200 = 20 degrees
*
* Returns:     nothing
***************************************************************************/
void setWahTop(int topAngle)
{
  // Set top AND bottom. Bottom is top - nominal range
  minWahAngle = getMax(topAngle - WAH_NOMINAL_RANGE_DEGREES, absoluteMinWahAngle);
  maxWahAngle = getMin(topAngle, absoluteMaxWahAngle);
  calibrateWah();
}

/****************************************************************************
* setWahPedalAngle
*
* Description: Application calls this to set the desired angle of the wah
*              pedal (simulates the real wah pedal angle).
*
* Parms:       angle - desired angle (in tenths, eg. 200 = 20 degrees
*
* Returns:     nothing
***************************************************************************/
void setWahPedalAngle(int angle)
{
  static long   tmpValue = 0;
  static UINT8 stepValue = 0;


  // determine pot value for given angle

  // Calibrate on the fly by setting bottom.  Top was set when
  // wah was turned on
  //setWahBottom(angle);

  // only set if valid angle  
  if (angle >= minWahAngle && angle <= maxWahAngle) {
    // Normalize the given angle to the absolute range  
    tmpValue  = angle - minWahAngle;
    tmpValue *= STEPS_PER_DEGREE;

    // Take off the precision, this is our stepValue
    tmpValue /= STEPS_PER_DEGREE_PRECISION;
    stepValue = (int)tmpValue;
  } else if (angle > maxWahAngle) {
    stepValue = WAH_POT_MAXVALUE;
  } else if (angle < minWahAngle) {
    stepValue = WAH_POT_MINVALUE;
  }

  setWahPedal(stepValue);

}

/****************************************************************************
* setWahPedal
*
* Description:  Sets wah pedal at given step value.  
*
* Parms:       stepValue - step value from 0-255
*
* Returns:     nothing
***************************************************************************/
void setWahPedal(UINT8 stepValue)
{
  UINT8 nSteps;

  // Make sure step value is in range
  if (stepValue < WAH_POT_MINVALUE) {
    stepValue = WAH_POT_MINVALUE;
  }

  if (stepValue > WAH_POT_MAXVALUE) {
    stepValue = WAH_POT_MAXVALUE;
  }

  // in this case we are using an incr/decr POT so we need
  // increment or decrement based on the current setting
  // and the given setting.
  if (stepValue > potSetting) {
    // Sanity check    
    nSteps = (stepValue - potSetting) % WAH_POT_STEPS;
    incrPotSetting(nSteps);
  } else if (stepValue < potSetting) {
    nSteps = (potSetting - stepValue) % WAH_POT_STEPS;
    decrPotSetting(nSteps);
  }
  // else if == then do nothing.

  // This is now the current value used to determine next
  // increment/decrement steps  
  //
  // If application and potSetting get out of sync then
  // we will not be able to recover until the pedal is turned
  // off/on again.  Therefore we will remain in sync with
  // application regardless of the sanity of the setting.
  //
  // However, we will not drive the POT farther than it
  // can go - even though it can handle such a case.
  //
  potSetting = stepValue;


#ifdef MVMT_DEBUG
  SCITransmitStr("POT:");
  itoa(stepValue, debugbuf, 20);  
  SCITransmitStr(debugbuf);
  SCITransmitStr("\r\n");
#endif

}


/* Typical electrical characteristics for the DS1804
CS to INC Setup             tCI 50 ns
U/ D to INC Setup           tDI 100 ns
INC Low Period              tIL 50 ns 
INC High Period             tIH 100 ns
INC inactive to CS Inactive tIC 500 ns 
CS Deselect Time            tCPH 100 ns
Wiper Change to INC Low     tIW 200 ns 
INC Rise and Fall Times     tR, tF 500 µs (30 with slew disabled) 
INC Low to CS Inactive      tIK 50 ns
*/
//
// 1) drive direction to increment (high) or decrement (low)
// 2) drive INC high
// 3) drive CS low:  wait (tCI (CS to INC Setup))
// 4) transition INC high-low: wait (tIL+tF)
// 5) transition INC low-high: wait (tIH+tR)
// 6) repeat 4,5 
// 7) leave low on last loop
// 8) drive CS high: wait (tIC)
//
// NOTE: If INC is high when CS transitions low-high, the
//       value will be stored in EEPROM - we don't need 
//       this feature.  Therefore, we will make sure INC
//       is low when CS is brought back to high
// 	 
// With these timings in mind, it could take up to
//  50ns + [steps * (50ns+3ns   + 100+3)] + 500ns
//
//  at 100 steps = 15.650us
//
//
// These minimum values are well below the speed at which
// we can switch them with a 16mhz clock
//
// Each clock cycle is 62.5ns at that speed, and a load
// instruction is a multi cycle instruction, so there
// should be no timing considerations with this chip
//
// nIncrement is number of steps to increment.  It
// is not sanitized.  If you go over the maximum steps
// the POT will stop at it's max setting
static void incrPotSetting(int nIncrement)
{
#ifdef MVMT_DEBUG
  SCITransmitStr("WIPERINCR:\r\n");
#endif

  // Drive direction to increment (high)
  WAH_POT_DIRECTION = TRUE;

  moveWiper(nIncrement);
}

static void decrPotSetting(int nDecrement)
{

#ifdef MVMT_DEBUG
  SCITransmitStr("WIPERDECR:\r\n");
#endif

  // Drive direction to decrement (low)
  WAH_POT_DIRECTION = FALSE;

  moveWiper(nDecrement);
}

// You must setup the direction register before calling
static void moveWiper(int nMoves)
{
  int idx;

#ifdef MVMT_DEBUG
  SCITransmitStr("WIPER:");
  itoa(nMoves, debugbuf, 20);  
  SCITransmitStr(debugbuf);
  SCITransmitStr("\r\n");
#endif


  // Drive INC high.  Each High to Low is an increment
  WAH_POT_INC = TRUE;  

  // Drive CS low.  After this you must wait at least
  // 50ns before driving INC low.  Not a problem at
  // the relatively low speed of 16mhz.
  WAH_POT_CHIP_SELECT = FALSE;

  // Increment given number of steps  
  for (idx=0; idx<nMoves; idx++) {
    // Transition increment from high to low
    WAH_POT_INC = FALSE;

    // Must be low for minimum 50ns

    // Transition from low to high, must be high
    // for 100ns, since we're going to top of loop
    // that will take a few clock cycles which should
    // be well over 100ns wait period

    if (idx+1 < nMoves) {
      // don't transition high on last loop.  If
      // we do then the EEPROM will be programmed
      // when we set CS high
      WAH_POT_INC = TRUE;
    }
  }

  // End wiper move
  WAH_POT_CHIP_SELECT = TRUE;  
}

/****************************************************************************
* calibrateWah
*
* Description: Does nothing. In the future could be used to calibrate.
*
* Parms:       none
*
* Returns:     nothing
***************************************************************************/
void calibrateWah(void)
{
  //static long angle;
  //static int  steps;

  // for better fixed point precision multiply by STEPS_PER_DEGREE_PRECISION  
  //steps = WAH_POT_STEPS * STEPS_PER_DEGREE_PRECISION;
  //angle = maxWahAngle - minWahAngle;
  //stepsPerDegree = steps / angle;
  //stepsPerDegree = STEPS_PER_DEGREE;
}






