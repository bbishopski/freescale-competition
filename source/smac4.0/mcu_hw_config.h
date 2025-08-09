

/**************************************************************
*	Prototypes
**************************************************************/

void init_gpio(void);
void use_external_clock(void);
void use_mcu_clock(void);
void MC13192_restart(void);
void MC13192_cont_reset(void);
void mcu_init(void);


/**************************************************************
*	Externals
**************************************************************/
extern byte rtx_mode; /* Global mode mirror. */

/**************************************************************
*	Defines
**************************************************************/

#define MCU_LOW_POWER_WHILE _asm wait
