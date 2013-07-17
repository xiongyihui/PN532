
#ifndef __PN532_SPI_H__
#define __PN532_SPI_H__

#include "PN532Interface.h"

class PN532SPI : public PN532Interface {
public:
    PN532SPI(uint8_t ss);
    
    void begin();
    void wakeup();
    int8_t writeCommand(uint8_t buf[], uint8_t len);
    int8_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    
private:
    uint8_t _ss;
    
    boolean isReady();
    void writeFrame(uint8_t buf[], uint8_t len);
    int8_t readFrame(uint8_t buf[], uint8_t len);
    int8_t readAckFrame();
};

#endif
