

#ifndef __PN532_INTERFACE_H__
#define __PN532_INTERFACE_H__

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

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
    
    /**
    * @brief    write a command and check ack
    * @param    buf     command to write, not contain prefix and suffix
    * @param    len     lenght of command
    * @return   0       success
    *           not 0   failed
    */
    virtual int8_t writeCommand(uint8_t buf[], uint8_t len) = 0;
    
    /**
    * @brief    read the response of a command
    * @param    buf     to contain the response data
    * @param    len     lenght to read
    * @param    timeout max time to wait, 0 means no timeout
    * @return   0       success
    *           not 0   failed
    */
    virtual int8_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout = 1000) = 0; 
};

#endif

