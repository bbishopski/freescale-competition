/**************************************************************
*	This is the SMAC C source driver header file for the HCS08 MCU
*   and MC13192 transceiver.
*   The header defines all externals, prototypes and MC13192
*   status mask bits used by the actual C driver.
**************************************************************/


/**************************************************************
*	Includes
**************************************************************/
#include "device_header.h"
#include "pub_def.h"

/**************************************************************
*	Externals
**************************************************************/
extern byte rtx_mode; /* Global mode mirror. */

/**************************************************************
*	Defines
*   MC13192 status register interrupt masks.
**************************************************************/
#define CLEAR_IRQ_FLAG			IRQSC_IRQACK = 1 /* Dependent upon interrupt source chosen. */
#define TX_IRQ_MASK				0x0040
#define RAMERR_IRQ_MASK			0x4000
#define RX_IRQ_MASK				0x0080
#define ATTN_IRQ_MASK			0x0400
#define CRC_VALID_MASK			0x0001
#define TIMER1_IRQ_MASK			0x0100
#define XCVR_SEQ_MASK			0xFFF8
#define LO_LOCK_IRQ_MASK		0x8000
#define CCA_IRQ_MASK			0x0020
#define DOZE_IRQ_MASK			0x0200
#define RESET_BIT_MASK			0x0080

/**************************************************************
*	Prototypes
*   See drivers.c for a complete description.
**************************************************************/
void WaitSPI_transfer_done(void);
void disable_MC13192_interrupts(void);
void restore_MC13192_interrupts(void);
void Wake_MC13192 (void);
void DeAssertRTXEN(void);
void AssertRTXEN(void);
void drv_write_spi_1(__uint8__, __uint16__);
__uint16__ drv_read_spi_1(__uint8__);
void drv_write_tx_ram(tx_packet_t *);
int drv_read_rx_ram(rx_packet_t *);

