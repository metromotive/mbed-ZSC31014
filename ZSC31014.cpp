// Copyright 2021 Metromotive

#include "ZSC31014.h"
#include "DigitalOut.h"

namespace metromotive {

ZSC31014::ZSC31014(I2C &i2c, char address, DigitalOut powerPin) : i2c(i2c), address(address), powerPin(powerPin) {
    i2c.frequency(400000);
}

void ZSC31014::startCommandMode() {
    powerPin.write(0);

    wait_us(1000);
 
    printf("Powering the amp and Starting command mode.\n");
    powerPin.write(1);

    wait_us(500);

    writeRegister(StartCommandMode, 0x0000);
}

void ZSC31014::startNormalOperationMode() {
    writeRegister(StartNormalOperationMode, 0x0000);
}

void ZSC31014::writeRegister(Register regAddress, uint16_t value) {
    char command[3] = { regAddress, (char)(value >> 8), (char)(value & 0xFF) };
    if (i2c.write(address, command, 3) != 0) {
        printf("Unable to write to device. Check i2c address and connections.\n");
    }
}

}