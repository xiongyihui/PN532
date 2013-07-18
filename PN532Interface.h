

#ifndef __PN532_INTERFACE_H__
#define __PN532_INTERFACE_H__

#include <Arduino.h>

#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)

#define PN532_HOSTTOPN532                   (0xD4)
#define PN532_PN532TOHOST                   (0xD5)


class PN532Interface {
public:
    virtual void begin() = 0;
    virtual void wakeup() = 0;
    virtual int8_t writeCommand(uint8_t buf[], uint8_t len) = 0;
    virtual int8_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout = 1000) = 0; 
};

#endif

