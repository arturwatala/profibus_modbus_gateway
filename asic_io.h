#include "AT91SAM7X256.h"
/* Interrupts registers */
#define ASIC_IRRL     0x00    // int. reqest reg low byte         R/W
#define ASIC_IRRH     0x01    // int. request reg high byte       R/W
#define ASIC_IARL     0x02    // int. ack. reg. low byte	  W
#define ASIC_IARH     0x03    // int. ack. req. high byte	  W
#define ASIC_IMRL     0x04    // int. mask reg low byte	  W
#define ASIC_IMRH     0x05    // int. mask reg high byte	  W
#define ASIC_IPRL     0x02    // int. pending reg low byte	  R
#define ASIC_IPRH     0x03    // int. pending reg high byte	  R
/* MODE REGISTER 0 */
#define ASIC_MR0L     0x06    // mode register 0 low byte	  R/W
#define ASIC_MR0H     0x07    // mode register 0 high byte	  R/W
/* MODE REGISTER 1 */
#define ASIC_MR1S     0x08    // mode register 1 set bit	  W
#define ASIC_MR1R     0x09    // mode register 1 reset bit	  W
#define ASIC_MR1      0x15    // mode register 1 status	  R
/* MODE REGISTER 2 */
#define ASIC_MR2      0x0c    // mode register 2	  W
/* STATUS REGS */
#define ASIC_SRL      0x04    // status register low byte	  R
#define ASIC_SRH      0x05    // status register high byte	  R
/* USER WATCHDOG */
#define ASIC_UWRL     0x18    // DP user watchdog reg l. byte	  R/W
#define ASIC_UWRH     0x19    // DP user watchdog reg h. byte	  R/W
/* BAUD RATE WD */
#define ASIC_BRWR     0x0A    // watchdog monitoring square	  W
/* DP SLAVE ADDRESS */
#define ASIC_TSAR     0x16    // DP this station addr. reg.	  R/W
/* DP SLAVE IDENT */  
#define ASIC_IDRL     0x3A    // DP slave ident reg. low byte	  R/W
#define ASIC_IDRH     0x3B    // DP slave ident reg. high byte	  R/W
/* Set Slave addr pointer reg */
#define ASIC_SSAPT    0x2E    // DP pointner to ssa buffer	  R/W
#define ASIC_SSALEN   0x2F    // recived data len in buffer	  R/W
/* DIN buffers Slave->Master */
#define ASIC_DINCUR   0x08
#define ASIC_DINLEN   0x1E    // data len to send
#define ASIC_DIN1PT   0x1F
#define ASIC_DIN2PT   0x20
#define ASIC_DIN3PT   0x21
/* DOUT buffers Master->Slave */
#define ASIC_DOCUR    0x0A
#define ASIC_DOLEN    0x1A    // received data len
#define ASIC_DO1PT    0x1B
#define ASIC_DO2PT    0x1C
#define ASIC_DO3PT    0x1D
/* DIAG buffers */
#define ASIC_DIAG1LN    0x24   
#define ASIC_DIAG2LN    0x25
#define ASIC_DIAG1PT    0x26
#define ASIC_DIAG2PT    0x27
/* AUX buffers */
#define ASIC_AUX1LN    0x28   
#define ASIC_AUX2LN    0x29
#define ASIC_AUX1PT    0x2B
#define ASIC_AUX2PT    0x2C
#define ASIC_AUXSEL    0x2A
/* PARAM buffer*/
#define ASIC_PRMPT     0x30
/* CONFG buffer*/
#define ASIC_CNFPT     0x32
/* OUT config buffer*/
#define ASIC_OUTCNFLN  0x33
#define ASIC_OUTCNFPT  0x34



#define ASIC_VERSION  (asic_byte_read(0x05) >> 4)

#ifndef ATTR_RUN_FROM_RAM 
#define ATTR_RUN_FROM_RAM __attribute__ ((section (".fast")))
#endif

#define ALE    1<<16
#define RD     1<<17
#define WR     1<<18
#define ASIC_RESET  1<<24  // pioa

#define __enter_critical()  __asm("mrs r5, cpsr\n orr r5,r5,#0xC0\n msr cpsr_c,r5\n");
#define __leave_critical()  __asm("mrs r5, cpsr\n bic r5,r5,#0xC0\n msr cpsr_c,r5\n");
#define ASIC_END_IRQ() asic_byte_write(ASIC_MR1S, 1<<1);
#define ASIC_INT_ACK(mask)  asic_byte_write(ASIC_IARL, (unsigned char)(mask)); asic_byte_write(ASIC_IARH, (unsigned char)(mask>>8));
#define ASIC_INT_MASK(mask) asic_byte_write(ASIC_IMRL, (unsigned char)(mask));  asic_byte_write(ASIC_IMRH, (unsigned char)(mask>>8));
#define ASIC_SET_BAUD_WDG(t) asic_byte_write(ASIC_BRWR, t); 


// odczyt/zapis pojedynczego bajtu z ASIC, kod umieszczony w SRAM
unsigned char asic_byte_read(unsigned short addr) ATTR_RUN_FROM_RAM;
void asic_byte_write(unsigned short addr, unsigned char data) ATTR_RUN_FROM_RAM;
// odczyt/zapis n bajtow z asic
void asic_read(char* buff, unsigned short addr, unsigned short len) ATTR_RUN_FROM_RAM;
void asic_write(char* buff, unsigned short addr, unsigned short len) ATTR_RUN_FROM_RAM;

unsigned short asic_malloc(unsigned char asic_seg_pt, unsigned char len);

void asic_reset(volatile unsigned int);
unsigned short asic_read_status();

