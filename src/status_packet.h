#ifndef DYNAMIXEL_STATUS_PACKET_H_
#define DYNAMIXEL_STATUS_PACKET_H_

namespace dynamixel {

class StatusPacket {
 public:
  StatusPacket(){}
  ~StatusPacket(){}

  void ShiftHeader();

  void Clear() { read_length_ = 0; }
  unsigned char* ReadPosition() { return &data_[read_length_]; }
  int RemainingLength() { return expected_length_ - read_length_; }

  void set_expected_length(int length) { expected_length_ = length; }
  int read_length() { return read_length_; }
  int expected_length() { return expected_length_; }

  void Read(int bytes) { read_length_ += bytes; }
  unsigned char* data() { return data_; }

 private:
   int expected_length_;
   int read_length_;
   unsigned char data_[10000];
   // MAXRX+10
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_STATUS_PACKET_H_
