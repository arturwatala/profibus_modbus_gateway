#include "AT91SAM7X256.h"
#include <targets/libat91sam7.h>
#include <ctl_api.h>
#include "irq.h"
//#include "usart.h"
#include "dbgu.h"

#define __DIS_FIQ() __asm("mrs r0, cpsr\n orr r0,r0,#0x40\n msr cpsr_c,r0\n");
#define __ENA_FIQ() __asm("mrs r0, cpsr\n bic r0,r0,#0x40\n msr cpsr_c,r0\n");

#define __DIS_IRQ() __asm("mrs r0, cpsr\n orr r0,r0,#0x80\n msr cpsr_c,r0\n");
#define __ENA_IRQ() __asm("mrs r0, cpsr\n bic r0,r0,#0x80\n msr cpsr_c,r0\n");





extern unsigned int blink_mask;
extern void sys_irq(void);

// PROFIBUS stany slave
typedef enum {BAUD_SEARCH, WAIT_PARAM, WAIT_CONFIG, DATA_EXCH} profi_state;



