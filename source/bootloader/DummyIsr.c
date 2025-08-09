/************************************************************************************
* This module contains the dummy ISR function
* 
*
* Author(s):  Michael V. Christensen
*
* (c) Copyright 2004, Freescale, Inc.  All rights reserved.
*
* Freescale Confidential Proprietary
* Digianswer Confidential
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
* Last Inspected: 29-03-01
* Last Tested:
************************************************************************************/
#ifndef WIN32

// **************************************************************************

__interrupt void Default_Dummy_ISR(void)
{
   // Unimplemented ISRs trap.
   asm BGND;
}

// **************************************************************************

#endif WIN32