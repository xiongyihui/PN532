
#ifndef __LLCP_H__
#define __LLCP_H__

#include "mac_link.h"

#define LLCP_CONNECTED 		1
#define LLCP_DISCONNECTED	0

class LLCP {
public:
	LLCP(PN532Interface &interface) : link(interface) {
		state = LLCP_DISCONNECTED;
	};

	/**
    * @brief    Actiave PN532 as a target
    * @param    timeout max time to wait, 0 means no timeout
    * @return   > 0     success
    *           = 0     timeout
    *           < 0     failed
    */
	int8_t activate(uint16_t timeout);

	/**
    * @brief    write a packet, the packet should be less than (255 - 2) bytes
    * @param    buf     the packet
    * @param    len     length
    * @return   true    success
    *           false   failed
    */
    bool write(const uint8_t *buf, uint8_t len);

    /**
    * @brief    read a  packet, the packet will be less than (255 - 2) bytes
    * @param    buf     the buffer to contain the packet
    * @param    len     lenght of the buffer
    * @return   >=0     length of the packet 
    *           <0      failed
    */
    int16_t read(uint8_t *buf, uint8_t len);


private:
	/**
	* @brief 	get a PDU's type
	* @param	buf 	the buffer which contain a PDU
	* @return   PDU's type
	*/
	uint8_t getPDUType(const uint8_t *buf);

	MACLink link;
	int8_t  state;
	uint8_t ssap;
	uint8_t dsap;
	uint8_t SYMM_PDU[2];
};

#endif // __LLCP_H__
