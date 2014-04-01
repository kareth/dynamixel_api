#include "../include/status_packet.h"

namespace dynamixel {

void StatusPacket::ShiftHeader() {
  int pos;
  for (pos = 0; pos < read_length_ - 1; pos++)
    if ((data_[pos] == 0xff && data_[pos + 1] == 0xff) ||
        (pos == read_length_ - 2 && data_[read_length_ - 1] == 0xff))
      break;

  if (pos > 0) {
    for (int j = 0; j < read_length_ - pos; j++)
      data_[j] = data_[j + pos];
    read_length_ -= pos;
  }
}

int StatusPacket::ChecksumValid()  {
  unsigned char csum = 0;
  for (int i = 2; i < length() + 3; i++)
    csum += data_[i];
  csum = ~csum;
  return (checksum() == csum);
}

}  // namespace dynamixel
