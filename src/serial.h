#ifndef DYNAMIXEL_SERIAL_H_
#define DYNAMIXEL_SERIAL_H_

#include <string>

namespace dynamixel {

class Serial {
 public:
  Serial();
  ~Serial();
  int dxl_hal_open(const std::string& device, int baudrate);
  void dxl_hal_close();
  int dxl_hal_set_baud(float baudrate);
  void dxl_hal_clear();
  int dxl_hal_tx(unsigned char *pPacket, int numPacket);
  int dxl_hal_rx(unsigned char *pPacket, int numPacket);
  void dxl_hal_set_timeout(int NumRcvByte);
  int dxl_hal_timeout();

 private:
  int GetBaudRate(int baudrate);

  int socket_fd_;
  long glStartTime_;
  float	gfRcvWaitTime_;
  float	gfByteTransTime_;
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_SERIAL_H_
