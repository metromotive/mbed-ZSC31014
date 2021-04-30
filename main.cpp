// Copyright 2021, Metromotive

#include "mbed.h"
#include "PinNames.h"
#include "ZSC31014.h"

// Calibration for LCA.1

// Read the customer ID fields
// Set the B_Config register to higest gain, offset that still allows small negative reading.
// Set the I2C address
// Set the customer ID fields?

using namespace metromotive;

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

static DigitalOut enable(PC_13);
static I2C amp(PB_9, PB_8);

const char address7Bit = 0x28;
const char address = 0x28 << 1;

static ZSC31014 lca(amp, address7Bit, enable);

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);

    lca.startCommandMode();

    int customerID0 = lca.getCustomerID0();
    int customerID1 = lca.getCustomerID1();
    int customerID2 = lca.getCustomerID2();

    printf("Customer IDs are %d, %d, %d.\n", customerID0, customerID1, customerID2);
}

// int main()
// {
//     // Initialise the digital pin LED1 as an output
//     DigitalOut led(LED1);

//     led.write(1);

//     lca.startCommandMode();

//     int customerID0 = lca.getCustomerID0();
//     int customerID1 = lca.getCustomerID1();
//     int customerID2 = lca.getCustomerID2();

//     printf("Customer IDs are %d, %d, %d.\n", customerID0, customerID1, customerID2);
//     // printf("Got here.\n");

//     led.write(0);
// }
