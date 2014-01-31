#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
//#include <linux/serial.h>
#include <fcntl.h>
#include <termios.h>


#include <sys/ioctl.h>
#include <sys/time.h>
#include "dxl_hal.h"

#define LATENCY_TIME        (16)

int	gSocket_fd	= -1;
long	glStartTime	= 0;
float	gfRcvWaitTime	= 0.0f;
float	gfByteTransTime	= 0.0f;

char	gDeviceName[40];

int dxl_hal_open(int deviceIndex, float baudrate)
{
  printf("baudrate: %f\n", baudrate);

  struct termios tty;
  //struct serial_struct serinfo;
  char dev_name[100] = {0, };

  sprintf(dev_name, "/dev/tty.usbserial-A9QPXR73");
  strcpy(gDeviceName, dev_name);

  memset(&tty, 0, sizeof(tty));
  dxl_hal_close();

  if((gSocket_fd = open(gDeviceName, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
    fprintf(stderr, "device open error: %s\n", dev_name);
    goto DXL_HAL_OPEN_ERROR;
  }


  /* Set Baud Rate */
  cfsetospeed (&tty, (speed_t)B57600);
  cfsetispeed (&tty, (speed_t)B57600);

  /* Setting other Port Stuff */
  //tty.c_cflag     &=  ~PARENB;        // Make 8n1
  tty.c_cflag     &=  ~CSTOPB;
  tty.c_cflag     &=  ~CSIZE;

  tty.c_cflag     |=  CS8;
  tty.c_cflag     |=  CLOCAL;
  tty.c_cflag     |=  CREAD;

  //tty.c_iflag     |=  IGNPAR;
  //tty.c_oflag     =  0;
  //tty.c_lflag	  = 0;

  //tty.c_cc[VTIME]	= 0;
  //tty.c_cc[VMIN]	= 0;

  /*newtio.c_cflag		= B38400|CS8|CLOCAL|CREAD;
  newtio.c_iflag		= IGNPAR;
  newtio.c_oflag		= 0;
  newtio.c_lflag		= 0;
  newtio.c_cc[VTIME]	= 0;	// time-out 값 (TIME * 0.1초) 0 : disable
  newtio.c_cc[VMIN]	= 0;	// MIN 은 read 가 return 되기 위한 최소 문자 개수
*/

  //tcflush(gSocket_fd, TCIFLUSH);
  tcsetattr(gSocket_fd, TCSANOW, &tty);

  if(gSocket_fd == -1)
    return 0;


  /*if(ioctl(gSocket_fd, TIOCGSERIAL, &serinfo) < 0) {
    fprintf(stderr, "Cannot get serial info\n");
    return 0;
  }

  serinfo.flags &= ~ASYNC_SPD_MASK;
  serinfo.flags |= ASYNC_SPD_CUST;
  serinfo.custom_divisor = serinfo.baud_base / baudrate;

  if(ioctl(gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
    fprintf(stderr, "Cannot set serial info\n");
    return 0;
  }
  dxl_hal_close();

  gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);

  strcpy(gDeviceName, dev_name);
  memset(&newtio, 0, sizeof(newtio));
  dxl_hal_close();

  if((gSocket_fd = open(gDeviceName, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
    fprintf(stderr, "device open error: %s\n", dev_name);
    goto DXL_HAL_OPEN_ERROR;
  }

  newtio.c_cflag		= B38400|CS8|CLOCAL|CREAD;
  newtio.c_iflag		= IGNPAR;
  newtio.c_oflag		= 0;
  newtio.c_lflag		= 0;
  newtio.c_cc[VTIME]	= 0;	// time-out 값 (TIME * 0.1초) 0 : disable
  newtio.c_cc[VMIN]	= 0;	// MIN 은 read 가 return 되기 위한 최소 문자 개수

  tcflush(gSocket_fd, TCIFLUSH);
  tcsetattr(gSocket_fd, TCSANOW, &newtio);

  */
  return 1;

    DXL_HAL_OPEN_ERROR:
    dxl_hal_close();
  return 0;
}

void dxl_hal_close()
{
  if(gSocket_fd != -1)
    close(gSocket_fd);
  gSocket_fd = -1;
}

int dxl_hal_set_baud( float baudrate )
{
  printf("---UNIMPLEMENTED! baudrate: %f\n", baudrate);
  return 0;
  /*struct serial_struct serinfo;

  if(gSocket_fd == -1)
    return 0;

  if(ioctl(gSocket_fd, TIOCGSERIAL, &serinfo) < 0) {
    fprintf(stderr, "Cannot get serial info\n");
    return 0;
  }

  serinfo.flags &= ~ASYNC_SPD_MASK;
  serinfo.flags |= ASYNC_SPD_CUST;
  serinfo.custom_divisor = serinfo.baud_base / baudrate;

  if(ioctl(gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
    fprintf(stderr, "Cannot set serial info\n");
    return 0;
  }

  //dxl_hal_close();
  //dxl_hal_open(gDeviceName, baudrate);

  gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);
  return 1;*/
}

void dxl_hal_clear(void)
{
  tcflush(gSocket_fd, TCIFLUSH);
}

int dxl_hal_tx( unsigned char *pPacket, int numPacket )
{
  return write(gSocket_fd, pPacket, numPacket);
}

int dxl_hal_rx( unsigned char *pPacket, int numPacket )
{
  memset(pPacket, 0, numPacket);
  return read(gSocket_fd, pPacket, numPacket);
}

static inline long myclock()
{
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void dxl_hal_set_timeout( int NumRcvByte )
{
  glStartTime = myclock();
  gfRcvWaitTime = (float)(gfByteTransTime*(float)NumRcvByte + 2 * LATENCY_TIME + 5.0f);
}

int dxl_hal_timeout(void)
{
  long time;

  time = myclock() - glStartTime;

  if(time > gfRcvWaitTime)
    return 1;
  else if(time < 0)
    glStartTime = myclock();

  return 0;
}
