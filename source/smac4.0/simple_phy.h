/**************************************************************
*	Includes
**************************************************************/
#include "pub_def.h"

/**************************************************************
*	Prototypes
*   See simple_phy.c for a complete description.
**************************************************************/
int pd_data_request(tx_packet_t *);
void pd_data_indication(void);
int PLME_hibernate_request(void);
int PLME_doze_request(void);
int PLME_doze_request_wClk(int acomaMode);
int PLME_wake_request(void);
int PLME_set_channel_request(__uint8__);
int PLME_set_trx_state_request(__uint8__);
__uint8__ PLME_energy_detect (void);
__uint8__ PLME_link_quality (void);
void PLME_get_time_request(__uint32__ *time);
int PLME_set_MC13192_clock_rate(__uint8__);
int PLME_set_MC13192_tmr_prescale (__uint8__); 
void PLME_set_time_request(__uint32__);
int PLME_enable_MC13192_timer1(__uint32__);
int PLME_disable_MC13192_timer1(void);
void PLME_MC13192_reset_indication(void);
int PLME_MC13192_soft_reset(void);
int PLME_MC13192_xtal_adjust(__int8__);
int PLME_MC13192_FE_gain_adjust(__int8__);
int PLME_MC13192_PA_output_adjust(__uint8__);
__uint8__ PLME_get_rfic_version(void);