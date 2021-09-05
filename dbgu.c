#include "dbgu.h"



char* dbgu_init(unsigned int baud_rate, unsigned short buff_size)
{
  dbgu_tx_buff = malloc(buff_size);
  
  DBGU_BRGR = 48000000/(baud_rate*16);
  DBGU_MR = 1<<11;
  DBGU_CR = 1<<4 | 1<<6;
  DBGU_PTCR = 1<<8;
  return dbgu_tx_buff;
}

void dbgu_print(char* dane)
{
  if(DBGU_TCR==0)
  {
    DBGU_TPR = (unsigned int)dane;
    DBGU_TCR = strlen(dane);
    return;
  }
  else 
    if(DBGU_TNCR==0)
    {
      DBGU_TNPR = (unsigned int)dane;
      DBGU_TNCR = strlen(dane);
      return;
    }
    
    
    while(DBGU_TCR!=0);
    DBGU_TPR = (unsigned int)dane;
    DBGU_TCR = strlen(dane);
    
}

void dbgu_printl(int dane)
{
}