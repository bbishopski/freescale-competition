#ifndef __NET_H
#define __NET_H

#include "common_def.h"
#include "pub_def.h"

// Max size of our packets
#define MAX_PACKET_BUFFER 20
#define POWER_SETTING NOMINAL_POWER

// How long to delay waiting for acknowledgement 
#define ACK_DELAY_COUNT 0xB000
#define NO_ACK_DELAY    0


// Data structure for holding channels to scan and associated
// power levels during the scan
typedef struct {
  UINT8 channel;
  UINT8 power;
}t_powerScan;

typedef UINT8 t_NetTransNum; 
typedef UINT8 t_NetData;
typedef UINT8 t_NetMsgType;

#define NET_IDSTRING_STRLEN   3
#define NET_ID_STRING         "WAH"
#define MAX_NET_TRANSNUM      128
#define MAX_NET_DATA          3

enum {
  KEEPALIVE, WAH_ON, WAH_OFF, WAH_MVMT, WAH_ACK
};

typedef struct {
  UINT8         idString[NET_IDSTRING_STRLEN];
  t_NetMsgType  msgType;
  UINT8 netData[MAX_NET_DATA];
}t_NetPacket;


// Callback function from net to application
typedef void (*t_NetCallback) (t_NetPacket *data);

int sendRFMessage(t_NetMsgType msgType);
int sendRFPacket(t_NetPacket *packet);
int rcvRFData(t_NetCallback pCallback);
int stopReceive(void);
void selectNextRFChannel();
void setRFChannel();


#endif

