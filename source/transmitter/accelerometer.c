/****************************************************************************
* accelerometer.c
* 
* Author: 	Bill Bishop - Sixth Sensor
* Title: 	  accelerometer.c
* 
* This file contains the interface to the accelerometers and the movement
* detection system.  The details of how to detect movements, gestures and
* other hardware specific details are encapsulated in this module.
*
****************************************************************************/
#include "accelerometer.h"


// Maintain two tables for historical purposes.  For example if a 
// jolt occurs at the beginning of a 2 second sample, how can we 
// determine that a jolt really occured if we can't look back at 
// the previous sample.
// The last index in the table contains the sum of the table.
static tSampleData  activityTable1;
static tSampleData  activityTable2;
static tAccSample   gestureTable[TILT_GESTURES];
static tSampleData  *pActivityTable=&activityTable1;
static short        sampleIndex=0;
static short        maxSampleIdx=0;
static tAccSample   prevGestureOff=0;

// Function Prototypes
static BOOL joltOccured(tIntegratedSample sample1, tIntegratedSample sample2);
static int joltDetected(void);

// Debug variables
#ifdef MVMT_DEBUG
static UINT8 debugJoltThreshold;
#endif

void ACC_init() {
#ifndef SIM_MODE

  // enable accelerometer  

  // enable desired ADC channels (AD0, AD1, AD7 on)
  ATD1PE=ADCChannelEnable;

  // ATD powered up, 8bit unsigned, right justified, prescale = 4, 
  ATD1C=ADCPrescaler_by_4; 
#endif

}

/****************************************************************************
 * ACC_read_x
 *
 * Description: Reads the x axis. This function generates a single conversion 
 *              and waits until it is completed. The result is returned as 
 *              an 8 bit variable.
 *
 * Parms:       xVal - pointer to variable to hold reading.
 *
 * Returns:     nothing. 
 ***************************************************************************/
void ACC_read_x(byte *xVal) {
#ifndef SIM_MODE
  UINT8 u8AttemptCount = 10; // Limit amount of attempts.

  ATD1SC = 0x01;//read X channel
  while ((ATD1SC & 0x80) != 0x80 && (u8AttemptCount>0)) {
  }
  u8AttemptCount--;
  *xVal = ATD1RH;
#endif
}

/****************************************************************************
 * ACC_read_y
 *
 * Description: Reads the y axis. This function generates a single conversion 
 *              and waits until it is completed. The result is returned as 
 *              an 8 bit variable.
 *
 * Parms:       xVal - pointer to variable to hold reading.
 *
 * Returns:     nothing. 
 ***************************************************************************/
void ACC_read_y(byte *yVal) {
#ifndef SIM_MODE
  UINT8 u8AttemptCount = 10; // Limit amount of attempts.

  ATD1SC = 0x00;//read Y channel
  while ((ATD1SC & 0x80) != 0x80 && (u8AttemptCount>0)) {
  }
  u8AttemptCount--;
  *yVal = ATD1RH;
#endif
}


/****************************************************************************
 * ACC_read_z
 *
 * Description: Reads the z axis. This function generates a single conversion 
 *              and waits until it is completed. The result is returned as 
 *              an 8 bit variable.
 *
 * Parms:       xVal - pointer to variable to hold reading.
 *
 * Returns:     nothing. 
 ***************************************************************************/
void ACC_read_z(byte *zVal) {
#ifndef SIM_MODE
  UINT8 u8AttemptCount = 10; // Limit amount of attempts.

  ATD1SC = 0x07;//read Z channel
  while ((ATD1SC & 0x80) != 0x80 && (u8AttemptCount>0)) {
  }
  u8AttemptCount--;
  *zVal = ATD1RH;
#endif
}

/****************************************************************************
 * ACC_read_z
 *
 * Description: Initialize this system
 *
 * Parms:       none
 *
 * Returns:     nothing. 
 ***************************************************************************/
void ACC_MovementInit(void)
{
  int i;

  // Initialize sample buffers
  for (i=0;i<MAXSAMPLES;i++) {
    activityTable1.sample[i].integratedSample=0;
    activityTable2.sample[i].integratedSample=0;
    activityTable1.sample[i].y_axisSample=0;
    activityTable2.sample[i].y_axisSample=0;
  }

  activityTable1.runningSum = 0;
  activityTable2.runningSum = 0;
  pActivityTable = &activityTable1;
  prevGestureOff = 0;
}

/****************************************************************************
 * nextActivityTable
 *
 * Description: Increment to next table.  Call when current table is full
 *
 * Parms:       none
 *
 * Returns:     nothing. 
 ***************************************************************************/
void nextActivityTable(void)
{
  // set activity table to next table  
  if (pActivityTable == &activityTable1) {
    pActivityTable = &activityTable2;
  } else {
    pActivityTable = &activityTable1;
  }
}

/****************************************************************************
 * movementInit
 *
 * Description: Application must call after a movement detect 
 *              or jolt detect, etc.
 *
 * Parms:       sampleRate - number of samples per second the app wishes to 
 *              take.
 *
 * Returns:     nothing. 
 ***************************************************************************/
void movementInit(short sampleRate)
{
  int i;

  nextActivityTable();

  for (i=0;i<MAXSAMPLES;i++) {
    pActivityTable->sample[i].integratedSample=0;
    pActivityTable->sample[i].y_axisSample=0;
  }
  pActivityTable->runningSum = 0;

  // Reset the global sample counter  
  sampleIndex=0;

  maxSampleIdx = sampleRate;
}

/****************************************************************************
 * getActivityTable
 *
 * Description: Gets active table.
 *
 * Parms:       none
 *
 * Returns:     pointer to active table. 
 ***************************************************************************/
tSampleData *getActivityTable(void)
{
  return pActivityTable;  
}

/****************************************************************************
 * getPrevActivityTable
 *
 * Description: Gets previous (holding) table.
 *
 * Parms:       none
 *
 * Returns:     pointer to holding table. 
 ***************************************************************************/
tSampleData *getPrevActivityTable(void)
{
  // current table is table1
  if (pActivityTable == &activityTable1) {//[0])
    // return table2
    return &activityTable2;//[0];
  } else {
    return &activityTable1;//[0];
  }
}

/****************************************************************************
 * gestureInit
 *
 * Description: Call this before detecting gestures.
 *
 * Parms:       none
 *
 * Returns:     nothing
 ***************************************************************************/
void gestureInit()
{
  prevGestureOff = 0;
}

/****************************************************************************
 * gestureInit
 *
 * Description: Determines if off gesture was executed by user.
 *
 * Parms:       accSample - last sample on the gesture axis
 *              gestureOffAcceleration - threshhold of gesture 
 *
 * Returns:     True if off detected, false if not
 ***************************************************************************/
int gestureOffDetected(tAccSample accSample, int gestureOffAcceleration)
{
  int retcode = 0;
  int gestureAcceleration;

  if (prevGestureOff > 0) {
    // if gesture off axis moves more than predefined
    // acceleration, then turn off
    gestureAcceleration = abs(accSample - prevGestureOff);

    if (gestureAcceleration > gestureOffAcceleration) {
      retcode = 1;
    }
  }

  // Store this sample for next check
  prevGestureOff = accSample;

  return retcode;  
}

/****************************************************************************
 * getSampleTable
 *
 * Description: Returns a history of samples starting at currentSample.  Will
 *              skip back in time by debounceSamples and retrieve numSamples
 *              before that time.
 *
 * Parms:       pTable - table to be updated
 *              nCurrentSample - index to current sample
 *              nDebounceSamples - number of samples to ignore due to bounce 
 *              nNumSamples - number of samples to get (pre-bounce) 
 *
 * Returns:     True if successful, false if not
 ***************************************************************************/
static int getSampleTable(tAccSample *pTable,   int nCurrentSample, 
                          int nDebounceSamples, int nNumSamples)
{
  int nStartSample = 0;
  int nRetrieved = 0;
  int idx = 0;
  tSampleData *pPrevious = getPrevActivityTable();

  if (nNumSamples <= maxSampleIdx) {
    // Since we have 2 tables to deal with, handle the case
    // where some samples come out of one table and the rest
    // of the samples come from the other table  
    nStartSample = nCurrentSample - (nDebounceSamples + nNumSamples);

    // Grab all we need to from previous table  
    if (nStartSample < 0) {
      // eg. 512 + (-12) == 500
      idx = maxSampleIdx + nStartSample;

      // grab this many samples from previous table first
      for (;nStartSample < 0 && nRetrieved < nNumSamples;nRetrieved++,nStartSample++) {
        pTable[nRetrieved] = pPrevious->sample[idx++].y_axisSample;
      }

      // start at 0 for next table    
      idx = 0;
    }

    // Now get the rest from current table
    for (;nRetrieved < nNumSamples; nRetrieved++) {
      pTable[nRetrieved] = pActivityTable->sample[idx++].y_axisSample;
    }
  }// if nNumSamples <= maxSampleIdx

  return nRetrieved;
}

/****************************************************************************
 * gestureOnAngle
 *
 * Description: Determines if given x axis sample is the proper gestureOn 
 *              angle.
 *
 * Parms:       yAxis - gesture axis sample
 *
 * Returns:     True if on, false if not
 ***************************************************************************/
static int gestureOnAngle(tAccSample yAxis)
{
  int retcode = 0;

  if (yAxis >= GESTURE_ON_MIN_Y && yAxis <= GESTURE_ON_MAX_Y) {
    retcode = 1;
  }

  return retcode;
}

/****************************************************************************
 * gestureOnAngle
 *
 * Description:  Determine if last sample resulted in gesture to turn on
 *               device. Must have previously called movementSample.
 *               A gestureOn is defined as a jolt with a certain inclination
 *               of the gesture axis.
 *
 * Parms:       none
 *
 * Returns:     True if on, false if not
 ***************************************************************************/
int gestureOnDetected(void)
{
  int joltIndex = -1;
  int retcode = 0;
  int idx = 0;
  int gestureCnt = 0;
  tAccSample xSample=0;

  // These are static because I believe there is a compiler
  // bug!  If they aren't static it doesn't always pull out
  // the correct value from the table!
  static tIntegratedSample sample1=0;
  static tIntegratedSample sample2=0;
  tSampleData *pPrevious = getPrevActivityTable();


  //
  // We need two samples to determine if gesture occured. 
  // First get the latest 2 samples.  sampleIndex is assumed
  // to be already incremented when sample was taken
  //

  // handle the case where this is the first sample in the 
  // new sample buffer (previous sample in previous buffer)
  if (sampleIndex == 1 && pPrevious->sample[maxSampleIdx-1].integratedSample > 0) {
    // ---- first sample in new table ----
    //
    // first sample is last sample in previous table.  
    // second sample is 0 in current table. 
    sample1 = pPrevious->sample[maxSampleIdx-1].integratedSample;
    sample2 = pActivityTable->sample[0].integratedSample;

    // if it turns out that a jolt occured, this is the place
    // where it occured.
    joltIndex = 0;
  } else if (sampleIndex > 1) {
    // ---- second sample in new table ----
    //
    // first sample is previous sample
    // second sample is current sample (the last sample taken)
    sample1 = pActivityTable->sample[sampleIndex-2].integratedSample;
    sample2 = pActivityTable->sample[sampleIndex-1].integratedSample;

    // if it turns out that a jolt occured, this is the place
    // where it occured.
    joltIndex = sampleIndex-1;

  }

  // else sampleIndex == 0 or 1 - can't determine gesture because
  // we need two samples minimum


  // Determine if jolt occured between 2 samples.  Will return
  // false if either is 0
  //
  // Only drop in here if we set sample1 and sample2 above
  if (joltIndex >= 0) {
    // did a jolt just occur
    if (joltOccured(sample1, sample2)) {
      // Debounce the jolt - a jolt takes a few samples to
      // level out.  Get a sample table taking out the bounce
      // from the jolt
      getSampleTable(gestureTable, joltIndex, DEBOUNCE_JOLT_SAMPLES, TILT_SAMPLES);

      // determine if the foot was properly angled for the
      // proper amount of time.
      for (gestureCnt=idx=0; idx<TILT_SAMPLES; idx++) {
        // count the number of positive gesture matches
        if (gestureOnAngle(gestureTable[idx])) {
          gestureCnt++;
        }
      }

      // now determine if positive gesture matches is enough
      // to trigger a positive match.  This should smooth out
      // any noise
      if (gestureCnt >= TILT_GESTURES) {
        // We have a positive match!!!
        retcode = 1;
      }
    }
  }

  return retcode;
}

/****************************************************************************
// Take a movement sample.  Movement is defined as movement
// over a unit of time, where movement is standard deviation 
// greater than a predetermined value.  Gesture checking only
// requires two samples.
//
// Returns true if buffer is full.  The application can then
// check for movement or gestures.  The size of the buffer
// determines how long of a sample is analyzed for movement.
// movementDetected() should not be called until this procedure
// returns true.  Gesture checking can be called at any time since
// it only requires two samples.
//
// When this procedure returns true, the application MUST 
// call movementInit() before calling this procedure again. 

// These three variables keep the isqrt from overflowing and
// corrupting my data!  This is yet another bug in the compiler.
 ***************************************************************************/
unsigned long tmpVal, tmpVal2, tmpVal3;
int movementSample(void)
{
  int retcode = 0;
  static tAccSample accX, accY, accZ;
  static unsigned long longX, longY, longZ, squareX, squareY, squareZ, sumOfSquares;
  static unsigned long sqrtSumOfSquares;
  static long tmpSum1, tmpSum2;
  static tIntegratedSample sampleStore;

  tmpVal = pActivityTable->runningSum;

  // Make sure we don't overflow boundaries.  Application
  // must call init when the sample buffer is full  
  if (sampleIndex >= maxSampleIdx) {
    retcode = 1;
  } else {
    ACC_read_x(&accX);
    ACC_read_y(&accY);
    ACC_read_z(&accZ);

#ifdef SIM_MODE
    accX = accY = accZ = 255;
#endif

    // Store the square root of the sum of the squares  
    longX = (long)accX;
    longY = (long)accY;
    longZ = (long)accZ;
    squareX = longX * longX;
    squareY = longY * longY; 
    squareZ = longZ * longZ; 

    sumOfSquares = squareX+squareY+squareZ;

    // store the square root of the sum of the squares
    //
    // NOTE: this is very CPU intensive. This takes 4ms to
    // process at 16mhz!

    sqrtSumOfSquares = isqrt (sumOfSquares);


    // This little piece of code keeps my data from getting
    // corrupted because of compiler bugs!
    tmpVal2 = pActivityTable->runningSum;
    if (tmpVal != tmpVal2) {
      tmpVal3 = 3; 
    }

    sampleStore = (tIntegratedSample)sqrtSumOfSquares;

    // sanity check  
    sampleStore = getMin(sampleStore, 441);
    pActivityTable->sample[sampleIndex].integratedSample = sampleStore;

    // store x axis sample
    pActivityTable->sample[sampleIndex].y_axisSample = accY;

    tmpSum1 = (long)sampleStore;
    tmpSum2 = pActivityTable->runningSum;

    // Store the sum, makes it easier to calculate avg later
    pActivityTable->runningSum = (tmpSum1 + tmpSum2); 

    if (++sampleIndex >= maxSampleIdx) {
      // it's up to the application to now check for movement
      // or gestures.
      retcode = 1;
    }
  }// if valid index

  return retcode;
}

/****************************************************************************
 * calcVariance
 *
 * Description:  Determines standard deviation from the mean for active buffer.
 *               Standard Deviation is calculated as the square root of the 
 *               variance.  The variance is the average of the differences 
 *               from the mean of the series. A data series like 1, 2, 3, 6 
 *               has a mean equal to (1+2+3+6)/4=3. The differences from the 
 *               mean are: -2, -1, 0, +3. The variance is: {(-2)^2 + (-1)^2 + 
 *               0 + (3^2)}/4=14/4=3.5. Finally, the standard deviation is 
 *               equal to the square root of the variance: SQR(3.5)=1.87. 
 *               One serious problem with the standard deviation as an 
 *               analytical tool: It is distorted by extreme values 
 *               (extremely high, or extremely low) in the data series. 
 *
 * Parms:        mean - mean of the series
 *
 * Returns:      variance from the mean for the series
 ***************************************************************************/
unsigned long calcVariance(int mean)
{
  int i=0;
  int variance;

  // Had to make these static because of stack overflow or some other
  // issue.  Need to investigate later.
  static unsigned long vSquare=0, vSquareTotal=0, vSquareAvg=0;
  static int vInt=0; 
  static unsigned long nSamples;
  static unsigned long stdDeviation;

  vSquareTotal=0;  
  for (i=0; i<maxSampleIdx; i++) {
    variance =  pActivityTable->sample[i].integratedSample - mean;
    variance = abs(variance);

    vInt = (int)variance;
    vSquare = vInt * vInt;
    vSquareTotal+=vSquare;
  }

  nSamples = maxSampleIdx;
  vSquareAvg = vSquareTotal/nSamples;

  // now the square root gives us std deviation  
  stdDeviation = isqrt (vSquareAvg);

  return stdDeviation;
}

/****************************************************************************
 * movementDetected
 *
 * Description: Returns true if movement is detected over the entire sample
 *              period.
 *
 * Parms:       none
 *
 * Returns:     True if mvmt detected, false if not
 ***************************************************************************/
int movementDetected(void)
{
  int retcode = 0;
  static unsigned long mean, sum, numsamp;
  static unsigned long stdDeviation;

  // the sum has already been calculated on the fly, just divide by num samples  
  numsamp = maxSampleIdx;
  sum = pActivityTable->runningSum;
  mean = sum / numsamp;

  // calculate the variance from the mean
  stdDeviation = calcVariance((int)mean);

  if (stdDeviation > NO_MOVEMENT_DEVIATION) {
    // movement detected
    retcode = 1;
  }

  return retcode;                

}

/****************************************************************************
* joltOccured
*
* Description: Determines if jolt occured between the two samples
*
* Parms:       sample1, sample2 - did jolt occur between these 2 samples.
*
* Returns:     True if jolt detected, false if not
***************************************************************************/
BOOL joltOccured(tIntegratedSample sample1, tIntegratedSample sample2)
{
  BOOL retcode = FALSE;
  int jolt;

  // Both samples must be valid  
  if (sample1 > 0 && sample2 > 0) {
    jolt = abs(sample1-sample2);

    if (jolt >= JOLT_DETECTION_THRESHHOLD) {
      retcode = TRUE;
    }
  }

#ifdef MVMT_DEBUG
  if (retcode) {
    // store jolt threshhold to be sent for debugger
    debugJoltThreshold = (UINT8)jolt;
  }
#endif

  return retcode;

}

/****************************************************************************
* joltOccured
*
* Description: Detects a jolt by comparing samples and looking for a big
*              jump in the amount of g's pulled by all accelerometers.
*              This determination is very much dependent on the sampling
*              rate.  For example, if sampling 16 times per second
*              the jolt must occur within .0625 seconds.
*
*              This procedure needs 2 samples to perform properly.  It
*              will return -1 if 2 samples have not been taken.
*
* Parms:       none
*
* Returns:     True if jolt detected, false if not
***************************************************************************/
static int joltDetected(void)
{
  int retcode = -1;
  short i;

  // first check jolt between last sample (stored in previous table)
  // and first sample in this table  
  tSampleData *pPrevious = getPrevActivityTable();

#ifdef MVMT_DEBUG
  debugJoltThreshold = 0;
#endif  

  // compare last sample from previous to first sample from current  
  // (if not first time through!).  This handles the case
  // where this procedure is called right after the first
  // sample is taken in the new sample table.  So we'll compare
  // this first sample to the last sample in the previous
  // table.  Then we'll do the loop below to compare the
  // rest of the table.
  if (pPrevious->sample[maxSampleIdx-1].integratedSample > 0) {
    // did a jolt occur
    if (joltOccured(pPrevious->sample[maxSampleIdx-1].integratedSample,
                    pActivityTable->sample[0].integratedSample)) {
      retcode = 0;
    }
  }

  // See if jolt occured in entire sample  
  for (i=0; retcode<0 && i<maxSampleIdx-1; i++) {
    if (joltOccured(pActivityTable->sample[i].integratedSample, pActivityTable->sample[i+1].integratedSample)) {
      retcode = i;
      break;
    }
  }

  return retcode;
}



