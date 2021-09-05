#include "AT91SAM7X256.h"

#define SWC1   1<<14;     // PIOA
#define SWC10  1<<12;     // PIOA
#define SW1    1<<8;      // PIOA
#define SW2    1<<13;     // PIOA
#define SW4    1<<14;     // PIOB
#define SW8    1<<9;      // PIOA

#define profibus_drv_enable() PIOA_SODR = 1<<25;
#define profibus_drv_disable() PIOA_CODR = 1<<25;

#define modbus_drv_enable() PIOA_SODR = 1<<2;
#define modbus_drv_disable() PIOA_CODR = 1<<2;

// diody LED
// LED1A  PB11  (czerwona)  profibus stat 1
// LED2A  PB12  (czerwona)  modbus stat 1
// LED2B  PB13  (zielona)   modbus stat 2
// LED3A  PB10  (czerwona)  system stat 1
// LED3B  PB15  (zielona)   system stat 2


typedef enum {LED1A = 1<<11, LED2A = 1<<12, LED2B = 1<<13, LED3A = 1<<10, LED3B = 1<<15} LED_;




