#include "irq.h"
#include "modbus.h"
#include "AT91SAM7X256.h"
#include <stdio.h>
volatile unsigned long countner;         // ticks countner
volatile unsigned long uptime;           // (seconds)
unsigned int  blink_mask;        // maska/diody ktore maja migac


int gc;
void asic_irq()
{
gc++;
   asic_byte_write(ASIC_IARL, 0x10);	              // int. ack
   asic_byte_write(ASIC_MR1S, 1<<1);	              // end of interrupt (clear int. output)
/*
  zdarzenie z asic
  ustawienie odpowiedniej flagi, lub nastepnego stanu asic np. flaga sa nowe dane
*/



  AIC_ICCR = 1;
}

void sys_irq()
{
  if (PIT_SR & PIT_SR_PITS)
    {
      int ret = PIT_PIVR;

      countner ++;
      
      if(countner==5)
      {
        led_toggle(blink_mask);
      }
      if(countner >= 20)
      {
        uptime ++;
        //dbgu_printf(0,"%i\r",uptime);
        countner = 0;
      }
      

    }

  if(DBGU_SR & 1)
  {
    
  //  dbgu_rx_buff[dbgu_curr] = (char)DBGU_RHR;
  //  dbgu_curr++;
    if(DBGU_RHR == 13){
    
     
   
     //  dbgu_curr = 0;
    
    }

  }

}


void US0_IRQ()
{
    unsigned int isr = US0_CSR & US0_IMR;
    
    if(isr & US0_CSR_ENDTX_MASK)
    {    
        US0_IDR = US0_IDR_ENDTX_MASK; 
        US0_CR = 1<<11;//US1_CR_RETTO_MASK;
        US0_CR = 1<<15; //US1_CR_STTTO_MASK;       /* start reply timeout timer*/
        US0_IER = US0_IER_RXRDY_MASK | US0_IER_TIMEOUT_MASK |7<<5 ;        /*  rx interrupt enable, rx errors */
    }

    if(isr & US0_CSR_TIMEOUT_MASK)
        usart_rx_timeout(USART0_BASE);

    if(isr & (US0_CSR_PARE_MASK|US0_CSR_OVRE_MASK|US0_CSR_FRAME_MASK))
        usart_error(USART0_BASE, 0);
    
    if(isr & US0_CSR_RXRDY_MASK)
        usart_rx_char(USART0_BASE);
}


void US1_IRQ()
{
    unsigned int isr = US1_CSR & US1_IMR;
    
    if(isr & US1_CSR_ENDTX_MASK)
    {    
        US1_IDR = US1_IDR_ENDTX_MASK; 
        US1_CR = 1<<11;//US1_CR_RETTO_MASK;
        US1_CR = 1<<15; //US1_CR_STTTO_MASK;       /* start reply timeout timer*/
        US1_IER = US1_IER_RXRDY_MASK | US1_IER_TIMEOUT_MASK |7<<5 ;        /*  rx interrupt enable, rx errors */
    }

    if(isr & US1_CSR_TIMEOUT_MASK)
        usart_rx_timeout(USART1_BASE);

    if(isr & (US1_CSR_PARE_MASK|US1_CSR_OVRE_MASK|US1_CSR_FRAME_MASK))
        usart_error(USART1_BASE, 0);
    
    if(isr & US1_CSR_RXRDY_MASK)
        usart_rx_char(USART1_BASE);
}

