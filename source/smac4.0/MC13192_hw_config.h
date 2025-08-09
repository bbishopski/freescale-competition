/**************************************************************
*	This is the SMAC C source driver header file for the
*   MC13192 transceiver hardware interconnections.
*   The header defines all prototypes and MC13192
*   interconnections used by the actual C driver.
*   Initialization is accomplished by the MC13192_hw_config.c
*   source file.
**************************************************************/


/**************************************************************
*	Includes
**************************************************************/
#include "device_header.h"
#include "pub_def.h"
//
/**************************************************************
*	Defines
*   MC13192 to MCU interconnects (hardware SPI assumed).
**************************************************************/
/* Daughter card settings. */

#if defined (AXM_0308) || defined (AXM_0308A) || defined (AXM_0308B) || defined (AXM_0308C)
	#define MC13192_CE				PTED_PTED2				
	#define MC13192_CE_PORT			PTEDD_PTEDD2				
	#define MC13192_ATTN				PTDD_PTDD5
	#define MC13192_ATTN_PORT		PTDDD_PTDDD5
	#define MC13192_RTXEN			PTDD_PTDD6
	#define MC13192_RTXEN_PORT		PTDDD_PTDDD6
	#define MC13192_RESET			PTDD_PTDD7
	#define MC13192_RESET_PORT		PTDDD_PTDDD7
	#define MC13192_RESET_PULLUP 	PTDPE_PTDPE7
	#define MC13192_IRQ_SOURCE		IRQSC
	#define MC13192_IRQ_IE_BIT		IRQSC_IRQIE
#endif

/* ARD settings. */
#if defined (ARD) || defined (ARD2)
	#define MC13192_CE				PTED_PTED2				
	#define MC13192_CE_PORT			PTEDD_PTEDD2				
	#define MC13192_ATTN				PTDD_PTDD3
	#define MC13192_ATTN_PORT		PTDDD_PTDDD3
	#define MC13192_RTXEN			PTAD_PTAD5
	#define MC13192_RTXEN_PORT		PTADD_PTADD5
	#define MC13192_RESET			PTAD_PTAD6
	#define MC13192_RESET_PORT		PTADD_PTADD6
	#define MC13192_RESET_PULLUP 	PTAPE_PTAPE6
	#define MC13192_IRQ_SOURCE		IRQSC
	#define MC13192_IRQ_IE_BIT		IRQSC_IRQIE
#endif

#if defined (MC13192SARD) || defined (MC13192EVB)
	#define MC13192_CE				    PTED_PTED2				
	#define MC13192_CE_PORT			  PTEDD_PTEDD2				
	#define MC13192_ATTN			    PTCD_PTCD2
	#define MC13192_ATTN_PORT		  PTCDD_PTCDD2
	#define MC13192_RTXEN			    PTCD_PTCD3
	#define MC13192_RTXEN_PORT		PTCDD_PTCDD3
	#define MC13192_RESET			    PTCD_PTCD4
	#define MC13192_RESET_PORT		PTCDD_PTCDD4
	#define MC13192_RESET_PULLUP 	PTCPE_PTCPE4
	#define MC13192_IRQ_SOURCE		IRQSC
	#define MC13192_IRQ_IE_BIT		IRQSC_IRQIE
	
	#ifdef ANTENNA_SWITCH
	  #define MC13192_ANT_CTRL			PTBD_PTBD6
	  #define MC13192_ANT_CTRL2			PTBD_PTBD6  //Second Control line not used in MC13192EVB
	  #define MC13192_ANT_CTRL_PORT		PTBDD_PTBDD6
	  #define MC13192_ANT_CTRL2_PORT		PTBDD_PTBDD6 //Second Control line not used in MC13192EVB
    #define ANT_CTRL_OFF			    0   //Logic low is off
    #define ANT_CTRL_ON			      1   //Logic high is on
  #endif ANTENNA_SWITCH
	
	#ifdef LNA
	  #define MC13192_LNA_CTRL      PTBD_PTBD0
	  #define MC13192_LNA_CTRL_PORT PTBDD_PTBDD0
	  #define LNA_ON                1
	  #define LNA_OFF               0
	#endif LNA
	
#endif

/* I board settings. */
#if defined (I_BOARD)
	#define MC13192_CE						PTED_PTED2		
	#define MC13192_CE_PORT				PTEDD_PTEDD2				
	#define MC13192_ATTN					PTAD_PTAD3
	#define MC13192_ATTN_PORT			PTADD_PTADD3
	#define MC13192_RTXEN					PTCD_PTCD2
	#define MC13192_RTXEN_PORT		PTCDD_PTCDD2
	#define MC13192_RESET					PTAD_PTAD2
	#define MC13192_RESET_PORT		PTADD_PTADD2
	#define MC13192_RESET_PULLUP 	PTAPE_PTAPE0
	#define MC13192_IRQ_SOURCE		IRQSC
	#define MC13192_IRQ_IE_BIT		IRQSC_IRQIE
	#define MC13192_ANT_CTRL			PTCD_PTCD5
	#define MC13192_ANT_CTRL2			PTCD_PTCD4
	#define MC13192_ANT_CTRL_PORT		PTCDD_PTCDD5
	#define MC13192_ANT_CTRL2_PORT	PTCDD_PTCDD4
	#define ANT_CTRL_OFF			      0
  #define ANT_CTRL_ON			        1
  
  #if defined (PA)
    #define MC13192_PA_CTRL_PORT  PTEDD_PTEDD1
    #define MC13192_PA_CTRL       PTED_PTED1
    #define PA_ON                 1
    #define PA_OFF                0
  #endif
  
#endif

/**************************************************************
*	Externals
**************************************************************/
extern byte rtx_mode; /* Global mode mirror. */

/**************************************************************
*	Prototypes
*   See drivers.c for a complete description.
**************************************************************/
void MC13192_init(void);




