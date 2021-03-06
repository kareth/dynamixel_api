//##########################################################
//##                      R O B O T I S                   ##
//##          ReadWrite Example code for Dynamixel.       ##
//##                                           2009.11.10 ##
//##########################################################
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "../../include/dynamixel.h"
#include <cmath>
#include <stdlib.h>
#include <chrono>
#include <string>

// Control table address
#define P_GOAL_POSITION_L	30
#define P_GOAL_POSITION_H	31
#define P_PRESENT_POSITION_L	36
#define P_PRESENT_POSITION_H	37
#define P_MOVING		46

// Defulat setting
#define DEFAULT_BAUDNUM		1 // 1Mbps
#define DEFAULT_ID		1

using namespace dynamixel;
dynamixel::Dynamixel* dynamixel_api;

void PrintCommStatus(int CommStatus);
void PrintErrorCode(void);

double TimeSince(std::chrono::time_point<std::chrono::system_clock> start) {
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  return elapsed_seconds.count();
}

void SetSpeed(int dynamixel_id, int speed) {
  dynamixel_api->WriteWord( dynamixel_id, 32, speed);
  printf("Speed: %d\n", speed);
      int status = dynamixel_api->status();

      if (status != kSuccess) {
        PrintCommStatus(status);
      }
}

void TestRead(int dynamixel_id) {
  printf("Testing read speed\n");
  int iterations = 2000;

  int words_count = 1;
  int words[] = {30, 32, 34, 36, 38, 40};

  int bytes_count = 0;
  int bytes[] = {42, 43, 46};

  for (int i = 0; i < words_count; i++) {
    auto start = std::chrono::system_clock::now();
    for (int it = 0; it < iterations; it++) {
      int pos = dynamixel_api->ReadWord(dynamixel_id, words[i]);
      int status = dynamixel_api->status();
      if (status != kSuccess) {
        PrintCommStatus(status);
        break;
      }
    }
    printf("Word %d speed: %lf Hz\n", words[i], double(iterations) / TimeSince(start));
  }

  for (int i = 0; i < bytes_count; i++) {
    auto start = std::chrono::system_clock::now();
    for (int it = 0; it < iterations; it++) {
      int pos = dynamixel_api->ReadByte(dynamixel_id, bytes[i]);
      int status = dynamixel_api->status();
      if (status != kSuccess) {
        PrintCommStatus(status);
        break;
      }
    }
    printf("Byte %d speed: %lf Hz\n", bytes[i], double(iterations) / TimeSince(start));
  }
}

void TestWrite(int dynamixel_id) {
  int iterations = 2000;

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < iterations; i++) {
    dynamixel_api->WriteWord(dynamixel_id, 32, 1023); // Moving speed
  }
  printf("Write speed: %lf Hz\n", double(iterations) / TimeSince(start) );
}

void TestMovingSpeed(int dynamixel_id, int speed) {
  int positions[2] = {140, 884};
  double range = (300.0 / 1024.0) * double(positions[1] - positions[0]);
  printf("Testing moving speed on %lf degrees range\n", range);

  SetSpeed(dynamixel_id, speed);

  int iterations = 4;
  printf("Iterations: %d\n", iterations);

  auto start = std::chrono::system_clock::now();

  for (int it = 0; it < iterations; it++) {
    int i = it % 2;
    dynamixel_api->WriteWord(dynamixel_id, P_GOAL_POSITION_L, positions[i]);

    int pos;
    do {
      pos = dynamixel_api->ReadWord(dynamixel_id, P_PRESENT_POSITION_L);
      int status = dynamixel_api->status();

      if (status != kSuccess) {
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

    dynamixel_api->WriteWord(dynamixel_id, P_GOAL_POSITION_L, positions[i]);

    int pos;
    do {
      pos = dynamixel_api->ReadWord( dynamixel_id, P_PRESENT_POSITION_L );
      int status = dynamixel_api->status();

      if (status != kSuccess) {
        PrintCommStatus(status);
        break;
      }
      /*
      int goal_position = dynamixel_api->ReadWord( DEFAULT_ID, 30);
      int goal_speed = dynamixel_api->ReadWord( DEFAULT_ID, 32);
      int limit = dynamixel_api->ReadWord( DEFAULT_ID, 34);
      int position = dynamixel_api->ReadWord( DEFAULT_ID, 36);
      int speed = dynamixel_api->ReadWord( DEFAULT_ID, 38);
      int load = dynamixel_api->ReadWord( DEFAULT_ID, 38);
      int moving = Moving = dynamixel_api->ReadByte( DEFAULT_ID, 46 );
      int temp = Moving = dynamixel_api->ReadByte( DEFAULT_ID, 46 );
      */
    } while(abs(positions[i] - pos) > 25);
  }
}

void MoveRoboticArm() {
  printf("Do you really want to run leg simulation? enter 123\n");
  int ans;
  scanf("%d",&ans);
  if (ans != 123) return;

  int speed = 100;
  printf("Speed: \n");
  scanf("%d", &speed);

  SetSpeed(1, speed);
  SetSpeed(2, speed);
  SetSpeed(3, speed);

  int p[2][3];
  p[0][0] = 612;
  p[0][1] = 232;
  p[0][2] = 412;
  p[1][0] = 732;
  p[1][1] = 162;
  p[1][2] = 360;

  int i = 0;
  while(1) {
    printf("Press enter\n");
    getchar();
    i^=1;

    if (p[i][0] < 884 && p[i][0] > 140)
      dynamixel_api->WriteWord(1, P_GOAL_POSITION_L, p[i][0]);
    else
      printf("OOB\n");

    if (p[i][1] < 884 && p[i][1] > 140)
      dynamixel_api->WriteWord(2, P_GOAL_POSITION_L, p[i][1]);
    else
      printf("OOB\n");

    if (p[i][2] < 884 && p[i][2] > 140)
      dynamixel_api->WriteWord(3, P_GOAL_POSITION_L, p[i][2]);
    else
      printf("OOB\n");
  }
}

int OpenUsb2Dynamixel() {
  ///////// Open USB2Dynamixel ////////////
  if (dynamixel_api->Initialize("/dev/ttymxc4", 1000000) == -1) {
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
  dynamixel_api = new dynamixel::Dynamixel();

  if (OpenUsb2Dynamixel() == -1)
    return 0;

  int id = 3;

  printf("%d\n", dynamixel_api->ReadByte(3, 5));
  dynamixel_api->WriteByte(3, 5, 1);

      int status = dynamixel_api->status();

      if (status != kSuccess) {
        PrintCommStatus(status);
      }

  printf("%d\n", dynamixel_api->ReadByte(3, 5));
  printf("%d\n", dynamixel_api->ReadByte(3, 4));
  //dynamixel_api->WriteByte(3, 4, 1);
  //dynamixel_api->WriteByte(id, 5, 250);
  //printf("%d\n", dynamixel_api->ReadByte(0, 4));
  //printf("%d\n", dynamixel_api->ReadByte(1, 4));
  //printf("%d\n", dynamixel_api->ReadByte(2, 4));
  printf("%d\n", dynamixel_api->ReadByte(3, 4));

  //dynamixel_api->WriteByte(id, 4, 1);
  //dynamixel_api->WriteByte(id, 4, 34);
  //printf("%d\n", dynamixel_api->ReadByte(id, 4));
  //return 0;

  // MoveRoboticArm();

  //ID
  //dynamixel_api->WriteByte(id, 3, 2);

  while (1) {
    printf("1 - Movement\n2 - FromTo\n3 - Read\n4 - Write\n");
    int n;
    scanf("%d",&n);
    if (n == 1) {
      printf("What speed?\n");
      int speed;
      scanf("%d", &speed);
      if (speed < 0 || speed > 1023)
        continue;
      TestMovingSpeed(id, speed);
    }
    if (n == 2)
      FromTo(id, 140, 884, 0);
    if (n == 3)
      TestRead(id);
    if (n == 4)
      TestWrite(id);
    if (n == 5) {

    }
    if (n == 6) {

    }
  }

  dynamixel_api->Close();
  return 0;
}

// Print communication result
void PrintCommStatus(int CommStatus) {
  switch(CommStatus) {
    case kTxFail:
      printf("COMM_TXFAIL: Failed transmit instruction packet!\n");
      break;

    case kTxError:
      printf("COMM_TXERROR: Incorrect instruction packet!\n");
      break;

    case kRxFail:
      printf("COMM_RXFAIL: Failed get status packet from device!\n");
      break;

    case kRxWaiting:
      printf("COMM_RXWAITING: Now recieving status packet!\n");
      break;

    case kRxTimeout:
      printf("COMM_RXTIMEOUT: There is no status packet!\n");
      break;

    case kRxCorrupt:
      printf("COMM_RXCORRUPT: Incorrect status packet!\n");
      break;

    default:
      printf("This is unknown error code!\n");
      break;
  }
}

// Print error bit of status packet
void PrintErrorCode() {
 /* if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
    printf("Input voltage error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
    printf("Angle limit error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
    printf("Overheat error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
    printf("Out of range error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
    printf("Checksum error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
    printf("Overload error!\n");

  if(dynamixel_api->dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
    printf("Instruction code error!\n");*/
}
