#ifndef __DEBUG_H__
#define __DEBUG_H__

//#define DEBUG

#ifdef DEBUG
#define DMSG(str)       Serial.print(str)
#define DMSG_HEX(n)     Serial.print(' '); Serial.print(n, HEX)
#else
#define DMSG(str)
#define DMSG_HEX(n)
#endif

#endif
