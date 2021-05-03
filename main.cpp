// Copyright 2021, Metromotive

#include "I2C.h"
#include "ThisThread.h"
#include "mbed.h"
#include "ZSC31014.h"

// Calibration for LCA.1

#define MM_ID 0x4D6D

using namespace metromotive;

static DigitalOut enable(PC_13);
static I2C i2c(PB_9, PB_8);
static ZSC31014 lca(i2c, 0x3F, enable);

int main()
{
    lca.startCommandMode();

    unsigned int customerID0 = lca.getCustomerID0();
    unsigned int customerID1 = lca.getCustomerID1();
    unsigned int customerID2 = lca.getCustomerID2();
    char i2cAddress = lca.getI2CAddress();

    struct ZSC31014::FactoryID factoryID = lca.getFactoryID();

    printf(
        "Factory ID:\nLot # %d\nWafer # %d\nCoordinate (x/y): %d/%d,\nI2C Address: 0x%x\n",
        factoryID.lotNumber,
        factoryID.waferNumber,
        factoryID.waferXCoordinate, 
        factoryID.waferYCoordinate,
        i2cAddress
    );

    customerID0 = MM_ID;
    customerID1 = 0x1CA1;

    int serialNumber;
    printf("\nEnter serial #: ");
    scanf("%d", &serialNumber);

    if (serialNumber % 2 == 1) {
        // Odd serial numbers are forward.
        i2cAddress = 0x3F;
    } else {
        // Even serial numbers are aft.
        i2cAddress = 0x3A;
    }

    customerID2 = serialNumber;

    lca.setCustomerID0(customerID0);

    ThisThread::sleep_for(15ms);

    lca.setCustomerID1(customerID1);

    ThisThread::sleep_for(15ms);

    lca.setCustomerID2(customerID2);

    ThisThread::sleep_for(15ms);

    struct ZSC31014::ZMDIConfig1 zmdiConfig1 = lca.getZMDIConfig1();

    zmdiConfig1.updateRate = ZSC31014::UpdateRate::fastest;

    lca.setZMDIConfig1(zmdiConfig1);

    ThisThread::sleep_for(15ms);

    struct ZSC31014::ZMDIConfig2 zmdiConfig2 = lca.getZMDIConfig2();

    zmdiConfig2.enableSensorConnectionCheck = 1;
    zmdiConfig2.enableSensorShortCheck = 1;
    zmdiConfig2.slaveAddress = 0x3F;
    zmdiConfig2.lockAddress = true;
    zmdiConfig2.lockEEPROM = false;

    lca.setZMDIConfig2(zmdiConfig2);

    ThisThread::sleep_for(15ms);

    struct ZSC31014::BridgeConfig bridgeConfig = lca.getBridgeConfig();

    bridgeConfig.disableNulling = false;
    bridgeConfig.mux = ZSC31014::MuxMode::fullBridge;
    bridgeConfig.useBSink = true;
    bridgeConfig.useLongIntegration = true;
    bridgeConfig.preAmpGain = ZSC31014::PreAmpGain::x192;
    bridgeConfig.preAmpOffset = 0b0001;

    lca.setBridgeConfig(bridgeConfig);

    ThisThread::sleep_for(15ms);

    lca.setOffset(0xE400);

    ThisThread::sleep_for(15ms);

    lca.startNormalOperationMode();

    ThisThread::sleep_for(15ms);

    printf("Wrote customer IDs and started normal operation mode.\n");

    while(1) {
        char reading[4] = {0, 0, 0, 0};
        i2c.read(i2cAddress << 1, reading, 3);

        printf("Read %d %d %d %d\n", reading[0], reading[1], reading[2], reading[3]);

        ThisThread::sleep_for(1000ms);
    }
}
