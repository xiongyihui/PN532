
#include "PN532SPI.h"

#include <SPI.h>

#define STATUS_READ     2
#define DATA_WRITE      1
#define DATA_READ       3

static const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

PN532SPI::PN532SPI(uint8_t ss)
{
    _ss = ss;
}

void PN532SPI::begin()
{
    pinMode(_ss, OUTPUT);
    
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);  // PN532 only supports mode0
    SPI.setBitOrder(LSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4); // set clock 4MHz(max: 5MHz)
}

void PN532SPI::wakeup()
{
    digitalWrite(_ss, LOW);
    delay(4);
    digitalWrite(_ss, HIGH);
}

int8_t PN532SPI::writeCommand(uint8_t buf[], uint8_t len)
{
    writeFrame(buf, len);
    
    uint8_t timeout = 100;
    while (!isReady()) {
        delay(1);
        timeout--;
        if (0 == timeout) {
            return -1;
        }
    }
    return readAckFrame();
}

int8_t PN532SPI::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    while (!isReady()) {
        delay(1);
        if (timeout > 0) {
            time++;
            if (time >= timeout) {
                return -1;
            }
        }
    }
    
    return readFrame(buf, len);
}

static inline void write(uint8_t data)
{
    SPI.transfer(data);
}

static inline uint8_t read()
{
    return SPI.transfer(0);
}

boolean PN532SPI::isReady()
{
    digitalWrite(_ss, LOW);
    
    write(STATUS_READ);
    uint8_t status = read() & 1;
    digitalWrite(_ss, HIGH);
    return status;
}

void PN532SPI::writeFrame(uint8_t buf[], uint8_t len)
{
    digitalWrite(_ss, LOW);
    delay(2);               // wake up PN532
    
    write(DATA_WRITE);
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);
    
    uint8_t length = len + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);         // checksum of length
    
    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
    
    Serial.print("write: ");
    
    for (uint8_t i = 0; i < len; i++) {
        write(buf[i]);
        sum += buf[i];
        
        Serial.print(' ');
        Serial.print(buf[i], HEX);
    }
    
    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);
    
    digitalWrite(_ss, HIGH); 
    
    Serial.print('\n');
}

int8_t PN532SPI::readFrame(uint8_t buf[], uint8_t len)
{
    digitalWrite(_ss, LOW);
    delay(2);
    
    Serial.print("read: ");
    
    write(DATA_READ);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = read();
        Serial.print(' ');
        Serial.print(buf[i], HEX);
    }
    
    digitalWrite(_ss, HIGH);
    
    Serial.print('\n');
    
    return 0;
}

int8_t PN532SPI::readAckFrame()
{
    uint8_t ackBuf[6];
    readFrame(ackBuf, 6);
    
    return memcmp(ackBuf, PN532_ACK, 6);
}
