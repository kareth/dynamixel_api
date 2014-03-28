#include "status_packet.h"

namespace dynamixel {

void StatusPacket::ShiftHeader() {
  // Find packet header
  int i, j;
  for (i = 0; i < (read_length_ - 1); i++) {
    if (data_[i] == 0xff && data_[i+1] == 0xff) {
      break;
    } else if (i == read_length_-2 && data_[read_length_-1] == 0xff) {
      break;
    }
  }
  if (i > 0) {
    for (j = 0; j < (read_length_ - i); j++)
      data_[j] = data_[j + i];
    read_length_ -= i;
  }
}

}  // namespace dynamixel
