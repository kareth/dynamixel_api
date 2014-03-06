#include "serial.h"
#include "../include/dynamixel.h"
#include <string>
#include "instruction_packet.h"
#include <cstdio>

#define ID					(2)
#define LENGTH				(3)
#define INSTRUCTION			(4)
#define ERRBIT				(4)
#define PARAMETER			(5)
#define DEFAULT_BAUDNUMBER	(1)

namespace dynamixel {

//unsigned char gbInstructionPacket[MAXNUM_TXPARAM+10] = {0};
unsigned char gbStatusPacket[MAXNUM_RXPARAM+10] = {0};
unsigned char gbRxPacketLength = 0;
unsigned char gbRxGetLength = 0;
int gbCommStatus = COMM_RXSUCCESS;
int giBusUsing = 0;

Serial* serial;
InstructionPacket iPacket;

int Dynamixel::dxl_initialize(int deviceIndex, int baudrate) {
  serial = new Serial();

  if (serial->Open("/dev/ttymxc4", baudrate) == -1)
    return 0;

  gbCommStatus = COMM_RXSUCCESS;
  giBusUsing = 0;
  return 1;
}

void Dynamixel::dxl_terminate(void) {
  delete serial;
}

// Attempts to read a packet from dynamixel
void Dynamixel::dxl_tx_packet(void) {
  unsigned char i;
  unsigned char TxNumByte, RealTxNumByte;
  unsigned char checksum = 0;

  if (giBusUsing == 1)
    return;

  giBusUsing = 1;

  if (iPacket.length() > (MAXNUM_TXPARAM+2)) {
    gbCommStatus = COMM_TXERROR;
    giBusUsing = 0;
    return;
  }

  if (iPacket.instruction() != INST_PING
      && iPacket.instruction() != INST_READ
      && iPacket.instruction() != INST_WRITE
      && iPacket.instruction() != INST_REG_WRITE
      && iPacket.instruction() != INST_ACTION
      && iPacket.instruction() != INST_RESET
      && iPacket.instruction() != INST_SYNC_WRITE) {
    gbCommStatus = COMM_TXERROR;
    giBusUsing = 0;
    return;
  }

  // TODO necessary?
  iPacket.clear();
  //gbInstructionPacket[0] = 0xff;
  //gbInstructionPacket[1] = 0xff;

  iPacket.GenerateChecksum();

  /*for (i = 0; i < (gbInstructionPacket[LENGTH]+1); i++)
    checksum += gbInstructionPacket[i+2];

  gbInstructionPacket[gbInstructionPacket[LENGTH]+3] = ~checksum;*/

  if (gbCommStatus == COMM_RXTIMEOUT || gbCommStatus == COMM_RXCORRUPT)
    serial->Flush();

  TxNumByte = iPacket.length() + 4;
  //RealTxNumByte = serial->Write((unsigned char*)gbInstructionPacket, TxNumByte);
  RealTxNumByte = serial->Write((unsigned char*)iPacket.data(), TxNumByte);

  if (TxNumByte != RealTxNumByte) {
    gbCommStatus = COMM_TXFAIL;
    giBusUsing = 0;
    return;
  }

  //if (gbInstructionPacket[INSTRUCTION] == INST_READ)
  if (iPacket.instruction() == INST_READ)
    serial->SetTimeout(iPacket.parameter(1) + 6);
  else
    serial->SetTimeout(6);

  gbCommStatus = COMM_TXSUCCESS;
}

void Dynamixel::dxl_rx_packet(void) {
  unsigned char i, j, nRead;
  unsigned char checksum = 0;

  if( giBusUsing == 0 )
    return;

  if (iPacket.id() == BROADCAST_ID) {
    gbCommStatus = COMM_RXSUCCESS;
    giBusUsing = 0;
    return;
  }

  if (gbCommStatus == COMM_TXSUCCESS) {
    gbRxGetLength = 0;
    gbRxPacketLength = 6;
  }

  nRead = serial->Read((unsigned char*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength);
  gbRxGetLength += nRead;
  if(gbRxGetLength < gbRxPacketLength) {
    if (serial->Timeout() == 1) {
      if (gbRxGetLength == 0)
        gbCommStatus = COMM_RXTIMEOUT;
      else
        gbCommStatus = COMM_RXCORRUPT;
      giBusUsing = 0;
      return;
    }
  }

  // Find packet header
  for (i = 0; i < (gbRxGetLength - 1); i++) {
    if (gbStatusPacket[i] == 0xff && gbStatusPacket[i+1] == 0xff) {
      break;
    } else if (i == gbRxGetLength-2 && gbStatusPacket[gbRxGetLength-1] == 0xff) {
      break;
    }
  }
  if (i > 0) {
    for (j = 0; j < (gbRxGetLength - i); j++)
      gbStatusPacket[j] = gbStatusPacket[j + i];
    gbRxGetLength -= i;
  }

  if (gbRxGetLength < gbRxPacketLength) {
    gbCommStatus = COMM_RXWAITING;
    return;
  }

  // Check id pairing
  if (iPacket.id() != gbStatusPacket[ID]) {
    gbCommStatus = COMM_RXCORRUPT;
    giBusUsing = 0;
    return;
  }

  gbRxPacketLength = gbStatusPacket[LENGTH] + 4;
  if (gbRxGetLength < gbRxPacketLength) {
    nRead = serial->Read((unsigned char*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength);
    gbRxGetLength += nRead;
    if (gbRxGetLength < gbRxPacketLength) {
      gbCommStatus = COMM_RXWAITING;
      return;
    }
  }

  // Check checksum
  for (i = 0; i < (gbStatusPacket[LENGTH]+1); i++)
    checksum += gbStatusPacket[i+2];
  checksum = ~checksum;

  if (gbStatusPacket[gbStatusPacket[LENGTH]+3] != checksum) {
    gbCommStatus = COMM_RXCORRUPT;
    giBusUsing = 0;
    return;
  }

  gbCommStatus = COMM_RXSUCCESS;
  giBusUsing = 0;
}

void Dynamixel::dxl_txrx_packet(void) {
  dxl_tx_packet();

  if (gbCommStatus != COMM_TXSUCCESS)
    return;

  do {
    dxl_rx_packet();
  } while (gbCommStatus == COMM_RXWAITING);
}

int Dynamixel::dxl_get_result(void) {
  return gbCommStatus;
}

/*
void Dynamixel::dxl_set_txpacket_id(int id) {
  iPacket.set_id(id);
}

void Dynamixel::dxl_set_txpacket_instruction(int instruction) {
  iPacket.set_instruction(instruction);
}

void Dynamixel::dxl_set_txpacket_parameter(int index, int value) {
  iPacket.set_parameter(index, value);
}

void Dynamixel::dxl_set_txpacket_length(int length) {
  iPacket.set_length(length);
}
*/
// STATUS
int Dynamixel::dxl_get_rxpacket_error(int errbit) {
  if( gbStatusPacket[ERRBIT] & (unsigned char)errbit )
    return 1;

  return 0;
}

// STATUS
int Dynamixel::dxl_get_rxpacket_length(void) {
  return (int)gbStatusPacket[LENGTH];
}

// STATUS
int Dynamixel::dxl_get_rxpacket_parameter(int index) {
  return (int)gbStatusPacket[PARAMETER+index];
}

// Calculates value based on lower and higher 8 bits
int Dynamixel::dxl_makeword(int lowbyte, int highbyte) {
  unsigned short word;

  word = highbyte;
  word = word << 8;
  word = word + lowbyte;
  return (int)word;
}

// Transforms value to lower 8 bits and higher
int Dynamixel::dxl_get_lowbyte(int word) {
  unsigned short temp;

  temp = word & 0xff;
  return (int)temp;
}

// Transforms value to lower 8 bits and higher
int Dynamixel::dxl_get_highbyte(int word) {
  unsigned short temp;

  temp = word & 0xff00;
  temp = temp >> 8;
  return (int)temp;
}

// Pings certain ID
void Dynamixel::dxl_ping(int id) {
  while(giBusUsing);
  iPacket.set_attr(id, 2, INST_PING);
  dxl_txrx_packet();
}


// Reads one byte
int Dynamixel::dxl_read_byte(int id, int address) {
  while(giBusUsing);


  iPacket.set_attr(id, 4, INST_READ);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, 1);
  dxl_txrx_packet();

  return (int)gbStatusPacket[PARAMETER];
}

// Writes one byte
void Dynamixel::dxl_write_byte(int id, int address, int value) {
  while(giBusUsing);

  iPacket.set_attr(id, 4, INST_WRITE);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, value);
  dxl_txrx_packet();
}

// Reads 2 bytes
int Dynamixel::dxl_read_word(int id, int address) {
  while(giBusUsing);
  iPacket.set_attr(id, 4, INST_READ);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, 2);
  dxl_txrx_packet();

  return dxl_makeword((int)gbStatusPacket[PARAMETER], (int)gbStatusPacket[PARAMETER+1]);
}

// Writes 2 bytes
void Dynamixel::dxl_write_word(int id, int address, int value) {
  while(giBusUsing);
  iPacket.set_attr(id, 5, INST_WRITE);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, dxl_get_lowbyte(value));
  iPacket.set_parameter(2, dxl_get_highbyte(value));
  dxl_txrx_packet();
}

}  // namespace dynamixel
