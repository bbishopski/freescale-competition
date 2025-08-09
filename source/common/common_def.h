// Common definitions that aren't defined in pub_type.h
//
// Don't want to modify pub_def.h since it's part of SMAC
#ifndef ___COMMON_DEF_H
#define ___COMMON_DEF_H 


typedef unsigned char BOOL;

typedef signed char INT8;
typedef unsigned char UINT8;
typedef volatile signed char VINT8;
typedef volatile unsigned char VUINT8;

typedef signed short INT16;
typedef unsigned short UINT16;
typedef volatile signed short VINT16;
typedef volatile unsigned short VUINT16;

typedef signed long INT32;
typedef unsigned long UINT32;
typedef volatile signed long VINT32;
typedef volatile unsigned long VUINT32;


#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

// currently only converts positive numbers up to hundreds
void itoa(unsigned int num, char *buf, int buflen);

// take the square root of an integer
//unsigned long iSqrt(unsigned long value);
long isqrt (long x);



#endif
