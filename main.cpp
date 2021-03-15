/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "PinNames.h"
#include "objects_gpio.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

static DigitalOut v33(D24);
static DigitalOut v323(D35);
static DigitalOut enable(D25);
static I2C amp(D43, D42);
//static I2C amp(QWIIC_SDA, QWIIC_SCL);

enum Commands {
    ReadCust_ID0 = 0x00,
    ReadZMDI_Config1,
    ReadZMDI_Config2,
    ReadOffset_B,
    ReadGain_B,
    ReadTcg,
    ReadTco,
    ReadSOT_Tco,
    ReadSOT_Tcg,
    ReadSOT_Bridge,
    ReadOffset_T,
    ReadGain_T,
    ReadSOT_T,
    ReadTsetl,
    ReadCust_ID1,
    ReadB_Config,
    ReadT_Config,
    ReadOsc_Trim,
    ReadSignature,
    ReadCust_ID2,
    WriteCust_ID0 = 0x40,
    WriteZMDI_Config1,
    WriteZMDI_Config2,
    WriteOffset_B,
    WriteGain_B,
    WriteTcg,
    WriteTco,
    WriteSOT_Tco,
    WriteSOT_Tcg,
    WriteSOT_Bridge,
    WriteOffset_T,
    WriteGain_T,
    WriteSOT_T,
    WriteTsetl,
    WriteCust_ID1,
    WriteB_Config,
    WriteT_Config,
    WriteCust_ID2,
    StartNormalOperationMode = 0x80,
    StartCommandMode = 0xA0
};

const char address = 0x28 << 1;

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);

    amp.frequency(400000);

    // Known unknowns: Is the chip getting VDD?
    // Are the I2C lines hooked up right?

    printf("\n\n\nDepowering the amp.\n");
    enable.write(1);
    v33.write(0);
    v323.write(0);

    wait_us(2000000);

    printf("Powering the amp.\n");
    enable.write(0);
    v33.write(1);
    v323.write(1);

    //wait_us(1000);

    printf("Starting command mode.\n");
    char command1[3] = { StartCommandMode, 0, 0 };
    if (amp.write(address, command1, 3) != 0) {
        printf("Unable to write.\n");
    }
     
    //wait_us(1000);

    while (true) {
        led = !led;
    
        printf("Trying to read customer id 0\n");
        char command2[3] = { ReadCust_ID0, 0, 0 };
        if (amp.write(address, command2, 3) != 0) {
            printf("Unable to write.\n");
        }

        char readData[3] = { 0 };
        if (amp.read(address, readData, 3) != 0) {
            printf("Unable to read.\n");
        } else {
            printf("Readed %d, %d, %d from customer ID 0.\n", readData[0], readData[1], readData[2]);
        }

        ThisThread::sleep_for(BLINKING_RATE);
    }
}
