//##########################################################
//##                      R O B O T I S                   ##
//##          ReadWrite Example code for Dynamixel.       ##
//##                                           2009.11.10 ##
//##########################################################
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <dynamixel.h>
#include <cmath>
#include <stdlib.h>
#include <chrono>

// Control table address
#define P_GOAL_POSITION_L	30
#define P_GOAL_POSITION_H	31
#define P_PRESENT_POSITION_L	36
#define P_PRESENT_POSITION_H	37
#define P_MOVING		46

// Defulat setting
#define DEFAULT_BAUDNUM		1 // 1Mbps
#define DEFAULT_ID		1

void PrintCommStatus(int CommStatus);
void PrintErrorCode(void);

double TimeSince(std::chrono::time_point<std::chrono::system_clock> start) {
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  return elapsed_seconds.count();
}

void SetSpeed(int dynamixel_id, int speed) {
  dxl_write_word( dynamixel_id, 32, speed);
  printf("Speed: %d\n", speed);
}

void TestRead(int dynamixel_id) {
  printf("Testing read speed\n");
  int iterations = 50;

  int words_count = 6;
  int words[] = {30, 32, 34, 36, 38, 40};

  int bytes_count = 3;
  int bytes[] = {42, 43, 46};

  for (int i = 0; i < words_count; i++) {
    auto start = std::chrono::system_clock::now();
    for (int it = 0; it < iterations; it++) {
      int pos = dxl_read_word(dynamixel_id, words[i]);
      int status = dxl_get_result();
    }
    printf("Word %d speed: %lf Hz\n", words[i], double(iterations) / TimeSince(start));
  }

  for (int i = 0; i < bytes_count; i++) {
    auto start = std::chrono::system_clock::now();
    for (int it = 0; it < iterations; it++) {
      int pos = dxl_read_byte(dynamixel_id, bytes[i]);
      int status = dxl_get_result();
    }
    printf("Byte %d speed: %lf Hz\n", bytes[i], double(iterations) / TimeSince(start));
  }
}

void TestWrite(int dynamixel_id) {
  int iterations = 100;

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < iterations; i++) {
    dxl_write_word(dynamixel_id, 32, 1023); // Moving speed
  }
  printf("Write speed: %lf Hz\n", double(iterations) / TimeSince(start) );
}

void TestMovingSpeed(int dynamixel_id, int speed) {
  int positions[2] = {140, 884};
  double range = (300.0 / 1024.0) * double(positions[1] - positions[0]);
  printf("Testing moving speed on %lf degrees range\n", range);

  SetSpeed(dynamixel_id, speed);

  int iterations = 16;
  printf("Iterations: %d\n", iterations);

  auto start = std::chrono::system_clock::now();

  for (int it = 0; it < iterations; it++) {
    int i = it % 2;
    dxl_write_word(dynamixel_id, P_GOAL_POSITION_L, positions[i]);

    int pos;
    do {
      pos = dxl_read_word(dynamixel_id, P_PRESENT_POSITION_L);
      int status = dxl_get_result();

      if (status != COMM_RXSUCCESS) {
        PrintCommStatus(status);
        break;
      }
    } while (abs(positions[i] - pos) > 15);
  }

  double time = TimeSince(start);
  double rotations_done = iterations * range / 360.0;
  double rpm = rotations_done / time * 60.0;
  printf("\nTotal rpm: %lf\n\n", rpm);
}

void FromTo(int dynamixel_id, int from, int to, int speed) {
  int positions[] = {from,  to};

  SetSpeed(dynamixel_id, speed);

  for (int i = 0;; i ^= 1) {
    printf("Press Enter key to continue!(press ESC and Enter to quit)\n");
    if (getchar() == 0x1b) break;

    dxl_write_word(dynamixel_id, P_GOAL_POSITION_L, positions[i]);

    int pos;
    do {
      pos = dxl_read_word( DEFAULT_ID, P_PRESENT_POSITION_L );
      int status = dxl_get_result();

      if (status != COMM_RXSUCCESS) {
        PrintCommStatus(status);
        break;
      }
      /*
      int goal_position = dxl_read_word( DEFAULT_ID, 30);
      int goal_speed = dxl_read_word( DEFAULT_ID, 32);
      int limit = dxl_read_word( DEFAULT_ID, 34);
      int position = dxl_read_word( DEFAULT_ID, 36);
      int speed = dxl_read_word( DEFAULT_ID, 38);
      int load = dxl_read_word( DEFAULT_ID, 38);
      int moving = Moving = dxl_read_byte( DEFAULT_ID, 46 );
      int temp = Moving = dxl_read_byte( DEFAULT_ID, 46 );
      */
    } while(abs(positions[i] - pos) > 25);
  }
}


int OpenUsb2Dynamixel() {
  ///////// Open USB2Dynamixel ////////////
  if(dxl_initialize(0, 34) == 0) {
    printf( "Failed to open USB2Dynamixel!\n" );
    return -1;
  }
  else {
    printf( "Succeed to open USB2Dynamixel!\n" );
    return 0;
  }
}


int main() {
  //int GoalPos[2] = {140, 884};

  if (OpenUsb2Dynamixel() == -1)
    return 0;

  //TestMovingSpeed(1, 1023);

  //FromTo(1, 140, 884, 0);
  TestRead(1);
  TestWrite(1);

  dxl_terminate();
  return 0;
}

// Print communication result
void PrintCommStatus(int CommStatus) {
  switch(CommStatus) {
    case COMM_TXFAIL:
      printf("COMM_TXFAIL: Failed transmit instruction packet!\n");
      break;

    case COMM_TXERROR:
      printf("COMM_TXERROR: Incorrect instruction packet!\n");
      break;

    case COMM_RXFAIL:
      printf("COMM_RXFAIL: Failed get status packet from device!\n");
      break;

    case COMM_RXWAITING:
      printf("COMM_RXWAITING: Now recieving status packet!\n");
      break;

    case COMM_RXTIMEOUT:
      printf("COMM_RXTIMEOUT: There is no status packet!\n");
      break;

    case COMM_RXCORRUPT:
      printf("COMM_RXCORRUPT: Incorrect status packet!\n");
      break;

    default:
      printf("This is unknown error code!\n");
      break;
  }
}

// Print error bit of status packet
void PrintErrorCode() {
  if(dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
    printf("Input voltage error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
    printf("Angle limit error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
    printf("Overheat error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
    printf("Out of range error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
    printf("Checksum error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
    printf("Overload error!\n");

  if(dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
    printf("Instruction code error!\n");
}
