#ifndef _WAH_PEDAL_H
#define _WAH_PEDAL_H

#include "common_def.h"

// Abstracts the WahPedal interface from the application
//
// The pedal is controlled by a digital potentiometer.  The
// linear POT is controlled by up/down signals - 100 steps
//
// In the future this can easily be swapped out for a 256
// IIC or SCI controlled pot.  However, because that was
// much harder to do on the SARD board, the up/down will
// suffice as a demonstration.
//
// However, the application interface is abstracted from
// the knowledge of how the pot works.  The app needs to
// determine the step between movements of the acclerometer.
//
// For example if a high accelerometer reading is received
// the pot needs to move more than if a low accelerometer
// reading is received.  However, the application does need
// to know the step range of the POT, which may be different
// for various POTs.  Also, the application must handle the
// timing of when the pot should move and how far.
//
// For SARD board, wah pedal is interfaced from port C
//
// Port C will be configured as a parallel port driving
// the digital POT interface
//
#define WAH_PARALLEL_PORT     PTBDD

// Set our control pins to output
#define WAH_PORT_DIRECTION 		(PTBDD_PTBDD0_MASK|PTBDD_PTBDD1_MASK|PTBDD_PTBDD2_MASK|PTBDD_PTBDD3_MASK)

// Slew Port
#define WAH_PORT_SLEW         PTBSE

// Slew values for all bits
#define WAH_SLEW_VALUE        (PTBSE_PTBSE0_MASK|PTBSE_PTBSE1_MASK|PTBSE_PTBSE2_MASK|PTBSE_PTBSE3_MASK)
//#define WAH_SLEW_VALUE        0

// Port B0 is Chip Select
#define WAH_POT_CHIP_SELECT   PTBD_PTBD0

// Port B1 is INC/DEC
#define WAH_POT_INC           PTBD_PTBD1

// Port B2 is Direction (up or down)
#define WAH_POT_DIRECTION     PTBD_PTBD2



// The 100k pot in the wah pedal ranges from  4k-100k
// 4k = pedal down
// 100k = pedal up
//
// A real wah pedal only moves about 5 degrees because it
// usually has a large base.  Since there is no base involved
// we can measure about 12 degrees of foot movement, from
// floor to top.
#define WAH_POT_MINVALUE      4
#define WAH_POT_MAXVALUE      99
#define WAH_POT_STEPS         (WAH_POT_MAXVALUE - WAH_POT_MINVALUE)
#define WAH_POT_POWERONVALUE  WAH_POT_MAXVALUE

// Typical range in degrees that the system supports
// However, will get calibrated if the user can achieve better
#define WAH_NOMINAL_RANGE_DEGREES  120  // 12 degrees	 
#define STEPS_PER_DEGREE_PRECISION 100

// Help the compiler out. It doesn't seem to be so good at math
// I tried to use the constants to figure it out, but the compiler
// got it wrong ((WAH_POT_STEPS * STEPS_PER_DEGREE_PRECISION) / WAH_NOMINAL_RANGE_DEGREES)
#define STEPS_PER_DEGREE  84



// Application interfaces
void setWahPedalAngle(int angle);
void initWahPedal(int minAngle, int maxAngle);
void setWahTop(int topAngle);
void setWahPedal(UINT8 stepValue);



#endif