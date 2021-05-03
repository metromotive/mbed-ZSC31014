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

    wait_us(500);
 
    powerPin.write(1);

    wait_us(500);

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

void ZSC31014::setCustomerID0(uint16_t customerID0) {
    this->write(WriteCust_ID0, customerID0);
}

void ZSC31014::setCustomerID1(uint16_t customerID1) {
    this->write(WriteCust_ID1, customerID1);
}

void ZSC31014::setCustomerID2(uint16_t customerID2) {
    this->write(WriteCust_ID2, customerID2);
}

char ZSC31014::getI2CAddress() {
    return (this->read(ReadZMDI_Config2) >> 3) & 0x7F;
}

void ZSC31014::setI2CAddress(char i2cAddress) {
    struct ZMDIConfig2 zmdiConfig2 = this->getZMDIConfig2();

    zmdiConfig2.slaveAddress = i2cAddress;

    this->setZMDIConfig2(zmdiConfig2);
}

struct ZSC31014::FactoryID ZSC31014::getFactoryID() {
    uint16_t customerID0 = this->getCustomerID0();
    uint16_t customerID1 = this->getCustomerID1();
    uint16_t customerID2 = this->getCustomerID2();

    struct FactoryID result;

    result.lotNumber = (customerID2 << 3) | (customerID0 >> 13);
    result.waferNumber = (customerID0 >> 8) & 0x1F;
    result.waferXCoordinate = customerID0 & 0x7F;
    result.waferYCoordinate = customerID1;

    return result;
}

struct ZSC31014::ZMDIConfig1 ZSC31014::getZMDIConfig1() {
    return this->decodeZMDIConfig1(this->read(ReadZMDI_Config1));
}

struct ZSC31014::ZMDIConfig2 ZSC31014::getZMDIConfig2() {
    return this->decodeZMDIConfig2(this->read(ReadZMDI_Config2));
}

struct ZSC31014::BridgeConfig ZSC31014::getBridgeConfig() {
    return this->decodeBridgeConfig(this->read(ReadB_Config));
}

void ZSC31014::setZMDIConfig1(struct ZMDIConfig1 zmdiConfig1) {
    this->write(WriteZMDI_Config1, this->encodeZMDIConfig1(zmdiConfig1));
}

void ZSC31014::setZMDIConfig2(struct ZMDIConfig2 zmdiConfig2) {
    this->write(WriteZMDI_Config2, this->encodeZMDIConfig2(zmdiConfig2));
}

void ZSC31014::setBridgeConfig(struct BridgeConfig bridgeConfig) {
    this->write(WriteB_Config, this->encodeBridgeConfig(bridgeConfig));
}

uint16_t ZSC31014::getOffset() {
    return this->read(ReadOffset_B);
}

void ZSC31014::setOffset(uint16_t offset) {
    this->write(WriteOffset_B, offset);
}

void ZSC31014::dumpEEPROM() {
    printf("EEPROM Values\n");
    for (int i = 0; i <= 0x13; i ++) {
        int value = this->read((Command)i);
        printf("0x%02x: 0x%04x\n", i, value);
        wait_us(10);
    }
}

uint16_t ZSC31014::read(Command command) {
    this->write(command);

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

void ZSC31014::write(Command command, uint16_t value) {
    char packet[3] = { command, (char)(value >> 8), (char)(value & 0xFF) };

    if (this->i2c.write(address, packet, 3) != 0) {
        printf("Unable to write to device. Check i2c address and connections.\n");
    }
}

struct ZSC31014::ZMDIConfig1 ZSC31014::decodeZMDIConfig1(uint16_t rawValue) {
    struct ZMDIConfig1 result;

    result.clockSpeed = ((rawValue >> 3) & 0b1) ? ClockSpeed::mhz1 : ClockSpeed::mhz4;
    result.commType = ((rawValue >> 4) & 0b1) ? CommType::spi : CommType::i2c;
    result.sleepMode = ((rawValue >> 5) & 0b1) ? OperationMode::onDemand : OperationMode::continuous;

    switch ((rawValue >> 6) & 0b11) {
        case 0b00:
            result.updateRate = UpdateRate::fastest;
            break;
        
        case 0b01:
            result.updateRate = UpdateRate::faster;
            break;
        
        case 0b10:
            result.updateRate = UpdateRate::slower;
            break;

        case 0b11:
            result.updateRate = UpdateRate::slowest;
            break;
    }

    result.sotCurve = ((rawValue >> 9) & 0b1) ? SOTCurve::sShaped : SOTCurve::parabolic;
    result.tcoSign = ((rawValue >> 10) & 0b1) ? Polarity::negative : Polarity::positive;
    result.tcgSign = ((rawValue >> 11) & 0b1) ? Polarity::negative : Polarity::positive;
    result.sotBridge = ((rawValue >> 12) & 0b1) ? Polarity::negative : Polarity::positive;
    result.sotTCO = ((rawValue >> 13) & 0b1) ? Polarity::negative : Polarity::positive;
    result.sotTCG = ((rawValue >> 14) & 0b1) ? Polarity::negative : Polarity::positive;
    result.sotT = ((rawValue >> 15) & 0b1) ? Polarity::negative : Polarity::positive;

    return result;
}

struct ZSC31014::ZMDIConfig2 ZSC31014::decodeZMDIConfig2(uint16_t rawValue) {
    struct ZMDIConfig2 result;

    result.spiPolarity = ((rawValue >> 0) & 0b1) ? Polarity::positive : Polarity::negative;
    result.enableSensorConnectionCheck = (rawValue >> 1) & 0b1;
    result.enableSensorShortCheck = (rawValue >> 2) & 0b1;
    result.slaveAddress = (rawValue >> 3) & 0b1111111;
    result.lockAddress = ((rawValue >> 10) & 0b111) == 0b011;
    result.lockEEPROM = ((rawValue >> 13) & 0b111) == 0b011;

    return result;
}

struct ZSC31014::BridgeConfig ZSC31014::decodeBridgeConfig(uint16_t rawValue) {
    struct BridgeConfig result;

    result.preAmpOffset = ((rawValue >> 0) & 0b1111);
    
    switch ((rawValue >> 4) & 0b111) {
        case 0b000:
            result.preAmpGain = PreAmpGain::x1_5;
            break;

        case 0b100:
            result.preAmpGain = PreAmpGain::x3;
            break;
            
        case 0b001:
            result.preAmpGain = PreAmpGain::x6;
            break;
            
        case 0b101:
            result.preAmpGain = PreAmpGain::x12;
            break;
            
        case 0b010:
            result.preAmpGain = PreAmpGain::x24;
            break;
            
        case 0b110:
            result.preAmpGain = PreAmpGain::x48;
            break;
            
        case 0b011:
            result.preAmpGain = PreAmpGain::x96;
            break;
            
        case 0b111:
            result.preAmpGain = PreAmpGain::x192;
            break;
    }

    result.polarity = ((rawValue >> 7) & 0b1) ? Polarity::positive : Polarity::negative;
    result.useLongIntegration = (rawValue >> 8) & 0b1;
    result.useBSink = (rawValue >> 9) & 0b1;

    switch ((rawValue >> 10) & 0b11) {
        case 0b10:
            result.mux = MuxMode::fullBridge;
            break;
        
        case 0b11:
            result.mux = MuxMode::halfBridge;
            break;
        
        default:
            printf("ERROR: Invalid Mux Mode read from bridge config!\n");
            break;
    }

    result.disableNulling = (rawValue >> 12) & 0b1;

    return result;
}

uint16_t ZSC31014::encodeZMDIConfig1(struct ZMDIConfig1 zmdiConfig1) {
    uint16_t result = 0x0000;

    int idtReserved1Bits = 0b001;
    int clockSpeedBit = (zmdiConfig1.clockSpeed == ClockSpeed::mhz1) ? 0b1 : 0b0;
    int commTypeBit = (zmdiConfig1.commType == CommType::spi) ? 0b1 : 0b0;
    int sleepModeBit = (zmdiConfig1.sleepMode == OperationMode::onDemand) ? 0b1 : 0b0;
    int updateRateBits;

    switch (zmdiConfig1.updateRate) {
        case UpdateRate::slowest:
            updateRateBits = 0b11;
            break;

        case UpdateRate::slower:
            updateRateBits = 0b10;
            break;
            
        case UpdateRate::faster:
            updateRateBits = 0b01;
            break;
            
        case UpdateRate::fastest:
            updateRateBits = 0b00;
            break;
            
    }

    int idtReserved2Bit = 0b0;
    int sotCurveBit = (zmdiConfig1.sotCurve == SOTCurve::sShaped) ? 0b1 : 0b0;
    int tcoSignBit = (zmdiConfig1.tcoSign == Polarity::negative) ? 0b1 : 0b0;
    int tcgSignBit = (zmdiConfig1.tcgSign == Polarity::negative) ? 0b1 : 0b0;
    int sotBridgeBit = (zmdiConfig1.sotBridge == Polarity::negative) ? 0b1 : 0b0;
    int sotTCOBit = (zmdiConfig1.sotTCO == Polarity::negative) ? 0b1 : 0b0;
    int sotTCGBit = (zmdiConfig1.sotTCG == Polarity::negative) ? 0b1 : 0b0;
    int sotTBit = (zmdiConfig1.sotT == Polarity::negative) ? 0b1 : 0b0;

    result |= idtReserved1Bits << 0;
    result |= clockSpeedBit << 3;
    result |= commTypeBit << 4;
    result |= sleepModeBit << 5;
    result |= updateRateBits << 6;
    result |= idtReserved2Bit << 8;
    result |= sotCurveBit << 9;
    result |= tcoSignBit << 10;
    result |= tcgSignBit << 11;
    result |= sotBridgeBit << 12;
    result |= sotTCOBit << 13;
    result |= sotTCGBit << 14;
    result |= sotTBit << 15;

    return result;
}

uint16_t ZSC31014::encodeZMDIConfig2(struct ZMDIConfig2 zmdiConfig2) {
    uint16_t result = 0x0000;

    int spiPolarityBit = (zmdiConfig2.spiPolarity == Polarity::positive) ? 0b1 : 0b0;
    int enableSensorConnectionCheckBit = zmdiConfig2.enableSensorConnectionCheck ? 0b1 : 0b0;
    int enableSensorShortCheckBit = zmdiConfig2.enableSensorShortCheck ? 0b1 : 0b0;
    int lockAddressBits = zmdiConfig2.lockAddress ? 0b011 : 0b000;
    int lockEEPROMBits = zmdiConfig2.lockEEPROM ? 0b011: 0b000;

    result |= spiPolarityBit << 0b0;
    result |= enableSensorConnectionCheckBit << 1;
    result |= enableSensorShortCheckBit << 2;
    result |= zmdiConfig2.slaveAddress << 3;
    result |= lockAddressBits << 10;
    result |= lockEEPROMBits << 13;

    return result;
}

uint16_t ZSC31014::encodeBridgeConfig(struct BridgeConfig bridgeConfig) {
    uint16_t result = 0x0000;

    int preAmpGainBits;

    switch (bridgeConfig.preAmpGain) {
        case PreAmpGain::x1_5:
            preAmpGainBits = 0b000;
            break;

        case PreAmpGain::x3:
            preAmpGainBits = 0b100;
            break;
            
        case PreAmpGain::x6:
            preAmpGainBits = 0b001;
            break;
            
        case PreAmpGain::x12:
            preAmpGainBits = 0b101;
            break;
            
        case PreAmpGain::x24:
            preAmpGainBits = 0b010;
            break;
            
        case PreAmpGain::x48:
            preAmpGainBits = 0b110;
            break;
            
        case PreAmpGain::x96:
            preAmpGainBits = 0b011;
            break;
            
        case PreAmpGain::x192:
            preAmpGainBits = 0b111;
            break;
            
    }

    int polarityBit = (bridgeConfig.polarity == Polarity::positive) ? 0b1 : 0b0;
    int useLongIntegrationBit = bridgeConfig.useLongIntegration ? 0b1 : 0b0;
    int useBSinkBit = bridgeConfig.useBSink ? 0b1 : 0b0;
    int muxBits = (bridgeConfig.mux == MuxMode::fullBridge) ? 0b10 : 0b11;
    int disableNullingBit = bridgeConfig.disableNulling ? 0b1 : 0b0;
    int idtReservedBits = 0b000;

    result |= bridgeConfig.preAmpOffset << 0;
    result |= preAmpGainBits << 4;
    result |= polarityBit << 7;
    result |= useLongIntegrationBit << 8;
    result |= useBSinkBit << 9;
    result |= muxBits << 10;
    result |= disableNullingBit << 12;
    result |= idtReservedBits << 13;

    return result;
}

} // namespace metromotive