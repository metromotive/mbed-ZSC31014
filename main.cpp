// Copyright 2021, Metromotive

#include "mbed.h"
#include "PinNames.h"
#include "ZSC31014.h"

// Calibration for LCA.1

using namespace metromotive;

static DigitalOut enable(PC_13);
static I2C amp(PB_9, PB_8);
static ZSC31014 lca(amp, 0x28, enable);

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
