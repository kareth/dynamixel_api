#ifndef DYNAMIXEL_INSTRUCTION_PACKET_H_
#define DYNAMIXEL_INSTRUCTION_PACKET_H_

#include "common.h"

namespace dynamixel {

class InstructionPacket {
 public:
  InstructionPacket();

  inline void set_id(int id) { data_[kInstrId] = id; }
  inline void set_length(int length) { data_[kInstrLength] = length; }
  inline void set_instruction(int instr) { data_[kInstrInstruction] = instr; }
  inline void set_parameter(int index, int value) { data_[kInstrParameter + index] = value; }

  inline int id() const { return data_[kInstrId]; }
  inline int length() const { return data_[kInstrLength]; }
  inline int instruction() const { return data_[kInstrInstruction]; }
  inline int parameter(int index) const { return data_[kInstrParameter + index]; }

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
