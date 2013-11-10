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

#define NDEF_MAX_LENGTH 128  // altough ndef can handle up to 0xfffe in size, arduino cannot.
typedef enum {COMMAND_COMPLETE, TAG_NOT_FOUND, FUNCTION_NOT_SUPPORTED, MEMORY_FAILURE, END_OF_FILE_BEFORE_REACHED_LE_BYTES} responseCommand;

class EmulateTag{

public:
  EmulateTag(PN532Interface &interface) : pn532(interface) { }
  
  bool init();

  /*
   * @param uid pointer to byte array of length 3 (uid is 4 bytes - first byte is fixed) or zero for uid 
   */
  void emulate(const uint8_t* uid = 0);


  void setNdefFile(const uint8_t* ndef, const int16_t ndefLength);

private:
  PN532 pn532;
  uint8_t ndef_file[NDEF_MAX_LENGTH];
  void setResponse(responseCommand cmd, uint8_t* buf, uint8_t* sendlen, uint8_t sendlenOffset =0);
};

#endif
