#include "../include/dynamixel.h"

namespace dynamixel {

Dynamixel::Dynamixel() : status_(0),
  serial_(), instruction_packet_(), status_packet_() {
}

int Dynamixel::Initialize(const std::string& device_name, int baudrate) {
  serial_ = new Serial();

  if (serial_->Open(device_name.c_str(), baudrate) == -1)
    return -1;

  Status(kSuccess);
  return 0;
}

Dynamixel::~Dynamixel() {
  if (serial_ != NULL)
    delete serial_;
}

void Dynamixel::Close(void) {
  delete serial_;
  serial_ = NULL;
}

int Dynamixel::Status(int status){
  status_ = status;
  return status;
}

int Dynamixel::TxPacket() {
  if (!instruction_packet_.IsValid())
    return Status(kTxError);

  instruction_packet_.GenerateChecksum();

  if (status_ == kRxTimeout || status_ == kRxCorrupt)
    serial_->Flush();

  int written_bytes = serial_->Write(instruction_packet_.data(), instruction_packet_.PacketLength());

  if (instruction_packet_.PacketLength() != written_bytes)
    return Status(kTxFail);

  if (instruction_packet_.instruction() == kRead)
    serial_->SetTimeout(instruction_packet_.parameter(1) + 6);
  else
    serial_->SetTimeout(6);

  return Status(kSuccess);
}

int Dynamixel::RxPacket(void) {
  if (instruction_packet_.id() == kBroadcastId)
    return Status(kSuccess);

  status_packet_.set_read_length(0);
  status_packet_.set_expected_length(6);

  status_packet_.Read(serial_->Read(status_packet_.ReadPosition(), status_packet_.RemainingLength()));
  status_packet_.ShiftHeader();

  while (status_packet_.RemainingLength() > 0) {
    status_packet_.Read(serial_->Read(status_packet_.ReadPosition(), status_packet_.RemainingLength()));
    status_packet_.ShiftHeader();

    if (serial_->Timeout())
      return Status(status_packet_.read_length() == 0 ? kRxTimeout : kRxCorrupt);
  }

  if (instruction_packet_.id() != status_packet_.id())
    return Status(kRxCorrupt);

  status_packet_.set_expected_length(status_packet_.length() + 4);

  return ReadPacketParams();
}

int Dynamixel::ReadPacketParams() {
  while (status_packet_.RemainingLength() > 0)
    status_packet_.Read(serial_->Read(status_packet_.ReadPosition(), status_packet_.RemainingLength()));

  if (!status_packet_.ChecksumValid())
    return Status(kRxCorrupt);

  return Status(kSuccess);
}

int Dynamixel::TxRxPacket() {
  TxPacket();
  if (status_ < 0)
    return status_;
  else
    return RxPacket();
}

// Pings certain ID
void Dynamixel::Ping(int id) {
  instruction_packet_.set_attr(id, 2, kPing);
  TxRxPacket();
}

// Reads one byte
int Dynamixel::ReadByte(int id, int address) {
  instruction_packet_.set_attr(id, 4, kRead);
  instruction_packet_.set_parameter(0, address);
  instruction_packet_.set_parameter(1, 1);
  TxRxPacket();
  return (int)status_packet_.param(0);
}

// Writes one byte
void Dynamixel::WriteByte(int id, int address, int value) {
  instruction_packet_.set_attr(id, 4, kWrite);
  instruction_packet_.set_parameter(0, address);
  instruction_packet_.set_parameter(1, value);
  TxRxPacket();
}

// Reads 2 bytes
int Dynamixel::ReadWord(int id, int address) {
  instruction_packet_.set_attr(id, 4, kRead);
  instruction_packet_.set_parameter(0, address);
  instruction_packet_.set_parameter(1, 2);
  TxRxPacket();

  return MakeWord(status_packet_.param(0), status_packet_.param(1));
}

// Writes 2 bytes
void Dynamixel::WriteWord(int id, int address, int value) {
  instruction_packet_.set_attr(id, 5, kWrite);
  instruction_packet_.set_parameter(0, address);
  instruction_packet_.set_parameter(1, LowByte(value));
  instruction_packet_.set_parameter(2, HighByte(value));
  TxRxPacket();
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
