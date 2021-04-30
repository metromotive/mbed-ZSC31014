// Copyright 2021 Metromotive

#include "ZSC31014.h"
#include "DigitalOut.h"

namespace metromotive {

ZSC31014::ZSC31014(I2C &i2c, char address7bit, DigitalOut powerPin) :
    i2c(i2c),
    address(address7bit << 1),
    powerPin(powerPin)
{
    i2c.frequency(400000);
}

void ZSC31014::startCommandMode() {
    powerPin.write(0);

    wait_us(1000000);
 
    powerPin.write(1);

    wait_us(1000);

    this->write(StartCommandMode);

    wait_us(10);
}

void ZSC31014::startNormalOperationMode() {
    this->write(StartNormalOperationMode);
}

uint16_t ZSC31014::getCustomerID0() {
    return this->read(ReadCust_ID0);
}

uint16_t ZSC31014::getCustomerID1() {
    return this->read(ReadCust_ID1);
}

uint16_t ZSC31014::getCustomerID2() {
    return this->read(ReadCust_ID2);
}

uint16_t ZSC31014::read(ReadCommand command) {
  char writePacket[3] = {command, 0x00, 0x00};
  if (this->i2c.write(address, writePacket, 3) != 0) {
    printf("Unable to write to device. Check i2c address and connections.\n");
  }

  wait_us(10);

  char readPacket[3] = {0x00, 0x00, 0x00};

  if (this->i2c.read(address, readPacket, 3) != 0) {
    printf("Unable to read from device. Check i2c address and connections.\n");
    return -1;
  } else if (readPacket[0] != 0x5A) {
    printf("Invalid response byte from device. Maybe not in command mode? "
           "(bytes are %2x %2x %2x).\n",
           readPacket[0], readPacket[1], readPacket[2]);
    return -1;
  } else {
    return (readPacket[1] << 8) | readPacket[2];
  }
}

void ZSC31014::write(WriteCommand command, uint16_t value) {
    char packet[3] = { command, (char)(value >> 8), (char)(value & 0xFF) };
    if (this->i2c.write(address, packet, 3) != 0) {
        printf("Unable to write to device. Check i2c address and connections.\n");
    }
// char packet[3] = { command, (char)(value >> 8), (char)(value & 0xFF) };
//   if (this->i2c.write(address, packet, 3) != 0) {
//     printf("Unable to write to device. Check i2c address and connections.\n");
//   }
}

} // namespace metromotive