/**************************************************************
*	Includes
**************************************************************/
#include "pub_def.h"

/**************************************************************
*	Prototypes
*   See simple_phy.c for a complete description.
**************************************************************/
int MCPS_data_request(tx_packet_t *);
int MLME_hibernate_request(void);
int MLME_wake_request(void);
int MLME_set_channel_request(__uint8__);
int MLME_RX_enable_request(rx_packet_t *, __uint32__);
int MLME_RX_disable_request(void);
int MLME_set_MC13192_clock_rate(__uint8__);
__uint8__ MLME_energy_detect(void);
int MLME_MC13192_soft_reset(void);
int MLME_MC13192_xtal_adjust(__uint8__);
__uint8__ MLME_link_quality (void);
int MLME_set_MC13192_tmr_prescale (__uint8__);
int MLME_MC13192_FE_gain_adjust(__uint8__);
int MLME_doze_request(void);
int MLME_doze_request_wClk(int acomaMode);
int MLME_MC13192_PA_output_adjust(__uint8__);
__uint8__ MLME_get_rfic_version(void);
