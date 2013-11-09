
#include "SPI.h"
#include "PN532_SPI.h"
#include "emulatetag.h"
#include "NdefMessage.h"

PN532_SPI pn532spi(SPI, 10);
EmulateTag nfc(pn532spi);

uint8_t ndefBuf[120];

void setup()
{
  Serial.begin(115200);
  Serial.println("------- Emulate Tag --------");
  
  nfc.init();
}

void loop(){
    NdefMessage message = NdefMessage();
    message.addUriRecord("http://www.seeedstudio.com");
    int messageSize = message.getEncodedSize();
    if (messageSize > sizeof(ndefBuf)) {
        Serial.println("ndefBuf is too small");
        while (1) { }
    }

    message.encode(ndefBuf);
  
    uint8_t uid[3] = { 0x12, 0x34, 0x56}  ;
  
    nfc.emulate(ndefBuf, messageSize, uid);

    delay(1000);
}
