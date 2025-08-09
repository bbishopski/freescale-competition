/****************************************************************************
* common_lib.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	common_lib.c
* 
* Commonly used utilities.
*
****************************************************************************/
#include "common_def.h"

/****************************************************************************
* itoa
*
* Description: converts ints to ascii.  Currently on positive numbers.
*
* Parms:       num - integer to convert
*              buf - pointer to converted ascii string
*              buflen - length of buf
*
* Returns:     nothing
***************************************************************************/
void itoa(unsigned int num, char *buf, int buflen) {

  int tmp, idx, units, n;
  for (units = 100, idx=0,tmp=num; units > 0 && idx < buflen;) {
    n = tmp / units;
    buf[idx++] = n + '0';
    if (tmp >= units)
      tmp = tmp - (n * units);
    units/=10;
  }

  buf[idx] = '\0';
}


/****************************************************************************
* isqrt
*
* Description: Integer square root by Halleck's method, with Legalize's speedup
*
* Parms:       x - take square root of this
*
* Returns:     square root of x
***************************************************************************/
long isqrt (long x)
{

  static long   squaredbit, remainder, root, lTmp;

  /* Load the binary constant 01 00 00 ... 00, where the number
   * of zero bits to the right of the single one bit
   * is even, and the one bit is as far left as is consistant
   * with that condition.)
   */
  squaredbit  = (long) ((((unsigned long) ~0L) >> 1) & 
                        ~(((unsigned long) ~0L) >> 2));
  /* This portable load replaces the loop that used to be 
   * here, and was donated by  legalize@xmission.com 
   */

  /* Form bits of the answer. */
  remainder = x;  
  root = 0L;
  while (squaredbit > 0L) {
    lTmp = squaredbit | root;
    if (remainder >= lTmp) {
      remainder -= lTmp;
      root >>= 1; 
      root |= squaredbit;
    } else {
      root >>= 1;
    }
    squaredbit >>= 2; 
  }

  return root;
}


