#ifndef __SCI_H_
#define __SCI_H_

#include "common_def.h"


/* initialisation constants */
#define initSCI2C2	0b00101100

#if defined (AXM_0308C) || defined (MC13192SARD)
  #define  SCIBDH   SCI1BDH
  #define  SCIBDL   SCI1BDL
  #define  SCIC1    SCI1C1 
  #define  SCIC2    SCI1C2 
  #define  SCIS1    SCI1S1 
  #define  SCIS2    SCI1S2 
  #define  SCIC3    SCI1C3 
  #define  SCID     SCI1D
  
  #define SCIS1_TDRE SCI1S1_TDRE
  #define SCIS1_TC	 SCI1S1_TC
#endif

#if defined (ARD)
  #define  SCIBDH   SCI2BDH
  #define  SCIBDL   SCI2BDL
  #define  SCIC1    SCI2C1 
  #define  SCIC2    SCI2C2 
  #define  SCIS1    SCI2S1 
  #define  SCIS2    SCI2S2 
  #define  SCIC3    SCI2C3 
  #define  SCID     SCI2D  
  
  #define SCIS1_TDRE SCI2S1_TDRE
  #define SCIS1_TC	 SCI2S1_TC
#endif

/* SCI functions */
void SCIInit(UINT8 baud);
void SCIStartTransmit(char cData);
void SCITransmitStr(char *pStr);
interrupt void Vscirx();
void SCITransmitArray(char *pStr, UINT8 length);

#endif