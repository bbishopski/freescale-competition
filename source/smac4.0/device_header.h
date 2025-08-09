/**************************************************************
*	Header: 	Select the desired core
***************************************************************/
#include "pub_def.h"

#if defined (AXM_0308) || defined (AXM_0308A) || defined (AXM_0308B) || defined (AXM_0308C)
	#include "smac_MC9S08GB60.h"
#endif

#if defined (ARD) || defined (ARD2)
	#include "smac_MC9S08GT60.h"
#endif

#if defined (MC13192SARD) || defined (MC13192EVB)
	#include "smac_MC9S08GT60.h"
#endif

#if defined (SARD)
	#include "smac_MC9S08GT60.h"
#endif

#if defined (I_BOARD)
	#include "smac_MC9S08GT60.h"
#endif	