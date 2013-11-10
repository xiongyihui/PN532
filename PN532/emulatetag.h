/**************************************************************************/
/*!
    @file     emulatetag.h
    @author   Armin Wieser
    @license  BSD

    Implemented using NFC forum documents & library of libnfc
*/
/**************************************************************************/


#ifndef __EMULATETAG_H__
#define __EMULATETAG_H__

#include "PN532.h"

class EmulateTag{

public:
  EmulateTag(PN532Interface &interface) : pn532(interface) { }
  
  bool init();

  /*
   * @param uid pointer to byte array of length 3 (uid is 4 bytes - first byte is fixed) or zero for uid 
   * @param ndef encoded Ndefmessage
   * @param ndef_length 
   */
  void emulate(const uint8_t* ndef, const int16_t ndefLength, const uint8_t* uid = 0);

private:
  PN532 pn532;
};

#endif
