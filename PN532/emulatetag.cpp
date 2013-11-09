/**************************************************************************/
/*!
    @file     emulatetag.cpp
    @author   Armin Wieser
    @license  BSD
*/
/**************************************************************************/

#include "emulatetag.h"
#include "PN532_debug.h"

#include <string.h>

#define NDEF_MAX_LENGTH 128  // altough ndef can handle up to 0xfffe in size, arduino cannot.
#define MAX_TGREAD


// Command APDU
#define C_APDU_CLA   0
#define C_APDU_INS   1 // instruction<
#define C_APDU_P1    2 // parameter 1
#define C_APDU_P2    3 // parameter 2
#define C_APDU_LC    4 // length command
#define C_APDU_DATA  5 // data

#define C_APDU_P1_SELECT_BY_ID   0x00
#define C_APDU_P1_SELECT_BY_NAME 0x04

// Response APDU
#define R_APDU_SW1_COMMAND_COMPLETE 0x90 
#define R_APDU_SW2_COMMAND_COMPLETE 0x00 

#define R_APDU_SW1_NDEF_TAG_NOT_FOUND 0x6a
#define R_APDU_SW2_NDEF_TAG_NOT_FOUND 0x82

// ISO7816-4 commands
#define ISO7816_SELECT_FILE 0xA4
#define ISO7816_READ_BINARY 0xB0

typedef enum { NONE, CC, NDEF } tag_file;   // CC ... Compatibility Container

const uint8_t compatibility_container[] = {
  0, 0x0F,
  0x20,
  0, 0x54,
  0, 0xFF,
  0x04,
  0x06,
  0xE1, 0x04,
  0xFF, 0xFE,
  0x00,
  0x00
};

bool EmulateTag::init(){
  pn532.begin();
  return pn532.SAMConfig();
}


void EmulateTag::emulate(const uint8_t* ndef, const int16_t ndefLength, const uint8_t* uid){
  if(ndefLength >  (NDEF_MAX_LENGTH -2)){
	DMSG("ndef file too large (> NDEF_MAX_LENGHT -2) - aborting");
	return;
  }

  uint8_t ndef_file[NDEF_MAX_LENGTH]; 
  ndef_file[0] = ndefLength >> 8;
  ndef_file[1] = ndefLength & 0xFF;
  memcpy(ndef_file+2, ndef, ndefLength);


  uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET,
        5,                  // MODE: PICC only, Passive only

        0x04, 0x00,         // SENS_RES
        0x00, 0x00, 0x00,   // NFCID1
        0x20,               // SEL_RES

        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,   // FeliCaParams
        0,0,

        0,0,0,0,0,0,0,0,0,0, // NFCID3t

        0, // length of general bytes
        0  // length of historical bytes
	  };

  if(uid != 0){  // if uid is set copy 3 bytes to nfcid1
    memcpy(command + 4, uid, 3);
  }

  if(1 != pn532.tgInitAsTarget(command,sizeof(command))){
    DMSG("tgInitAsTarget failed!");
    return;
  }

  uint8_t rwbuf[128];
  uint8_t sendlen;
  int16_t status;
  tag_file currentFile = NONE;
  uint16_t cc_size = sizeof(compatibility_container);

  while(1){

    status = pn532.tgGetData(rwbuf, sizeof(rwbuf));
    if(status < 0){
      DMSG("tgGetData failed!\n");
      return;
    }


    uint8_t p1 = rwbuf[C_APDU_P1];
    uint8_t p2 = rwbuf[C_APDU_P2];
    uint8_t lc = rwbuf[C_APDU_LC];

    switch(rwbuf[C_APDU_INS]){
    case ISO7816_SELECT_FILE:
      switch(p1){
      case C_APDU_P1_SELECT_BY_ID:
	if(p2 != 0x0c){
	  DMSG("C_APDU_P2 != 0x0c\n");
	  rwbuf[0] = 0x6A;
	  rwbuf[1] = 0x81;
	  sendlen = 2;
	} else if(lc == 2 && rwbuf[C_APDU_DATA] == 0xE1 && (rwbuf[C_APDU_DATA+1] == 0x03 || rwbuf[C_APDU_DATA+1] == 0x04)){
	  rwbuf[0] = R_APDU_SW1_COMMAND_COMPLETE;
	  rwbuf[1] = R_APDU_SW2_COMMAND_COMPLETE;
	  sendlen = 2;
	  if(rwbuf[C_APDU_DATA+1] == 0x03){
	    currentFile = CC;
	  } else if(rwbuf[C_APDU_DATA+1] == 0x04){
	    currentFile = NDEF;
	  }
	} else {
	  rwbuf[0] = R_APDU_SW1_NDEF_TAG_NOT_FOUND;
	  rwbuf[1] = R_APDU_SW2_NDEF_TAG_NOT_FOUND;
	  sendlen = 2;
	}
	break;
      case C_APDU_P1_SELECT_BY_NAME:       
	if(p2 == 0 
	   && lc == 7 
	   && rwbuf[5]  == 0xd2 
	   && rwbuf[6]  == 0x76 
	   && rwbuf[7]  == 0x00 
	   && rwbuf[8]  == 0x00 
	   && rwbuf[9]  == 0x85 
	   && rwbuf[10] == 0x01 
	   && rwbuf[11] == 0x01 
	   && rwbuf[12] == 0x00 
	  ){
	  rwbuf[0] = R_APDU_SW1_COMMAND_COMPLETE;
	  rwbuf[1] = R_APDU_SW2_COMMAND_COMPLETE;
	  sendlen = 2;
	} else{
	  DMSG("function not supported\n");
	  rwbuf[0] = 0x6A;
	  rwbuf[1] = 0x81;
	  sendlen = 2;
	} 
	break;
      }
      break;
    case ISO7816_READ_BINARY:
      switch(currentFile){
      case NONE:
	rwbuf[0] = R_APDU_SW1_NDEF_TAG_NOT_FOUND;
	rwbuf[1] = R_APDU_SW2_NDEF_TAG_NOT_FOUND;
	sendlen = 2;
	break;
      case CC:
	memcpy(rwbuf,compatibility_container + (p1 << 8) + p2, lc);
	rwbuf[lc  ] = R_APDU_SW1_COMMAND_COMPLETE;
	rwbuf[lc+1] = R_APDU_SW2_COMMAND_COMPLETE;
        sendlen = lc+2;
	break;
      case NDEF:
	// TODO check length
	memcpy(rwbuf, ndef_file + (p1 << 8) + p2, lc);
	rwbuf[lc  ] = R_APDU_SW1_COMMAND_COMPLETE;
        rwbuf[lc+1] = R_APDU_SW2_COMMAND_COMPLETE;
	sendlen = lc+2;
	break;
      }
      break;
    default:
      DMSG("Command not supported!");
      DMSG_HEX(rwbuf[C_APDU_INS]);
      DMSG("\n");
      rwbuf[0] = R_APDU_SW1_COMMAND_COMPLETE;
      rwbuf[1] = R_APDU_SW2_COMMAND_COMPLETE;
      sendlen = 2;
    }

    status = pn532.tgSetData(rwbuf, sendlen);
    if(status < 0){
      DMSG("tgSetData failed\n!");
      return;
    }
  }

}
