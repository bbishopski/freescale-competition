/**************************************************************
*	This is the SMAC C source driver file for the HC(S)08 MCU
*   and MC13192 transceiver.
*   The driver controls all interfaces to/from the MC13192 by the
*   SPI, selected interrupt source and MCU GPIO's assigned to
*   other MC13192 pins.
*   The SMAC driver is the lowest layer of C code.
*   Actual hardware interconnects from the MCU to the MC13192 are
*   defined in the MC13192_hw_config.h header file.
**************************************************************/


/**************************************************************
*	Includes
**************************************************************/
#include "device_header.h"
#include "pub_def.h"
#include "drivers.h"
#include "MC13192_regs.h"
#include "MC13192_hw_config.h"
#include "mcu_hw_config.h"
#include "simple_phy.h"

/**************************************************************
*	Defines
**************************************************************/
#define AssertCE        MC13192_CE = 0 /* Asserts the MC13192 CE pin */
#define DeAssertCE      MC13192_CE = 1 /* Deasserts the MC13192 CE pin */
#define SPIClkInvert      SPIC1 |= 0x04; /*Set CPHA bit of SPCR (clk polarity) */
#define SPIClkNormal      SPIC1 &= 0xFB; /*clr CPHA bit of SPCR (clk polarity) */

/**************************************************************
*	Globals
**************************************************************/
rx_packet_t *drv_rx_packet;
cca_measurement_t drv_cca_reading; 
__uint8__ irq_value = 0;
extern byte rtx_mode;

/**************************************************************
*	Interrupt: 	MC13192 initiated interrupt handler
*	Parameters: none
*	Return:		The interrupt will RTI unless valid data is recvd.
*				In this case a pd_data_indication function call-back
*				will be executed first, followed by a RTI
**************************************************************/
interrupt void irq_isr(void)
{
  /* The vector is defined in vectortable.c */
  volatile __uint16__ status_content, reg; /* Result of the status register read. */
  __uint8__ dataLength = 0;		/* Data length for the RX packet */
  CLEAR_IRQ_FLAG; /* Acknowledge the interrupt. MC13192 IRQ pin still low. */     
  status_content = drv_read_spi_1(STATUS_ADDR); /* Read the MC13192 status register. */
  /* Replace this with C code, if possible!!!!!!!!!!!!*/
  /* Test for IRQ glitch or possible fast double IRQ. */
  asm
  {
  BIH irq_high
  }
  /* IRQ is low. Re-read status. */
  CLEAR_IRQ_FLAG; /* Acknowledge the interrupt. MC13192 IRQ pin still low. */     
  reg = drv_read_spi_1(STATUS_ADDR); /* Read the MC13192 status register. */
  status_content = status_content | reg; /* Actual status is the OR of both. */
  asm
  {
  irq_high:
  }
  status_content &= TX_IRQ_MASK | RX_IRQ_MASK | ATTN_IRQ_MASK | CRC_VALID_MASK | TIMER1_IRQ_MASK | CCA_IRQ_MASK | LO_LOCK_IRQ_MASK;
  if (rtx_mode != RX_MODE_WTO)
  {
  /* If timeout not being used, mask out timer. */
		status_content &= ~(TIMER1_IRQ_MASK);
  }
  if ((status_content & ~(CRC_VALID_MASK)) == 0)
  {
  /* If no status bits are set just return. */
		return;
  }
  /* DOZE Complete Interrupt */
  if ((status_content & DOZE_IRQ_MASK) != 0)
  {
  /* Not implemented!!!!!!!!!!!!!! */
  }  
  /* ATTN IRQ Handler*/
  if ((status_content & ATTN_IRQ_MASK) != 0)
  {
  /* If attn interrupt, set the rtx_state mirror. */
  /* For MC13192 V2.x devices, read the reset indication in R25/7 first. */
  /* If a reset is indicated, call back to a reset handler. */
		status_content = drv_read_spi_1(RESIND_ADDR); /* Read the MC13192 reset indicator register. */
		status_content &= RESET_BIT_MASK;
		if (status_content == 0) /* Reset */
		{
			rtx_mode = MC13192_RESET_MODE; /* Set the rtx_state mirror to idle with attn. */
			PLME_MC13192_reset_indication();
			return;
		}
		else
		{
			rtx_mode = IDLE_MODE_ATTN; /* Set the rtx_state mirror to idle with attn. */
			return;
		}
  }
  /* TIMER1 IRQ Handler (Used for receiver timeout notification) */
  if ((status_content & TIMER1_IRQ_MASK) != 0) 
  {
  /* If a timeout (and timeout enabled) is done, update the mode mirror state. */  
	  	if (rtx_mode == RX_MODE_WTO)
	  	{
			/* Clear Timer1 if in RX_MODE_WTO */
			drv_write_spi_1(T1_HI_ADDR, 0x8000); /* Disables TC1 and clears the IRQ. */
			drv_write_spi_1(T1_LO_ADDR, 0x0000);
			drv_rx_packet->status = TIMEOUT;
	  		DeAssertRTXEN(); /* Forces the MC13192 to idle. */
			rtx_mode = IDLE_MODE;
			/* TIMEOUT notification */
			pd_data_indication();
			return;
		}
  }
  /* LO LOCK IRQ - Occurs when MC13192 loses LOCK */
  /* For receive cycles, re-enable. For transmit/CCA, abort. */
  if ((status_content & LO_LOCK_IRQ_MASK) != 0)
  {
  		DeAssertRTXEN(); /* Forces the MC13192 to idle. */
  		if ((rtx_mode == RX_MODE) || (rtx_mode == RX_MODE_WTO) || (rtx_mode == CCA_MODE)) /* Unlock from receive cycles */
  		{
			status_content = (drv_read_spi_1(MODE_ADDR) & 0xFF7F); /* Read the MC13192 trx register. Timer trigger off. */
			drv_write_spi_1(MODE_ADDR, status_content); /* Re-write the trx register. */  
	  		AssertRTXEN(); /* Re-start the sequence. */
  		}
  		else
  		{
  			rtx_mode = IDLE_MODE;
  		}
		return;
  }
  if ((rtx_mode == IDLE_MODE) || ((status_content & CCA_IRQ_MASK) != 0) || ((status_content & TX_IRQ_MASK) != 0))
  {
  /* If in idle mode already or if CCA or TX is done, just return. */
  		DeAssertRTXEN(); /* Forces the MC13192 to idle. */
		rtx_mode = IDLE_MODE;
		return;
  }
  /* If rx is done */
  if ((status_content & RX_IRQ_MASK) != 0)
  {  
		DeAssertRTXEN(); /* Forces the MC13192 to idle. */
	  	if ((status_content & CRC_VALID_MASK) == 0)
  		{
 		/* If an invalid CRC, restart receiver. */
			status_content = (drv_read_spi_1(MODE_ADDR) & 0xFF7F); /* Read the MC13192 trx register. Timer trigger off. */
			drv_write_spi_1(MODE_ADDR, status_content); /* Update the trx register. */  		
  			AssertRTXEN(); /* Forces the MC13192 to enter the receive mode. */
			return;
  		}
  		else
  		{
 	  		dataLength = (__uint8__) (drv_read_spi_1(RX_PKT_LEN) & 0x7F); /* Read received packet length register and mask off length bits */
	  		if (dataLength < 3) /* Rx_pkt_length is bad when 0, 1 or 2. */
	  		{
				status_content = (drv_read_spi_1(MODE_ADDR) & 0xFF7F); /* Read the MC13192 trx register. Timer trigger off. */
				drv_write_spi_1(MODE_ADDR, status_content); /* Update the trx register. */  		
				AssertRTXEN(); /* Forces the MC13192 to enter the receive mode. */
				return;
			}
			/* A valid packet has been received. */
			rtx_mode = IDLE_MODE;       /* set the rtx_state to idle */
			drv_write_spi_1(T1_HI_ADDR, 0x8000); /* Disables TC1 and clears the IRQ. */
			drv_write_spi_1(T1_LO_ADDR, 0x0000);
			drv_rx_packet->dataLength = dataLength;
			drv_rx_packet->status = SUCCESS;
	   		pd_data_indication();	/* Notify PHY that there is data available. */
			return;
		}
  }
}

/**************************************************************
*	Function: 	Wake the MC13192 from Hibernate/Doze mode
*	Parameters: none
*	Return:		
**************************************************************/
void Wake_MC13192 (void)
{
	MC13192_ATTN = 0; /* Assert ATTN */
	MC13192_ATTN = 1; /* Deassert ATTN */
}

/**************************************************************
*	Function: 	Deassert the MC13192 RTXEN pin (forces IC to idle)
*	Parameters: none
*	Return:		
**************************************************************/
void DeAssertRTXEN(void)
{
	MC13192_RTXEN = 0; /* Deassert RTXEN */
}

/**************************************************************
*	Function: 	Assert the MC13192 RTXEN pin (initiates programmed cycle)
*	Parameters: none
*	Return:		
**************************************************************/
void AssertRTXEN(void)
{
	MC13192_RTXEN = 1; /* Assert RTXEN */
}

/**************************************************************
*	Function: 	write 1 word to SPI 
*	Parameters: SPI address, the word
*	Return:		
**************************************************************/
void drv_write_spi_1(__uint8__ addr, __uint16__ content)
{
   __uint8__ temp_value; /* Used to flush the SPI1D register during read */
  temp_value = SPI1S; /* Clear status register (possible SPRF, SPTEF) */  
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */                       
  disable_MC13192_interrupts(); /* Necessary to prevent double SPI access */
  AssertCE; /* Enables MC13192 SPI */
  SPI1D = addr & 0x3F; /* Mask address, 6bit addr. Set write bit (i.e. 0). */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Now write content MSB */
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  SPI1D = content >> 8; /* Write MSB */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Now write content LSB */
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  SPI1D = content & 0x00FF; /* Write LSB */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Now read last of garbage */
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  DeAssertCE; /* Disables MC13192 SPI */
  restore_MC13192_interrupts(); /* Restore MC13192 interrupt status */
}

/**************************************************************
*	Function: 	read 1 word from SPI 
*	Parameters: SPI address
*	Return:		a word, w. w[0] is the MSB, w[1] is the LSB
**************************************************************/
__uint16__ drv_read_spi_1(__uint8__ addr)
{
  __uint16__  w; /* w[0] is MSB, w[1] is LSB */
  __uint8__ temp_value; /* Used to flush the SPI1D register during read */
  temp_value = SPI1S; /* Clear status register (possible SPRF, SPTEF) */  
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */                       
  disable_MC13192_interrupts(); /* Necessary to prevent double SPI access */
  AssertCE; /* Enables MC13192 SPI */
  SPI1D = (addr & 0x3f) | 0x80; /* Mask address, 6bit addr, Set read bit. */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set */
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  SPI1D = addr; /* Dummy write. Receive register of SPI will contain MSB */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Get MSB */
  ((__uint8__*)&w)[0] = SPI1D; /* MSB */
  SPI1D = addr; /* Dummy write. Waiting until after reading received data insures no overrun */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Get LSB */
  ((__uint8__*)&w)[1] = SPI1D; /* LSB */
  DeAssertCE; /* Disables MC13192 SPI */
  restore_MC13192_interrupts(); /* Restore MC13192 interrupt status */
  return w;
}

/**************************************************************
*	Parameters: None
*	Purpose: Waits until the SPI1D has been transferred and received
**************************************************************/
void WaitSPI_transfer_done(void)
{
  while (!(SPI1S_SPRF))
  {
  }
}

/**************************************************************
*	Function: 	disable MC13192 interrupts
*	Parameters: none
*	Return:		
**************************************************************/
void disable_MC13192_interrupts(void)
{
	irq_value = MC13192_IRQ_SOURCE;	/* Save the context of the MC13192_INT_REG (global) */
	MC13192_IRQ_SOURCE = irq_value & ~(0x06);  /* Disable the MC13192 interrupt source */
}

/**************************************************************
*	Function: 	restore MC13192 interrupts to previous condition
*	Parameters: none
*	Return:		
**************************************************************/
void restore_MC13192_interrupts(void)
{
	MC13192_IRQ_SOURCE = irq_value;	/* Restore the context of the IRQ register from global */
	
}


/**************************************************************
*	Function: write a block of data to TX packet RAM (whichever is selected)
*	Parameters: length		length of the block of data in bytes
*				*contents	pointer to the data block
**************************************************************/
void drv_write_tx_ram(tx_packet_t *tx_pkt)
{
  __uint8__ i, ibyte, temp_value; /* i, ibyte are counters. temp_value is used to flush the SPI1D register during read */
  __uint16__  reg; /* TX packet length register value */
  reg = drv_read_spi_1(TX_PKT_LEN); /* Read the TX packet length register contents */
  reg = (0xFF80 & reg) | (tx_pkt->dataLength + 2); /* Mask out old length setting and update. Add 2 for CRC */
  drv_write_spi_1(TX_PKT_LEN, reg); /* Update the TX packet length field */
  temp_value = SPI1S; /* Clear status register (possible SPRF, SPTEF) */  
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */                       
  disable_MC13192_interrupts(); /* Necessary to prevent double SPI access */
  AssertCE; /* Enables MC13192 SPI */
  SPI1D = TX_PKT; /* SPI TX ram data register */
  WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Now write content MSB */
  temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  ibyte = 0; /* Byte counter for *contents */
  for (i=0; i<((tx_pkt->dataLength+1) >> 1); i++) /* Word loop. Round up. */ 
  {
 		SPI1D = tx_pkt->data[ibyte + 1];	/* Write MSB */
  		WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Now write content LSB */
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
	  	SPI1D = tx_pkt->data[ibyte];	/* Write LSB */  		
 		ibyte=ibyte+2; /* Increment byte counter */
  		WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set.*/
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
  }
  DeAssertCE; /* Disables MC13192 SPI */
  restore_MC13192_interrupts(); /* Restore MC13192 interrupt status */
}

/**************************************************************
*	Function: read a block of data from RX packet RAM (whichever is selected)
*	Parameters: *length		returned length of the block of data in bytes
*				*contents	pointer to the data block storage
**************************************************************/
int drv_read_rx_ram(rx_packet_t *rx_pkt)
{
  __uint8__ i, ibyte, temp_value; /* i, ibyte are counters. temp_value is used to flush the SPI1D register during read */
  __uint8__  status=0; /* holder for the return value */
  __uint16__ rx_length;
  rx_length = drv_read_spi_1(RX_PKT_LEN); /* Read the RX packet length register contents */
  rx_length &= 0x007F; /* Mask out all but the RX packet length */
  /* MC13192 reports length with 2 CRC bytes, remove them. */
  /* ShortPacket is also checked in RX_ISR */
  if (rx_length >= 3)
  {
	  	rx_pkt->dataLength = rx_length - 2;
  }
  else
  {
  		rx_pkt->dataLength = 0;
  }	
  if ((rx_pkt->dataLength >= 1) && (rx_pkt->dataLength <= rx_pkt->maxDataLength)) /* If <3, the packet is garbage */
  {
  		temp_value = SPI1S; /* Clear status register (possible SPRF, SPTEF) */  
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */                       
  		disable_MC13192_interrupts(); /* Necessary to prevent double SPI access */
  		AssertCE; /* Enables MC13192 SPI */
	 	SPI1D = RX_PKT | 0x80; /* SPI RX ram data register */
  		WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set.*/
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
		SPI1D = temp_value; /* Dummy write. Receive register of SPI will contain MSB garbage for first read */
		WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set.*/
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
		SPI1D = temp_value; /* Dummy write. Receive register of SPI will contain LSB garbage for first read */
		WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set.*/
  		temp_value = SPI1D; /* Clear receive data register. SPI entirely ready for read or write */
		ibyte = 0; /* Byte counter for *contents */
		for (i=0; i<((rx_length-1)>>1); i++) /* Word loop. Round up. Deduct CRC. */
		{
			SPI1D = temp_value; /* Dummy write. Receive register of SPI will contain MSB */
			WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Get MSB */
			if ((ibyte+3)==rx_length) /* For a trailing garbage byte, just read and discard */
			{
				temp_value = SPI1D; /* Discard */
			}
			else
			{
				rx_pkt->data[ibyte+1] = SPI1D; /* Read MSB */
			}
			SPI1D = temp_value; /* Dummy write. Receive register of SPI will contain LSB */
			WaitSPI_transfer_done(); /* For this bit to be set, SPTED MUST be set. Get LSB */
			rx_pkt->data[ibyte] = SPI1D; /* Read LSB */
	 		ibyte=ibyte+2; /* Increment byte counter */
	 	}
  		DeAssertCE; /* Disables MC13192 SPI */
  		rx_pkt->status = SUCCESS;
  		restore_MC13192_interrupts(); /* Restore MC13192 interrupt status */
  }
	/* Check to see if a larger packet than desired is received. */  
  if (rx_pkt->dataLength > rx_pkt->maxDataLength)
  rx_pkt->status = OVERFLOW;
  return status;  
}

