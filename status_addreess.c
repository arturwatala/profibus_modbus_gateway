#include "status_address.h"

void led_on(LED_ dioda)
{
  PIOB_SODR = dioda;
}

void led_off(LED_ dioda)
{
  PIOB_CODR = dioda;
}

void led_toggle(LED_ dioda)
{
  if(dioda & PIOB_ODSR)
    PIOB_CODR = dioda;
  else
    PIOB_SODR = dioda;
}


/******************************************************
*    ODCZYT ADRESU Z PRZELACZNIKOW NA PAELU 2011.02.04
*******************************************************/
char read_addr_switch()
{
  unsigned char ret;

  // odczyt dziesiatek
  PIOA_ODR = SWC1;    // pin SWC1 weak pull-up
  
  PIOA_OER = SWC10;   // pin SWC10 output
  PIOA_SODR = SWC1;   // pin SWC1 pull-up
  PIOA_CODR = SWC10;  // pin SWC10 -> 0

  ret = ~(((PIOA_PDSR & 1<<8)>>8) | ((PIOA_PDSR & 1<<13)>>12) |
         ((PIOB_PDSR & 1<<14)>>12) | ((PIOA_PDSR & 1<<9)>>6));

  ret = (ret & 0xF) * 10;

  // odczyt jednosci
  PIOA_ODR = SWC10;    // pin SWC10 weak pull-up
  PIOA_OER = SWC1;     // pin SWC1 out
  PIOA_SODR = SWC10;   // pin SWC10 pull-up
  PIOA_CODR = SWC1;    // pin SWC1 -> 0

  ret += (~(((PIOA_PDSR & 1<<8)>>8) | ((PIOA_PDSR & 1<<13)>>12) |
        ((PIOB_PDSR & 1<<14)>>12) | ((PIOA_PDSR & 1<<9)>>6)))&0xF;

  return ret;
}

char modbus_voltage_chck()
{
  if(PIOA_PDSR & 1<<4)
    return 1;
  else
    return 0;
}

char profibus_voltage_chck()
{
  if(PIOA_PDSR & 1<<26)
    return 1;
  else
    return 0;
}


