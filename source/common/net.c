/****************************************************************************
* net.c
* 
* Author: Bill Bishop - Sixth Sensor
* Title: 	net.c
* 
* This file contains procedures to abstract the layer3 signalling for the
* remote wah pedal system.
*
****************************************************************************/

#include "simple_mac.h"
#include "MC13192_hw_config.h"
#include "SCI.h"
#include "drivers.h"
#include "sard_board.h"
#include "HAL.h"
#include "net.h"

// Possible channels to communicate on
static t_powerScan scanChannels[]={ {0,POWER_SETTING},{5,POWER_SETTING},
  {10,POWER_SETTING},{15,POWER_SETTING}};
static UINT8 numScanChannels=sizeof(scanChannels)/sizeof(scanChannels[0]);

// Index into scanChannels, determines current channel
static UINT8 scanChannelIndex=0;
static t_NetTransNum transNum=0;


// Header, not to be confused with other SMAC devices
static t_NetPacket netPacket={NET_ID_STRING, 0, 0};

static t_NetCallback appCallback=NULL;

// Data structures for communicating with Simple MAC layer
static rx_packet_t rxPacket;
static tx_packet_t txPacket;
static byte rxDataBuffer[MAX_PACKET_BUFFER];

// Prototypes
t_NetTransNum getNexTransNum(void);

/****************************************************************************
 * stopReceive
 *
 * Description: Cancels outstanding receives,
 *
 * Parms:       none
 *
 * Returns:     0 if success, 1 if error
 ***************************************************************************/
int stopReceive(void)
{
  int retcode = 0;

  // bring 13192 into idle mode
  if (MLME_RX_disable_request() == ERROR) {
    retcode = 1;  
  }

  return retcode;
}

/****************************************************************************
 * rcvRFData
 *
 * Description: Application calls this to be notified when packets arrive.
 *              The callback function is used to notify the application when 
 *              a packet is received.  This net layer only supports one 
 *              outstanding transaction at a time.  
 *
 * Parms:       pCallback - function pointer to be called when packet received.
 *
 * Returns:     0 if success, 1 if error
 ***************************************************************************/
int rcvRFData(t_NetCallback pCallback)
{
  if (pCallback != NULL) {
    appCallback = pCallback;
  }

  // Setup the receive packet
  rxPacket.dataLength = 0;
  rxPacket.data = &rxDataBuffer[0];
  rxPacket.maxDataLength = sizeof(t_NetPacket);
  rxPacket.status = 0;

  // return immediately, no delay 
  MLME_RX_enable_request(&rxPacket, NO_ACK_DELAY);          
}

/****************************************************************************
 * MCPS_data_indication
 *
 * Description: SMAC (layer-2) will call this when packet arrives. This proc
 *              will then check the contents of the packet.  Only if valid
 *              packet will application be called.
 *
 * Parms:       rx_packet - pointer to received packet.
 *
 * Returns:     nothing
 ***************************************************************************/
void MCPS_data_indication(rx_packet_t *rx_packet) 
{
  t_NetPacket *pPacket;

  // Get pointer to data  
  pPacket = (t_NetPacket *)rx_packet->data; 

  if (rx_packet->status == SUCCESS) {
    // Packet received, see if id string matches
#ifndef MVMT_DEBUG
    if (memcmp(NET_ID_STRING, pPacket->idString, NET_IDSTRING_STRLEN) == 0)
#endif // all packets valid if debugging
    {
      // valid packet
      if (appCallback != NULL) {
        appCallback(pPacket);
      }

    }// if good ack
  }
}

/****************************************************************************
 * MLME_MC13192_reset_indication
 *
 * Description: resets 13192
 *
 * Parms:       nothing
 *
 * Returns:     nothing
 ***************************************************************************/
void MLME_MC13192_reset_indication() 
{
  // Notifies you that the MC13192 has been reset.
  // Application must handle this here.
  //
  // do nothing.
}


/****************************************************************************
 * selectNextRFChannel
 *
 * Description: Selects next RF Channel
 *
 * Parms:       nothing
 *
 * Returns:     nothing
 ***************************************************************************/
void selectNextRFChannel()
{
  if (++scanChannelIndex >= numScanChannels)
    scanChannelIndex=0;

  setRFChannel();
}

/****************************************************************************
 * setRFChannel
 *
 * Description: sets next RF Channel
 *
 * Parms:       nothing
 *
 * Returns:     nothing
 ***************************************************************************/
void setRFChannel()
{

  // Set tx channel  
  MLME_set_channel_request(scanChannels[scanChannelIndex].channel);

  // Set MAX power setting
  MLME_MC13192_PA_output_adjust(scanChannels[scanChannelIndex].power);
}

/****************************************************************************
 * setRFChannel
 *
 * Description: Sends a complete packet to receiver.  SMAC will do a
 *              low power while during the transmission
 *
 * Parms:       packet - pointer to outgoing layer 3 packet
 *
 * Returns:     1 if success, 0 if fail
 ***************************************************************************/
int sendRFPacket(t_NetPacket *packet)
{
  int retcode = TRUE;

  // Setup packet to be passed to SMAC
  txPacket.data = packet; 
  txPacket.dataLength = sizeof(t_NetPacket);

  // Send the packet
  if (MCPS_data_request(&txPacket) != SUCCESS) {
    // problem occured
    retcode = FALSE;
  }

  return retcode;
}

/****************************************************************************
 * sendRFMessage
 *
 * Description: Sends a status message to receiver.  Returns immediately and 
 *             leaves the RF receiver on, waiting for packet.  It is the 
 *             responsibility of the application to monitor received packets
 *             and to handle timing out.  The application MUST NOT go into 
 *             sleep mode until a packet is received (or timeout).  Otherwise 
 *             the return packet may be lost.
 *
 * Parms:       msgType - layer 3 message type
 *
 * Returns:     1 if success, 0 if fail
 ***************************************************************************/
int sendRFMessage(t_NetMsgType msgType)
{
  int retcode = TRUE;

  // Setup the data packet
  // NOTE: header.idString is already setup for speed
  netPacket.msgType = msgType;

  // Initialize data section
  //memset(netPacket.data, 0, sizeof(netPacket.data));

  retcode = sendRFPacket(&netPacket);  
  return retcode;
}

