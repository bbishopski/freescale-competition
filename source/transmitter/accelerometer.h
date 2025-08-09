#ifndef _ACCELEROMETER_H
#define _ACCELEROMETER_H

#include "smac_MC9S08GT60.h"
#include "common_def.h"
#include "pub_def.h"
#include "HAL.h"

// ADC Control Bits - for ATD1C register.
// all values contain: ATD powered up, right justification, 8bit unsigned.  
// The only value that changes is the prescaler
enum ADCControl {
  ADCPrescaler_by_2 = 0xE0,
  ADCPrescaler_by_4 = 0xE1,
  ADCPrescaler_by_6 = 0xE2,
  ADCPrescaler_by_8 = 0xE3,
  ADCPrescaler_by_10= 0xE4
};

// enable desired ADC channels (AD0, AD1, AD7 on)
#define ADCChannelEnable 0x83

void ACC_init();
void ACC_read_x(byte *xVal);
void ACC_read_y(byte *yVal);
void ACC_read_z(byte *zVal);

// Max number of samples per second (1/ACC_SAMPLE_FREQUENCY_FAST)
#define ACC_SAMPLES_PER_SECOND      256  

// After detecting jolt, wait this long before starting to detect
// movement.  If someone has fallen, then the jolt may be followed
// by movement caused by the fall.
#define MOVEMENT_CHECK_WAIT_SEC     6
#define MOVEMENT_CHECK_WAIT_CNT     (MOVEMENT_CHECK_WAIT_SEC/ACC_SAMPLE_PERIOD_SEC)

// Allowable sample frequencies
#define ACC_SAMPLES_PER_SECOND_SLOW 16
#define ACC_DOZE_FREQ_SLOW          RTI_EXT_DOZE_62_MSEC
#define ACC_SAMPLE_FREQUENCY_SLOW   62

#define ACC_SAMPLES_PER_SECOND_FAST 256
#define ACC_DOZE_FREQ_FAST          RTI_EXT_DOZE_4_MSEC
#define ACC_SAMPLE_FREQUENCY_FAST   4

// Number of seconds to sample before determining movement
#define ACC_SAMPLE_PERIOD_SEC       2 

// Check for movement for 30 seconds. 
//#define MOVEMENT_RETRY_SEC          30
//#define MOVEMENT_RETRY_CNT          (MOVEMENT_RETRY_SEC/ACC_SAMPLE_PERIOD_SEC)

typedef unsigned short tIntegratedSample;
typedef byte tAccSample;

#define MAXSAMPLES (ACC_SAMPLES_PER_SECOND*ACC_SAMPLE_PERIOD_SEC)

typedef struct
{
   tIntegratedSample integratedSample;
   tAccSample y_axisSample;
}tSamplePoint;

typedef struct
{
  tSamplePoint  sample[MAXSAMPLES];
  long          runningSum;
}tSampleData;

// Maximum value that can be stored for a sample
//255 - max accelerometer reading
//65025 - 255^2
//195075 - 255^2 * 3 (all 3 axes)
//441.672955930063709849498817084 - this is the largest number possible
#define MAX_SAMPLE_VALUE    441

// anything greater than this std deviation means movement occured
#define NO_MOVEMENT_DEVIATION 4

// a 1-g jolt is not much when considering a drop test of a helmet
// produces more than 100g!  However, this value is the difference
// between two 62ms samples.  Therefore, we do not need to detect a huge
// g force, just a big change in g force in a short amount of time.
//
// In testing this value worked best to detect falls accurately without
// too many false positives.  However, it is better to error on the 
// side of caution. The user has time to cancel the alarm if they did
// not actually fall.
//
// This value is not a G value it is the difference between two samples
// and each sample is SQRT of sum of the squares of x,y,z.
//
// Play it safe and assume that the fall was almost straight down.
// This will detect jolt in many situations, however the jolt must
// be preceded by a quick descent in the x-axis.
#define JOLT_DETECTION_THRESHHOLD 80 //100

// Falls are different from jolts.  A jolt is a quick movement as determined
// by looking at subsequent samples.  A fall occurs over a greater period of
// time (eg. .5 seconds), with a jolt at the end as impact occurs.
//
// Falls can also take different forms: you could fall forward, backward
// or many combinations of x,y,z axis.  The common event being a jolt
// at the end of the fall. 
//
// Analyze g-forces on X axis over this many samples to determine
// if fall occured
#define FALL_SAMPLE_PERIODS       6  // ~.3 seconds

// Constant for the data conversion to Gs.  Accelerometers will read 
// approximately 61 for each g.  61=-1g, 122=0g, 183=1g, etc.
// However this value is for one axis only.  In a fall situation
// the g-forces may be directly on one axis (straight down) or on
// multi axis such as falling forward.  Therefore, I will take a multiple
// step approach:
//
// 1) A fall is defined as a fall in the x-axis followed by jolt
// 2) a jolt value is defined that errors on the side of caution
// 3) after a jolt, the previous x-axis samples are analyzed to 
//    determine if a fall occured.
#define SINGLE_AXIS_G_CONST      61      // single axis value
#define GFORCE_1                 (SINGLE_AXIS_G_CONST*3)

// If avg value over FALL_SAMPLE_PERIODS on x axis is greater than
// this value, then you have experienced a fall.  That's good 
// enough to set off the alarm.
#define FALL_DETECT_AVG_X        100       // slightly more than 1g fall


// **********
// GESTURE MATCHING
//
// The gesture to turn on the device is angling your foot at
// the proper angle (somewhere between 15-45 degrees) and kicking
// down with your heel.
//
// Since the kick with the heel will cause a bounce we will
// debounce the jolt.  To debounce we ignore the last DEBOUNCE_JOLT_SAMPLES
// before the jolt occured.
//
// To determine gesture we determine if the foot was properly
// tilted for TILT_GESTURES during the last TILT_SAMPLES 
// (after considering the debounce) 
// 
// On gesture y value somewhere between these two numbers.  If jolt
// occurs with y value between these two values then the
// device transitions into run mode.  
#define GESTURE_ON_MIN_Y          95
#define GESTURE_ON_MAX_Y          110

// Move the gesture axis by this much acceleration to
// turn off (for example, side to side movement)
//#define GESTURE_OFF_ACCELERATION  20 // 30 works for 14hz sample, for 7hz use 20

// Jolt bounces - to debounce, ignore this many samples before
// the jolt occurs to get accurate measurements.
#define DEBOUNCE_JOLT_SAMPLES     (ACC_SAMPLES_PER_SECOND/5) // .2 seconds

// Number of samples to look at to determine if board was tilted
// for a gesture determination
#define TILT_SAMPLES              (ACC_SAMPLES_PER_SECOND/2) // .5 seconds

// In that TILT_SAMPLE, the number of samples that must have 
// a positive match for the gesture to be true: .
#define TILT_GESTURES             (ACC_SAMPLES_PER_SECOND/4) //.25 seconds


// initialize the movement detection system
void movementInit(short sampleRate);

// Take a sample of x,y,z accelerometer and integrate the
// results into a combined sample
int movementSample(void);

// Was movement detected over the sample period
int movementDetected(void);

// Was fall detected
int fallDetected(void);

void gestureInit();
int gestureOnDetected(void);
int gestureOffDetected(tAccSample accSample, int gestureOffAcceleration);
//tAccSample xAxisSample(void);

// Application must initialize the movement sensor system
// before using.
void ACC_MovementInit(void);

#endif

