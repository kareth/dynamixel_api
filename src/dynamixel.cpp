#include "serial.h"
#include "../include/dynamixel.h"
#include <string>
#include "instruction_packet.h"
#include "status_packet.h"
#include <cstdio>

#define ID					(2)
#define LENGTH				(3)
#define INSTRUCTION			(4)
#define ERRBIT				(4)
#define PARAMETER			(5)
#define DEFAULT_BAUDNUMBER	(1)

namespace dynamixel {

//unsigned char gbInstructionPacket[MAXNUM_TXPARAM+10] = {0};
int giBusUsing = 0;

Serial* serial;
InstructionPacket iPacket;
StatusPacket sPacket;

Dynamixel::Dynamixel() : status_(COMM_RXSUCCESS) {
}

int Dynamixel::dxl_initialize(int deviceIndex, int baudrate) {
  serial = new Serial();

  if (serial->Open("/dev/ttymxc4", baudrate) == -1)
    return 0;

  status_ = COMM_RXSUCCESS;
  giBusUsing = 0;
  return 1;
}

void Dynamixel::dxl_terminate(void) {
  delete serial;
}

void Dynamixel::Error(int code) {
  status_ = code;
  giBusUsing = 0;
}

void Dynamixel::WaitForBus() {
  // TODO actually bus is useless...
  while (giBusUsing == 1);
  giBusUsing = 1;
}

// Attempts to read a packet from dynamixel
void Dynamixel::dxl_tx_packet() {
  WaitForBus();

  if (!iPacket.IsValid()) {
    Error(COMM_TXERROR);
    return;
  }

  iPacket.GenerateChecksum();

  if (status_ == COMM_RXTIMEOUT || status_ == COMM_RXCORRUPT)
    serial->Flush();

  int written_bytes = serial->Write(iPacket.data(), iPacket.PacketLength());

  if (iPacket.PacketLength() != written_bytes) {
    // TODO errors
    Error(COMM_TXFAIL);
    return;
  }

  if (iPacket.instruction() == INST_READ)
    serial->SetTimeout(iPacket.parameter(1) + 6);
  else
    serial->SetTimeout(6);

  status_ = COMM_TXSUCCESS;
}

void Dynamixel::dxl_rx_packet(void) {
  unsigned char i, j;
  unsigned char checksum = 0;

  if (giBusUsing == 0)
    return;

  if (iPacket.id() == BROADCAST_ID) {
    status_ = COMM_RXSUCCESS;
    giBusUsing = 0;
    return;
  }

  // TODO what else?
  if (status_ == COMM_TXSUCCESS) {
    sPacket.Clear();
    sPacket.set_expected_length(6);
  }

  int read_bytes = serial->Read(sPacket.ReadPosition(), sPacket.RemainingLength());
  sPacket.Read(read_bytes);

  if (sPacket.RemainingLength() > 0) {
    if (serial->Timeout() == 1) {
      if (sPacket.read_length() == 0)
        status_ = COMM_RXTIMEOUT;
      else
        status_ = COMM_RXCORRUPT;
      giBusUsing = 0;
      return;
    }
  }

  sPacket.ShiftHeader();

  if (sPacket.RemainingLength() > 0) {
    status_ = COMM_RXWAITING;
    return;
  }

  // Check id pairing
  if (iPacket.id() != sPacket.data()[ID]) {
    status_ = COMM_RXCORRUPT;
    giBusUsing = 0;
    return;
  }

  sPacket.set_expected_length(sPacket.data()[LENGTH] + 4);
  if (sPacket.RemainingLength() > 0) {
    read_bytes = serial->Read(sPacket.ReadPosition(), sPacket.RemainingLength());
    sPacket.Read(read_bytes);

    if (sPacket.RemainingLength() > 0) {
      status_ = COMM_RXWAITING;
      return;
    }
  }

  // Check checksum
  for (i = 0; i < (sPacket.data()[LENGTH]+1); i++)
    checksum += sPacket.data()[i+2];
  checksum = ~checksum;

  if (sPacket.data()[sPacket.data()[LENGTH]+3] != checksum) {
    status_ = COMM_RXCORRUPT;
    giBusUsing = 0;
    return;
  }

  status_ = COMM_RXSUCCESS;
  giBusUsing = 0;
}

void Dynamixel::dxl_txrx_packet(void) {
  dxl_tx_packet();

  if (status_ != COMM_TXSUCCESS)
    return;

  do {
    dxl_rx_packet();
  } while (status_ == COMM_RXWAITING);
}

int Dynamixel::dxl_get_result(void) {
  return status_;
}

// STATUS
int Dynamixel::dxl_get_rxpacket_error(int errbit) {
  if( sPacket.data()[ERRBIT] & (unsigned char)errbit )
    return 1;

  return 0;
}

// STATUS
int Dynamixel::dxl_get_rxpacket_length(void) {
  return (int)sPacket.data()[LENGTH];
}

// STATUS
int Dynamixel::dxl_get_rxpacket_parameter(int index) {
  return (int)sPacket.data()[PARAMETER+index];
}

// Pings certain ID
void Dynamixel::dxl_ping(int id) {
  while (giBusUsing);
  iPacket.set_attr(id, 2, INST_PING);
  dxl_txrx_packet();
}

// Reads one byte
int Dynamixel::dxl_read_byte(int id, int address) {
  while (giBusUsing);
  iPacket.set_attr(id, 4, INST_READ);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, 1);
  dxl_txrx_packet();
  return (int)sPacket.data()[PARAMETER];
}

// Writes one byte
void Dynamixel::dxl_write_byte(int id, int address, int value) {
  while (giBusUsing);
  iPacket.set_attr(id, 4, INST_WRITE);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, value);
  dxl_txrx_packet();
}

// Reads 2 bytes
int Dynamixel::dxl_read_word(int id, int address) {
  while (giBusUsing);
  iPacket.set_attr(id, 4, INST_READ);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, 2);
  dxl_txrx_packet();

  return MakeWord((int)sPacket.data()[PARAMETER], (int)sPacket.data()[PARAMETER+1]);
}

// Writes 2 bytes
void Dynamixel::dxl_write_word(int id, int address, int value) {
  while (giBusUsing);
  iPacket.set_attr(id, 5, INST_WRITE);
  iPacket.set_parameter(0, address);
  iPacket.set_parameter(1, LowByte(value));
  iPacket.set_parameter(2, HighByte(value));
  dxl_txrx_packet();
}

// Calculates value based on lower and higher 8 bits
int Dynamixel::MakeWord(int lowbyte, int highbyte) {
  unsigned short word;
  word = highbyte;
  word = word << 8;
  word = word + lowbyte;
  return (int)word;
}

// Transforms value to lower 8 bits and higher
int Dynamixel::LowByte(int word) {
  unsigned short temp;

  temp = word & 0xff;
  return (int)temp;
}

// Transforms value to lower 8 bits and higher
int Dynamixel::HighByte(int word) {
  unsigned short temp;

  temp = word & 0xff00;
  temp = temp >> 8;
  return (int)temp;
}

}  // namespace dynamixel
