
#include <SPI.h>
#include <PN532_SPI.h>
#include "mac_link.h"


PN532_SPI pn532spi(SPI, 10);
MACLink link(pn532spi);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("------- NFC Peer to Peer -----");
}

boolean isConnectPDU(uint8_t *buf)
{
    uint8_t dsap = buf[0] >> 2;
    uint8_t ssap = buf[1] & 0xC;
    uint8_t ptype = (buf[0] & 0x3) << 2 + buf[1] >> 6;

    return ptype == 0x4;
}

uint8_t rwbuf[128];
const uint8_t SYMM_PDU[] = {0, 0};

void loop()
{
    // put your main code here, to run repeatedly:

    if (link.activateAsTarget()) {

        server();
    }

    delay(3000);
}

/*
<---SYMM
--->SYMM
...
<---CONN
--->CC
<---SYMM
--->SYMM
...
<---INFO
--->RR
<---SYMM
--->INFO
<---RR
--->SYMM
<---DISC
--->DM
*/
void server(void)
{
    uint8_t dsap;
    uint8_t ssap;
    uint8_t ptype;

    do {
        if (2 > link.read(rwbuf, sizeof(rwbuf))) {
            Serial.println("failed to read PDU\n");
            return;
        }

        dsap = rwbuf[0] >> 2;
        ssap = rwbuf[1] & 0x3F;
        ptype = ((rwbuf[0] & 0x3) << 2) + (rwbuf[1] >> 6);

        if (0x4 == ptype) {
            break;
        }

        if (0 == rwbuf[0] && 0 == rwbuf[0]) {
            if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
                Serial.println("failed to write SYMM PDU");
                return;
            }
        } else {
            return;
        }
    } while (1);


    // connection complete
    rwbuf[0] = (ssap << 2) + 0x1;
    rwbuf[1] = (0x2 << 6) + dsap;
    link.write(rwbuf, 2);

    do {
        if (2 > link.read(rwbuf, sizeof(rwbuf)) < 0) {
            return;
        }
        if (rwbuf[0] == 0 && rwbuf[1] == 0) {
            link.write(SYMM_PDU, sizeof(SYMM_PDU));
        } else {
            break;
        }
    } while (1);

    rwbuf[0] = (ssap << 2) + 0x3;
    rwbuf[1] = (0x1 << 6) + dsap;
    rwbuf[2] = 0x01;
    link.write(rwbuf, 3);
    if (2 > link.read(rwbuf, sizeof(rwbuf))) {
        return;
    }

    uint8_t snep_res[] = {0, 0, 0, 0x10, 0x81, 0, 0, 0, 0};
    snep_res[0] = (ssap << 2) + 0x3;
    snep_res[1] = dsap;
    snep_res[2] = 0x01;
    link.write(snep_res, sizeof(snep_res));
    if (2 > link.read(rwbuf, sizeof(rwbuf))) {
        return;
    }

#if 0
    rwbuf[0] = (ssap << 2) + 0x1;
    rwbuf[1] = (0x1 << 6) + dsap;
    //    rwbuf[2] = 0x01;
    link.write(rwbuf, 2);
#else
    link.write(SYMM_PDU, sizeof(SYMM_PDU));
#endif


    if (2 > link.read(rwbuf, sizeof(rwbuf))) {
        return;
    }
    rwbuf[0] = (ssap << 2) + 0x1;
    rwbuf[1] = (0x3 << 6) + dsap;
    rwbuf[2] = 0x0;
    link.write(rwbuf, 3);
}

void client(void)
{
    if (2 > link.read(rwbuf, sizeof(rwbuf))) {
        Serial.println("failed to read PDU\n");
        return;
    }

    rwbuf[0] = 0x11;
    rwbuf[1] = 0x20;
    link.write(rwbuf, 2);

    do {
        if (2 > link.read(rwbuf, sizeof(rwbuf))) {
            return;
        }

        if (0x81 == rwbuf[0] && 0x84 == rwbuf[1]) {
            break;
        }
        link.write(SYMM_PDU, sizeof(SYMM_PDU));
    } while (1);

    uint8_t hello[] = {0x13, 0x20, 0x0, 0x10, 0x2, 0x0, 0x0, 0x0, 0x18, 0xD2, 0xA, 0xB, 0x74,
                       0x65, 0x78, 0x74, 0x2F, 0x70, 0x6C, 0x61, 0x69, 0x6E, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64
                      };
    link.write(hello, sizeof(hello));
    link.read(rwbuf, sizeof(rwbuf));

    link.write(SYMM_PDU, sizeof(SYMM_PDU));
    link.read(rwbuf, sizeof(rwbuf));

    rwbuf[0] = 0x13;
    rwbuf[1] = 0x60;
    rwbuf[2] = 0x01;
    link.write(rwbuf, 3);
    link.read(rwbuf, sizeof(rwbuf));

    rwbuf[0] = 0x11;
    rwbuf[1] = 0x60;
    link.write(rwbuf, 2);
    link.read(rwbuf, sizeof(rwbuf));
}
