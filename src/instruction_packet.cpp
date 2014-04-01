#include "../include/instruction_packet.h"

namespace dynamixel {

InstructionPacket::InstructionPacket() {
  // Header present in every packet
  data_[0] = 0xff;
  data_[1] = 0xff;
  for (int i = 2; i < kMaxTxParams + kFrameSize; i++)
    data_[i] = 0;
}

void InstructionPacket::GenerateChecksum() {
  unsigned char checksum = 0;
  for (int i = 2; i <= length() + 2; i++)
    checksum += data_[i];
  data_[length() + 3] = ~checksum;
}

bool InstructionPacket::IsValid() {
  return InstructionValid() && LengthValid();
}

bool InstructionPacket::InstructionValid() {
  if (instruction() == kPing ||
      instruction() == kRead ||
      instruction() == kWrite ||
      instruction() == kRegWrite ||
      instruction() == kAction ||
      instruction() == kReset ||
      instruction() == kSyncWrite)
    return true;
  return false;
}

bool InstructionPacket::LengthValid() {
  return length() <= kMaxTxParams;
}

}  // namespace dynamixel
