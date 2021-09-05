#include <stdio.h>
#include "asic_io.h"

/*
**
*/
static unsigned short __asic_buff__pt;	

unsigned char asic_byte_read(unsigned short addr)
{
  unsigned char data;
  __enter_critical();

  PIOB_SODR = WR|RD|ALE;
  PIOB_ODSR = addr<<19;                   // wystawienie pelnego adresu na szyne
  PIOB_OER = 0x7ff<<19;                    // szyna jako wyjscie
  PIOB_CODR = ALE;                        // ALE ->0 mlodsz cz. adresu zatrzasnieta
  PIOB_ODR = 0xff<<19;                    // mlodsza czesc szyny jako wejscie
  PIOB_CODR = RD;       
  PIOB_CODR = RD;       
  PIOB_CODR = RD;                         // musi byc powt.
  data = (PIOB_PDSR >> 19) & 0xFF;
  PIOB_SODR = RD;                         // RD->1
  PIOB_SODR = ALE;                        // ALE->1

  __leave_critical();
  return data; 
}

void asic_byte_write(unsigned short addr, unsigned char data) 
{
  __enter_critical();
  PIOB_SODR = WR|RD|ALE;
  PIOB_ODSR = addr<<19;                   // wystawienie pelnego adresu na szyne
  PIOB_OER = 0x7ff<<19;                   // szyna jako wyjscie
  PIOB_CODR = ALE;                        // ALE ->0 mlodsz cz. adresu zatrzasnieta
  PIOB_CODR = WR;                         // WR->0
  PIOB_ODSR = ((addr&0xf00)|data)<<19;    // starsze bity adresu musza byc ciagle obecne na szynie
  PIOB_SODR = WR;                         // WR->1
  PIOB_ODR = 0xff<<19;                    // mlodsza czesc szyny jako wejscie
  PIOB_SODR = ALE;
  __leave_critical();
}

void asic_read(char* buff, unsigned short addr, unsigned short len)
{
  PIOB_SODR = WR|RD|ALE;
  __enter_critical();
  while(len > 0)
  {
    PIOB_OER = 0x7ff<<19;                 // szyna jako wyjscie
    PIOB_ODSR = addr<<19;                 // wystawienie pelnego adresu na szyne
    PIOB_CODR = ALE;                      // ALE ->0 mlodsz cz. adresu zatrzasnieta
    PIOB_ODR = 0xff<<19;                  // mlodsza czesc szyny jako wejscie
    PIOB_CODR = RD; 
    PIOB_CODR = RD; 
    PIOB_CODR = RD;                       // spore opoznienie danych po RD (?)
    *buff = (PIOB_PDSR >> 19) & 0xFF;
    PIOB_SODR = RD;                       // RD->1
    PIOB_SODR = ALE;                      // ALE->1
    addr++;
    buff++;
    len--;
  };
  __leave_critical();
  
}

void asic_write(char* buff, unsigned short addr, unsigned short len)
{
 
   PIOB_SODR = WR|RD|ALE;
   PIOB_OER = 0x7ff<<19;                   // szyna jako wyjscie
   __enter_critical();
   while(len >0)
   {
    PIOB_ODSR = addr<<19;                   // wystawienie pelnego adresu na szyne
    PIOB_CODR = ALE;                        // ALE ->0 mlodsz cz. adresu zatrzasnieta
    PIOB_CODR = WR;                         // WR->0
    PIOB_ODSR = ((addr&0xf00)|*buff)<<19;
    PIOB_SODR = WR;                         // WR->1
    PIOB_SODR = ALE;
    addr ++;
    buff ++;
    len--;
  }
  PIOB_ODR = 0xff<<19;                    // mlodsza czesc szyny jako wejscie
  __leave_critical();
}

/*
 *   Funkcja wystawia reset uk³adu ASIC, ustawia tryb pracy zgodny PROFIBUS SPC3
 *   oraz zeruje wewnêtrzn¹ pamiêæ RAM uk³adu ASIC.
 *   Parametry: d³ugoœæ sygna³u resetu
 *   Zwraca: brak
 */
void asic_reset(unsigned int delay)
{
  PIOA_SODR = ASIC_RESET;
  volatile int a = delay;
  while(a) a--;
  PIOA_CODR = ASIC_RESET;
  __asic_buff__pt = 0x40;
  asic_byte_write(0x0, 0x03); // VPC3 mode select

  for( a=0x16; a<=0x7ff;a++)   // zerowanie pamieci
      asic_byte_write(a, 0);
}

/*
 *   Funkcja zraca wartoœæ rejestru statusu uk³adu ASIC.
 *   Parametry: brak
 *   Zwraca: s³owo statusu 
 */
unsigned short asic_read_status()
{
  unsigned short status;

  status = asic_byte_read(0x04);
  status |= asic_byte_read(0x05)<<8;

  return status;
}

/*
 *   Funkcja allokuje pamiêæ na bufory w ukladzie ASIC.
 *   Parametry: adres pod którym ma byæ zapisany wskaŸnik do segmentu
 *              zaallokowanej pamiêci, rozmiar allokowanego bufora w bajtach
 *              (bêdzie zaokr¹glony w górê do wielokrotnoœci 8)
 *   Zwraca: adres pierwszego elementu w buforze, lub zero gdy brak pamiêci.
 */
unsigned short asic_malloc(unsigned char asic_seg_pt, unsigned char len)
{
  unsigned short tmp = __asic_buff__pt;

  if((len % 8)!=0)
      __asic_buff__pt += ((len>>3) + 1)<<3;   
  else 
      __asic_buff__pt += len;

  if((__asic_buff__pt) <= 0x7FF)
    asic_byte_write(asic_seg_pt, tmp >> 3);
  else 
  {
    __asic_buff__pt = tmp;
    return 0;
  }
  return tmp;
}

void asic_zero_buff(unsigned short addr, unsigned short len)
{
    while(len)
    {
        asic_byte_write(addr, 0);
        addr+=1;
        len-=1;
    }
}
