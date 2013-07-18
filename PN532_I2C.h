
#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#include <Wire.h>
#include "PN532Interface.h"

class PN532_I2C : public PN532Interface {
public:
    PN532_I2C(TwoWire &wire);
    
    void begin();
    void wakeup();
    int8_t writeCommand(uint8_t buf[], uint8_t len);
    int8_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    
private:
    TwoWire* _wire;
    
    boolean isReady();
    void writeFrame(uint8_t buf[], uint8_t len);
    int8_t readFrame(uint8_t buf[], uint8_t len, uint16_t timeout);
    int8_t readAckFrame();
    
    inline uint8_t write(uint8_t data) {
        #if ARDUINO >= 100
            return _wire->write(data);
        #else
            return _wire->send(data);
        #endif
    }
    
    inline uint8_t read() {
        #if ARDUINO >= 100
            return _wire->read();
        #else
            return _wire->receive();
        #endif
    }
};

#endif
