#include "AT91SAM7X256.h"

typedef enum {BAUD_4800=4800, BAUD_9600=9600, BAUD_19200=19200, 
              BAUD_38400=38400, BAUD_57600=57600, BAUD_115200=115200} USART_BAUD_RATE;

typedef enum {PARITY_NONE=4, PARITY_ODD=1, PARITY_EVEN=0} USART_PARITY;
typedef enum {STOPBITS_1=0, STOPBITS_15=1, STOPBITS_2=2} USART_STOPBITS;
typedef enum {US0=USART0_BASE, US1=USART1_BASE} USART_NO;

#define _N(x) (*(volatile unsigned long *)(x))

typedef struct {
  unsigned char port;
  unsigned char baud_rate;
  unsigned char stop_bits;
  unsigned char parity;
  unsigned int timeout;
} _usart_config;


void usart_conf(_usart_config *config);
void usart_config(unsigned int baud_rate, unsigned char par, unsigned char stopb);



