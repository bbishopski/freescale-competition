/**************************************************************
*	This is the SMAC C source driver header file for
*   the MC13192 transceiver register map.
**************************************************************/


/******** MC13192 soft reset **********/
#define RESET						0x00

/******** Packet RAM **********/
#define RX_PKT						0x01	/* RX Packet RAM */
#define RX_PKT_LEN					0x2D	/* RX Packet RAM Length [6:0] */
#define TX_PKT						0x02	/* TX Packet RAM */
#define TX_PKT_LEN					0x03	/* TX Packet RAM Length */

/******** IRQ Status Register *******/
#define IRQ_MASK					0x05
#define STATUS_ADDR					0x24
#define RESIND_ADDR					0x25
#define TIMER1_IRQMASK_BIT			0x0001

/******** Mask and mode **********/
#define MODE_ADDR					0x06	 
#define MODE2_ADDR					0x07	 
#define LO1_COURSE_TUNE				0x8000

/******** Main Timer **********/
#define TIMER_PRESCALE				0x09
#define TIMESTAMP_HI_ADDR			0x26
#define TIMESTAMP_LO_ADDR			0x27
#define TIMESTAMP_HI_MASK			0x00FF

/******** frequency ***************/
#define XTAL_ADJ_ADDR				0x0A
#define CLKS_ADDR 					0x0A
#define LO1_IDIV_ADDR				0x0F
#define LO1_NUM_ADDR				0x10
#define PRESCALE_ADDR 				0x09

/******** Timer comparators **********/
#define T1_HI_ADDR					0x1B	
#define T1_LO_ADDR					0x1C

/******** CCA **********/
#define CCA_THRESHOLD				0x04
#define CCA_RESULT_ADDR				0x2D
#define FEGAIN_ADDR 				0x04

/******** TX ***********/
#define PA_ADJUST_ADDR				0x12

/******* GPIO **********/
#define GPIO_CONFIG					0x0B
#define GPIO_DATA						0x0C
#define GPIO_DATA_MASK				0x003F

/******* version *******/
#define VERSION_REG					0x2C
#define VERSION_MASK					0x1C00