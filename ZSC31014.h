// Copyright 2021 Metromotive

#ifndef ZSC13014_H
#define ZSC13014_H

namespace metromotive {

#include "mbed.h"

class ZSC13014 {
public:
    ZSC13014(I2C &i2c, char address);
    
    enum ClockSpeed {
        mhz4 = 0,
        mhz1 = 1
    };

    enum CommType {
        i2c = 0,
        spi = 1
    };

    enum OperationMode {
        continuous = 0,
        onDemand = 1
    };

    enum UpdateRate { // 1MHz clock / 4MHz clock
        fastest = 0,  //    1.6ms   /   0.5ms
        faster = 1,   //    5.0ms   /   1.5ms
        slower = 2,   //   25.0ms   /   6.5ms
        slowest = 3   //  125.0ms   /  32.0ms
    };

    enum SOTCurve {
        parabolic = 0,
        sShaped = 1
    };

    enum Polarity {
        positive = 0,
        negative = 1
    };

    enum MuxMode {
        fullBridge = 0b10,
        halfBridge = 0b11
    }

    enum PreAmpGain {
        x1_5 = 0b000,
        x3   = 0b100,
        x6   = 0b001,
        x12  = 0b101,
        x24  = 0b010,
        x48  = 0b110,
        x96  = 0b011,
        x192 = 0b111
    };

    struct ZMDIConfig1 {
    	ClockSpeed clockSpeed;
    	CommType commType;
    	OperationMode sleepMode;
    	UpdateRate updateRate;
    	SOTCurve sotCurve;
    	Polarity tcoSign;
    	Polarity tcgSign;
    	Polarity sotBridge;
    	Polarity sotTCO;
    	Polarity sotTCG;
    	Polarity sotT;
    };

    struct ZMDIConfig2 {
    	Polarity spiPolarity;
    	bool enableSensorConnectionCheck;
    	bool enableSensorShortCheck;
    	char slaveAddress;
    	bool lockAddress;
    	bool lockEEPROM;
    };

    struct BridgeConfig {
    	bool disableNulling;
    	MuxMode mux;
    	bool useBSink;
    	bool useLongIntegration;
    	Polarity polarity;
    	PreAmpGain preAmpGain;
    	PreAmpOffset preAmpOffset;
    };

    struct FactoryID {
        int lotNumber;
        int waferNumber;
        int waferYCoordinate;
        int waferXCoordinate;
    };

    // Mode Changes
    void startCommandMode();
    void startNormalOperationMode();
    
    // ID Fields (no impact on operation)
    struct FactoryID getFactoryID();

    uint16_t getCustomerID0();
    uint16_t getCustomerID1();
    uint16_t getCustomerID2();

    void setCustomerID0(uint16_t customerID0);
    void setCustomerID1(uint16_t customerID1);
    void setCustomerID2(uint16_t customerID2);
    
    // Configuration (can typically be set without first measuring individual unit)
    ClockSpeed getClockSpeed();
    CommType getCommType();
    OperationMode getOperationMode();
    UpdateRate getUpdateRate();
    Polarity getSPIPolarity();
    char getI2CAddress();
    bool isI2CAddressLocked();
    
    void setClockSpeed(ClockSpeed clockSpeed);
    void setCommType(CommType commType);
    void setOperationMode(OperationMode operationMode);
    void setUpdateRate(UpdateRate updateRate);
    void setSPIPolarity(Polarity spiPolarity);
    void setI2CAddress(char i2cAddress);
    void setI2CAddressLocked(bool locked);
    
    // Analog correction settings (ditto)
    int getPreAmpOffset();
    PreAmpGain getPreAmpGain();
    
    void setAnalogOffset(int preAmpOffset);
    void setAnalogGain(PreAmpGain preAmpGain);
    
    // First-order correction settings
    int16_t getOffset();
    int32_t getGain();

    void setOffset(int16_t offset);
    void setGain(int32_t gain);
    
    // Second-order correction settings
    SOTCurve getSecondOrderTemperatureCurve();
    int getOffsetTemperatureCorrectionCoefficient();
    int getGainTemperatureCorrectionCoefficient();
    int getOffsetTemperatureCorrectionSecondOrderTerm();
    int getGainTemperatureCorrectionSecondOrderTerm();
    int getSecondOrderTerm();

    void setSecondOrderTemperatureCurve(SOTCurve sotCurve);
    void setOffsetTemperatureCorrectionCoefficient(int tcg);
    void setGainTemperatureCorrectionCoefficient(int tco);
    void setOffsetTemperatureCorrectionSecondOrderTerm(int sotTCG);
    void setGainTemperatureCorrectionSecondOrderTerm(int sotTCO);
    void setSecondOrderTerm(int sot);
    
    // Danger Zone
    bool isEEPROMLocked();
    void lockEEPROM();

private:
    I2C i2c;
    char address;
    
    // Read/write registers (must be in command mode)
    int16_t readRegister(char address);
    void writeRegister(char address, int16_t value);
    
    struct ZMDIConfig1 getZMDIConfig1();
    struct ZMDIConfig2 getZMDIConfig2();
    struct BridgeConfig getBridgeConfig();

    setZMDIConfig1(struct ZMDIConfig1 zmdiConfig1);
    setZMDIConfig2(struct ZMDIConfig2 zmdiConfig2);
    setBridgeConfig(struct BridgeConfig bridgeConfig);
};

} // namespace metromotive

#endif ZSC13014_H
