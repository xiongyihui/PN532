
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
  
  NdefMessage message = NdefMessage();
  message.addUriRecord("http://www.seeedstudio.com");
  int messageSize = message.getEncodedSize();
  if (messageSize > sizeof(ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
  }

  message.encode(ndefBuf);
  
  // comment out this command for no ndef message
  nfc.setNdefFile(ndefBuf, messageSize);
  
  nfc.init();
}

void loop(){
    uint8_t uid[3] = { 0x12, 0x34, 0x56 };
 
    // uncomment for overriding ndef in case a write to this tag occured
    // nfc.setNdefFile(ndefBuf, messageSize); 
    nfc.emulate(uid);
    
    if(nfc.writeOccured()){
       Serial.println("\nWrite occured !");
       uint8_t* tag_buf;
       uint16_t length;
       
       nfc.getContent(&tag_buf, &length);
       NdefMessage msg = NdefMessage(tag_buf, length);
       msg.print();
    }

    delay(1000);
}
