#ifndef DYNAMIXEL_SERIAL_H_
#define DYNAMIXEL_SERIAL_H_

#include <string>

namespace dynamixel {

class Serial {
 public:
  Serial();
  ~Serial();
  int Open(const std::string& device, int baudrate);
  void Close();
  void Flush();
  int Write(unsigned char *pPacket, int numPacket);
  int Read(unsigned char *pPacket, int numPacket);

  // TODO to remove
  void SetTimeout(int NumRcvByte);
  int Timeout();

 private:
  int GetBaudRateConst(int baudrate);

  int socket_fd_;

  // TODO to remove
  long glStartTime_;
  float	gfRcvWaitTime_;
  float	gfByteTransTime_;
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_SERIAL_H_
