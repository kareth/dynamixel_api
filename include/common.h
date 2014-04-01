#ifndef DYNAMIXEL_COMMON_H_
#define DYNAMIXEL_COMMON_H_

namespace dynamixel {

enum CommandStatus {
  kSuccess = 0,
  kTxFail = -1,
  kRxFail = -2,
  kTxError = -3,
  kRxWaiting = -4,
  kRxTimeout = -5,
  kRxCorrupt = -6
};

enum Errbits {
  kErrVoltage = 1,
  kErrAngle = 2,
  kErrOverheat = 4,
  kErrRange = 8,
  kErrChecksum = 16,
  kErrIverload = 32,
  kErrInstruction = 64
};

enum InstuctionPacketData {
  kInstrId = 2,
  kInstrLength = 3,
  kInstrInstruction = 4,
  kInstrParameter = 5,
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

enum StatusPacketData {
  kStatusId = 2,
  kStatusLength = 3,
  kStatusError = 4,
  kStatusParameter = 5,
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_COMMON_H_
