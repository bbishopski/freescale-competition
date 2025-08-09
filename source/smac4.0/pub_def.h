
#ifndef __PUB_DEFINE__
#define __PUB_DEFINE__
/*--------------Test only.---------------------------*/
//#define inline



typedef unsigned char   __uint8__;
typedef unsigned char   __byte__;
typedef signed char		__int8__;
typedef unsigned short  __uint16__;
typedef signed short    __int16__;
typedef unsigned long   __uint32__;
typedef signed long     __int32__;

/* Modes defined for the transciever */
#define IDLE_MODE						0x00

#define RX_MODE							0x02
#define TX_MODE 						0x03
#define CCA_MODE						0x01
#define RX_MODE_WTO					0x80
#define HIBERNATE_MODE			0x81
#define DOZE_MODE						0x82
#define IDLE_MODE_DOZE			0x83
#define IDLE_MODE_ATTN			0x84
#define TIMEOUT							0x85
#define SYSTEM_RESET_MODE		0x86	/* Not implemented */
#define MC13192_RESET_MODE	0x87	/* Not implemented */
#define MC13192_CONFIG_MODE	0x88	/* Not implemented */
#define RESET_DELAY					0x89	/* Set to 100ms	*/

#define TRUE  1
#define FALSE 0

#define MAXPACKETSIZE 		126

#define MAX_POWER 100	/* Numbers chosen arbitrarily but > 16 */
#define NOMINAL_POWER 0x0B
#define MIN_POWER 50	/* Numbers chosen arbitrarily but > 16 */

/* Status enumations for the PHY. */
#define SUCCESS 0x77
#define INITIAL_VALUE 0x0
enum pd_data_status {RX_ON = 1, TRX_OFF};
enum mc13192_power_modes {RF_POWER_ON = 1, RF_POWER_HIBERNATE, RF_POWER_DOZE};
enum PLME_set_trx_state_request {ERROR = 1};
#define OVERFLOW 1

typedef struct 
{
	__uint8__ maxDataLength;
	__uint8__ dataLength;
	__uint8__ *data;
	__uint8__ status;

} rx_packet_t;

typedef struct
{
	__uint8__ dataLength;
	__uint8__ *data;

} tx_packet_t;

typedef struct
{	
	__uint8__ cca_mode;
	__uint8__ instant_value;
	__uint8__ final_value;	
} cca_measurement_t;

#ifndef NULL
#define NULL			((void *)0)
#endif

#endif