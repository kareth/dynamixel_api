#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>

#include <sys/ioctl.h>
#include <sys/time.h>

#include "serial.h"

#define LATENCY_TIME        (4)

namespace dynamixel {

Serial::Serial()
  : socket_fd_(-1),
    glStartTime_(0),
    gfRcvWaitTime_(0),
    gfByteTransTime_(0) {
}

Serial::~Serial() {
  Close();
}

int Serial::Open(const std::string& device, int baudrate) {
  struct termios newtio;
  memset(&newtio, 0, sizeof(newtio));

  Close();

  if ((socket_fd_ = open(device.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
    fprintf(stderr, "device open error: %s\n", device.c_str());
    return -1;
  }

  int baud = GetBaudRateConst(baudrate);
  if (baud == -1)
    return -1;

  newtio.c_cflag     = baud | CS8 | CLOCAL | CREAD;
  newtio.c_iflag     = IGNPAR;
  newtio.c_oflag     = 0;
  newtio.c_lflag     = 0;
  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN]  = 0;

  tcflush(socket_fd_, TCIFLUSH);
  tcsetattr(socket_fd_, TCSANOW, &newtio);

  return 0;
}

void Serial::Close() {
  if (socket_fd_ != -1)
    close(socket_fd_);
  socket_fd_ = -1;
}

void Serial::Flush(void) {
  tcflush(socket_fd_, TCIFLUSH);
}

int Serial::Write(unsigned char *pPacket, int numPacket) {
  return write(socket_fd_, pPacket, numPacket);
}

int Serial::Read(unsigned char *pPacket, int numPacket) {
  // TODO this memset is probably not necessary
  memset(pPacket, 0, numPacket);
  return read(socket_fd_, pPacket, numPacket);
}

int Serial::GetBaudRateConst(int baudrate){
  switch (baudrate) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
    default:
      return -1;
  }
}

// TODO timeouts should go away
static inline long myclock() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void Serial::SetTimeout(int NumRcvByte) {
  glStartTime_ = myclock();
  gfRcvWaitTime_ = (float)(gfByteTransTime_*(float)NumRcvByte + 2.0 * LATENCY_TIME + 0.0f);
}

int Serial::Timeout() {
  long time;

  time = myclock() - glStartTime_;

  if (time > gfRcvWaitTime_)
    return 1;
  else if (time < 0)
    glStartTime_ = myclock();

  return 0;
}

}  // namespace dynamixel
