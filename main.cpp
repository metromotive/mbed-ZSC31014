// Copyright 2021, Metromotive

#include "mbed.h"
#include "PinNames.h"
#include "objects_gpio.h"
#include "ZSC31014.h"

// Calibration for LCA.1

// Read the customer ID fields
// Set the B_Config register to higest gain, offset that still allows small negative reading.
// Set the I2C address
// Set the customer ID fields?



// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

static DigitalOut enable(D24);
static I2C amp(D25, D27);
//static I2C amp(QWIIC_SDA, QWIIC_SCL);

const char address = 0x28 << 1;

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);

    amp.frequency(400000);

    printf("\n\n\nDepowering the amp.\n");
    enable.write(0);

    wait_us(1000);
 
    // printf("Powering the amp and Starting command mode.\n");
    enable.write(1);

    // wait_us(500);

    // char command1[3] = { StartCommandMode, 0, 0 };
    // if (amp.write(address, command1, 3) != 0) {
    //     printf("Unable to write.\n");
    // }
 
    while (true) {
        led = !led;
        char readData[3] = { 0 };

        if (amp.read(address, readData, 3) != 0) {
            printf("Unable to read.\n");
        } else {
            printf("Readed %d, %d, %d from device.\n", readData[0], readData[1], readData[2]);
        }

        // printf("Trying to read customer id 0\n");
        // char command2[3] = { ReadCust_ID0, 0, 0 };
        // if (amp.write(address, command2, 3) != 0) {
        //     printf("Unable to write.\n");
        // }

        // //char readData[3] = { 0 };
        // if (amp.read(address, readData, 3) != 0) {
        //     printf("Unable to read.\n");
        // } else {
        //     printf("Readed %d, %d, %d from customer ID 0.\n", readData[0], readData[1], readData[2]);
        // }

        ThisThread::sleep_for(BLINKING_RATE);
    }
}
