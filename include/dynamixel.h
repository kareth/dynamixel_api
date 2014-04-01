#ifndef DYNAMIXEL_DYNAMIXEL_H_
#define DYNAMIXEL_DYNAMIXEL_H_

#include <string>
#include <cstdio>

#include "status_packet.h"
#include "instruction_packet.h"
#include "serial.h"
#include "common.h"

namespace dynamixel {

//! Main class to use for communication with dynamixels
/*! By far it doesnt support SyncWrite.
 *  All status packet and instruction packet methods has been hidden */
class Dynamixel {
 public:
  //! Default constructor
  Dynamixel();

  //! Closes serial port
  ~Dynamixel();

  //! Initializes serial port for communication with dynamixels
  /*! \param device_name serial port path to use
   *  \param baudrate serial port speed, only standard values are accepted
   *  \return 0 if succeeded
   *  \return -1 if opening serial port failed */
  int Initialize(const std::string& device_name, int baudrate);

  //! Closes serial port
  /*! After \a Close is called, reading and writing methods should not
   *  be used until another initialize is called */
  void Close();


  //! Sends ping command to servo
  /*! \param id id of servo to be pinged (0-254) */
  void Ping(int id);

  //! Reads one byte from given servo
  /*! \param id id of servo (0-254)
   *  \param address address of the servo register
   *  \return read byte value */
  int ReadByte(int id, int address);

  //! Writes one byte to given servo
  /*! \param id id of servo (0-254)
   *  \param address address of the servo register */
  void WriteByte(int id, int address, int value);

  //! Reads one word (two bytes) from given servo
  /*! \param id id of servo (0-254)
   *  \param address address of the first servo register */
  int ReadWord(int id, int address);

  //! Writes one word (two bytes) to given servo
  /*! \param id id of servo (0-254)
   *  \param address address of the first servo register */
  void WriteWord(int id, int address, int value);

  //! Returns status of the previous commands
  /*! \return 0 if succeeded
   *  \return negative CommandStatus value if fails*/
  int status() const { return status_; }

 private:
  //! Writes previously prepared \a instruction_packet_ to serial
  /*! \return \a 0 if success
   *  \return negative \a CommandStatus value if fails */
  int TxPacket();

  //! Reads packet from serial to \a status_packet_
  /*! \return \a 0 if success
   *  \return negative \a CommandStatus value if fails */
  int RxPacket();

  //! Reads additional parameters of packet based on received length
  /*! \return \a 0 if success
   *  \return negative \a CommandStatus value if fails */
  int ReadPacketParams();

  //! Writex packet to serial and reads answer afterwards
  /*! \return \a 0 if success
   *  \return negative \a CommandStatus value if fails */
  int TxRxPacket();

  //! Joins two bytes into 16-bit word
  int MakeWord(int lowbyte, int highbyte);

  //! Returns lower (less important) byte of 16-bit word
  int LowByte(int word);

  //! Returns higher (more important) byte of 16-bit word
  int HighByte(int word);

  //! Saves status to status_ and returns itself
  //  \param status status that just occured
  //  \return \a status
  int Status(int status);

  //! Broadcast ID.
  /*! Sending command to this ID sends it to all servos,
   *  and doesn't read any answers afterwards */
  const static int kBroadcastId = 254;

  //! Serial port used for communication
  Serial* serial_;

  //! Instruction packet object prepared before each TX
  InstructionPacket instruction_packet_;

  //! Status packet object filled after each RX
  StatusPacket status_packet_;

  //! Last command status
  int status_;
};

}  // namespace dynamixel

#endif  // DYNAMIXEL_DYNAMIXEL_H_
