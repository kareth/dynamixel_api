#ifndef DYNAMIXEL_INSTRUCTION_PACKET_H_
#define DYNAMIXEL_INSTRUCTION_PACKET_H_

namespace dynamixel {

enum InstuctionPacketData {
  kId = 2,
  kLength = 3,
  kInstruction = 4,
  kParameter = 5,
};

enum InstructionType {
  kPing = 1,
  kRead = 2,
  kWrite = 3,
  kRegWrite = 4,
  kAction = 5,
  kReset = 6,
  kSyncWrite = 131
};

class InstructionPacket {
 public:
  InstructionPacket();

  inline void set_id(int id) { data_[kId] = id; }
  inline void set_length(int length) { data_[kLength] = length; }
  inline void set_instruction(int instr) { data_[kInstruction] = instr; }
  inline void set_parameter(int index, int value) { data_[kParameter + index] = value; }

  inline int id() const { return data_[kId]; }
  inline int length() const { return data_[kLength]; }
  inline int instruction() const { return data_[kInstruction]; }
  inline int parameter(int index) const { return data_[kParameter + index]; }

  inline void set_attr(int id, int length, int instr) { set_id(id); set_length(length); set_instruction(instr); }

  void GenerateChecksum();
  bool IsValid();

  // Length + 2 first bytes + id + checksum
  int PacketLength() { return length() + 4; }

  unsigned char* data() { return data_; }

 private:
  bool InstructionValid();
  bool LengthValid();

  static const int kFrameSize = 10;
  static const int kMaxTxParams = 150;
  static const int kMaxRxParams = 60;

  unsigned char data_[kMaxTxParams + kFrameSize];
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_INSTRUCTION_PACKET_H_
