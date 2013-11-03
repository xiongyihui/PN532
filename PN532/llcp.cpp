
#include "llcp.h"

// LLCP PDU Type Values
#define PDU_SYMM    0x00
#define PDU_PAX     0x01
#define PDU_CONNECT 0x04
#define PDU_DISC    0x05
#define PDU_CC      0x06
#define PDU_DM      0x07
#define PDU_I       0x0c
#define PDU_RR      0x0d

uint8_t LLCP::SYMM_PDU[2] = {0, 0};

int8_t LLCP::activate(uint16_t timeout)
{
	return link.activateAsTarget();
}

int8_t LLCP::waitForConnect(uint16_t timeout)
{
	uint8_t type;
	uint8_t buf[3];

	// Get CONNECT PDU
	do {
		if (2 > link.read(buf, sizeof(buf))) {
			return -1;
		}

		type = getPDUType(buf);
		if (PDU_CONNECT == type) {
			break;
		} else if (PDU_SYMM == type) {
			if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
				return -2;
			}
		} else {
			return -3;
		}

	} while (1);

	// Put CC PDU
	ssap = getDSAP(buf);
	dsap = getSSAP(buf);
	buf[0] = (dsap << 2) + (PDU_CC >> 2);
	buf[1] = ((PDU_CC & 0x3) << 2) + ssap;
	if (!link.write(buf, 2)) {
		return -2;
	}

	state = LLCP_CONNECTED;
	return 1;
}

bool LLCP::write(const uint8_t *buf, uint8_t len)
{
	uint8_t type;
	uint8_t header[3];

	if (2 != link.read(header, sizeof(header))) {
		return false;
	}

	header[0] = (dsap << 2) + (PDU_RR >> 2);
	header[1] = ((PDU_RR & 0x3) << 2) + ssap;
	header[2] = 0x01;		// sequence
	if (!link.write(header, 3, buf, len)) {
		return false;
	}


	return true;
}

int16_t LLCP::read(uint8_t *buf, uint8_t length)
{
	uint8_t type;
	uint16_t status;

	if (LLCP_CONNECTED != state) {
		return -1;
	}

	// Get INFO PDU
	do {
		status = link.read(buf, length);
		if (2 > status) {
			return -1;
		}

		type = getPType(buf);
		if (PDU_I == type) {
			break;
		} else if (PDU_SYMM == type) {
			if (!link.write(SYMM_PDU, sizeof(SYMM_PDU))) {
				return -2;
			}
		} else {
			return -3;
		}

	} while (1);

	uint8_t length = status - 3;
	ssap = getDSAP(buf);
	dsap = getSSAP(buf);
	buf[0] = (dsap << 2) + (PDU_RR >> 2);
	buf[1] = ((PDU_RR & 0x3) << 2) + ssap;
	buf[2] = 0x01;		// sequence
	if (!link.write(buf, 3)) {
		return -2;
	}

	memcpy(buf, buf + 3, length);
	return length;
}

inline uint8_t getPType(const uint8_t *buf)
{
	return ((buf[0] & 0x3) << 2) + (buf[1] >> 6);
}

inline uint8_t getSSAP(const uint8_t *buf)
{
	return  buf[1] & 0x3f;
}

inline uint8_t getDSAP(const uint8_t *buf)
{
	return buf[0] >> 2;
}

uint8_t LLCP::packPDU(uint8_t type, uint8_t *buf, uint8_t length)
{

}