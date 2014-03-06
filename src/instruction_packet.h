#ifndef DYNAMIXEL_INSTRUCTION_PACKET_H_
#define DYNAMIXEL_INSTRUCTION_PACKET_H_

namespace dynamixel {

enum InstuctionPacketData {
  kId = 2,
  kLength = 3,
  kInstruction = 4,
  kParameter = 5,
  //kErrbit = 4
};

class InstructionPacket {
 public:
  InstructionPacket() {
    data_[0] = 0xff;
    data_[1] = 0xff;
    for (int i = 2; i < kMaxTxParams + kFrameSize; i++)
      data_[i] = 0;
  }

  inline void clear() { data_[0] = 0xff; data_[1] = 0xff; }

  inline void set_id(unsigned char id) { data_[kId] = id; }
  inline void set_length(unsigned char length) { data_[kLength] = length; }
  inline void set_instruction(unsigned char instr) { data_[kInstruction] = instr; }
  inline void set_parameter(int index, unsigned char value) { data_[kParameter + index] = value; }

  inline int id() { return data_[kId]; }
  inline int length() { return data_[kLength]; }
  inline int instruction() { return data_[kInstruction]; }
  inline int parameter(int index) { return data_[kParameter + index]; }

  inline void set_attr(unsigned char id, unsigned char length, unsigned char instr) { set_id(id); set_length(length); set_instruction(instr); }

  void GenerateChecksum() {
    int checksum = 0;
    for (int i = 0; i < length() + 1; i++)
      checksum += data_[i+2];
    data_[length() + 3] = ~checksum;
  }

  unsigned char* data() { return data_; }

 private:
  static const int kFrameSize = 10;
  static const int kMaxTxParams = 150;
  static const int kMaxRxParams = 60;

  unsigned char data_[kMaxTxParams + kFrameSize];
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_INSTRUCTION_PACKET_H_
