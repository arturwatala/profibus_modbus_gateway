#include "AT91SAM7X256.h"
#include "status_address.h"
#include "dbgu.h"
#include "asic_io.h"

#ifndef ATTR_RUN_FROM_RAM 
#define ATTR_RUN_FROM_RAM __attribute__ ((section (".fast")))
#endif

void asic_irq() __attribute__ ((interrupt ("FIQ")));

void sys_irq() __attribute__ ((interrupt ("IRQ")));
void US0_IRQ() __attribute__ ((interrupt ("IRQ"))); //ATTR_RUN_FROM_RAM;
void US1_IRQ() __attribute__ ((interrupt ("IRQ"))); //ATTR_RUN_FROM_RAM;

extern unsigned short dbgu_curr;
extern unsigned int licznikb;
//extern char dbgu_tx_buff[128];
