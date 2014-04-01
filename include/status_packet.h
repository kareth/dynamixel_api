#ifndef DYNAMIXEL_STATUS_PACKET_H_
#define DYNAMIXEL_STATUS_PACKET_H_

#include "common.h"

namespace dynamixel {

class StatusPacket {
 public:
  // Shifts all data left until two 0xff are found
  void ShiftHeader();

  // 1 if valid, 0 if invalid
  int ChecksumValid();

  // Read position based on amount of bytes already read
  unsigned char* ReadPosition() { return &data_[read_length_]; }

  // Notifies about another bytes read
  void Read(int bytes) { read_length_ += bytes; }

  int RemainingLength() const { return expected_length_ - read_length_; }

  unsigned char id() const { return data_[kStatusId]; }
  unsigned char length() const { return data_[kStatusLength]; }
  unsigned char param(int index) const { return data_[kStatusParameter + index]; }
  unsigned char error(unsigned char errbit) const { return (data_[kStatusError] & errbit) ? 1 : 0; }
  unsigned char checksum() const { return data_[length() + 3]; }

  int read_length() const { return read_length_; }
  int expected_length() const { return expected_length_; }
  void set_read_length(int length) { read_length_ = length; }
  void set_expected_length(int length) { expected_length_ = length; }

 private:
  int expected_length_;
  int read_length_;

  static const int kFrameSize = 10;
  static const int kMaxRxParams = 60;

  unsigned char data_[kMaxRxParams + kFrameSize];
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_STATUS_PACKET_H_
