// llcp_test.ino
// get a message from android

#include "SPI.h"
#include "PN532_SPI.h"
#include "llcp.h"

PN532_SPI pn532spi(SPI, 10);
LLCP nfc(pn532spi);

uint8_t nfcBuf[64];

void setup()
{
    Serial.begin(115200);
    Serial.println("-------Peer to Peer--------");
}


void loop()
{
    do
    {
        if (nfc.activate() > 0)
        {
            int16_t status = nfc.waitForConnection();
            if (0 >= status)
            {
                Serial.print("Failed to connect, return ");
                Serial.println(status);
                break;
            }

            if (0 >= nfc.read(nfcBuf, sizeof(nfcBuf)))
            {
                Serial.println("Failed to read");
                break;
            }

            uint8_t response[] = {0x10, 0x81, 0, 0, 0, 0};
            if (0 >= nfc.write(response, sizeof(response)))
            {
                Serial.println("Failed to write");
                break;
            }

            nfc.waitForDisconnection();

        }
    }
    while (0);
    delay(3000);

}

