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

int	gSocket_fd	= -1;
long	glStartTime	= 0;
float	gfRcvWaitTime	= 0.0f;
float	gfByteTransTime	= 0.0f;

int GetBaudRate(int baudrate){
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

int dxl_hal_open(const std::string& device, int baudrate) {
  struct termios newtio;
  memset(&newtio, 0, sizeof(newtio));

  dxl_hal_close();

  if((gSocket_fd = open(device.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
    fprintf(stderr, "device open error: %s\n", device.c_str());
    return -1;
  }

  int baud = GetBaudRate(baudrate);
  if (baud == -1) return -1;

  newtio.c_cflag     = baud | CS8 | CLOCAL | CREAD;
  newtio.c_iflag     = IGNPAR;
  newtio.c_oflag     = 0;
  newtio.c_lflag     = 0;
  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN]  = 0;

  tcflush(gSocket_fd, TCIFLUSH);
  tcsetattr(gSocket_fd, TCSANOW, &newtio);

  return 0;
}

void dxl_hal_close() {
  if(gSocket_fd != -1)
    close(gSocket_fd);
  gSocket_fd = -1;
}

void dxl_hal_clear(void) {
  tcflush(gSocket_fd, TCIFLUSH);
}

int dxl_hal_tx( unsigned char *pPacket, int numPacket ) {
  return write(gSocket_fd, pPacket, numPacket);
}

int dxl_hal_rx( unsigned char *pPacket, int numPacket ) {
  // TODO this memset is probably not necessary
  memset(pPacket, 0, numPacket);
  return read(gSocket_fd, pPacket, numPacket);
}

// TODO timeouts should go away
static inline long myclock() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void dxl_hal_set_timeout( int NumRcvByte ) {
  glStartTime = myclock();
  gfRcvWaitTime = (float)(gfByteTransTime*(float)NumRcvByte + 2.0 * LATENCY_TIME + 0.0f);
}

int dxl_hal_timeout(void) {
  long time;

  time = myclock() - glStartTime;

  if(time > gfRcvWaitTime)
    return 1;
  else if(time < 0)
    glStartTime = myclock();

  return 0;
}

}  // namespace dynamixel
