
#include "PN532_I2C.h"
#include "debug.h"

#define PN532_I2C_ADDRESS       (0x48 >> 1)


PN532_I2C::PN532_I2C(TwoWire &wire)
{
    _wire = &wire;
}

void PN532_I2C::begin()
{
    _wire->begin();
}

void PN532_I2C::wakeup()
{
    _wire->beginTransmission(PN532_I2C_ADDRESS); // I2C start
    delay(20);
    _wire->endTransmission();                    // I2C end
}

int8_t PN532_I2C::writeCommand(const uint8_t buf[], uint8_t len)
{
    writeFrame(buf, len);

    return readAckFrame();
}

int8_t PN532_I2C::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    return readFrame(buf, len, timeout);
}

void PN532_I2C::writeFrame(const uint8_t buf[], uint8_t len)
{
    _wire->beginTransmission(PN532_I2C_ADDRESS); // I2C start
    
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);
    
    uint8_t length = len + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);         // checksum of length
    
    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
    
    DMSG("write: ");
    
    for (uint8_t i = 0; i < len; i++) {
        write(buf[i]);
        sum += buf[i];
        
        DMSG_HEX(buf[i]);
    }
    
    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);
    
    _wire->endTransmission();             // I2C end
    
    DMSG('\n');
}

int8_t PN532_I2C::readFrame(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t endtime = millis() + timeout;
    uint8_t status;

    do {
        if (_wire->requestFrom(PN532_I2C_ADDRESS, len + 2)) {
            status = read();
            if (status & 1) {
                break;
            }
        }

        delay(1);
        if ((0 != timeout) && (millis() > endtime)) {
            return -1;
        }
    } while (1); 

    DMSG("read:  ");
    for (uint8_t i=0; i<len; i++) {
        buf[i] = read();
        
        DMSG_HEX(buf[i]);
    }
    
    DMSG('\n');
    
    return 0;
}

int8_t PN532_I2C::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    
    uint8_t ackBuf[6];
    readFrame(ackBuf, 6, 20);
    
    return memcmp(ackBuf, PN532_ACK, 6);
}
