#include "usart.h"
#define MASTERCLK 48000000
#include "AT91SAM7X256.h"

const unsigned int baud_rate[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

extern void(*end_rx_callback_p)(char*, unsigned int len);
extern void(*end_tx_callback_p)(unsigned int len);

/*
 *  Funkcja ustawia parametry portów szeregowych
 *
 */
void usart_config(unsigned int baud_rate, unsigned char par, unsigned char stopb)
{
    unsigned int tmp;

    PMC_PCER = 1<<7 | 1<<6;

    tmp = 3<<6 | par<<9 | stopb<<12;

    US0_MR = tmp | 1;   /* RS-485*/
    US1_MR = tmp;       /* RS-232*/

    tmp = MASTERCLK/(baud_rate * 16);

    US0_BRGR = tmp;
    US1_BRGR = tmp;

    US1_CR = 1<<4 | 1<<6;
    US1_TCR = 0;
    US1_TNCR = 0;
    US1_PTCR =  1<<8 | 1<<1;
}

void usart_conf(_usart_config *config)
{
    unsigned int USART_BASE, tmp = 0;

    if(config->port)
    {
        USART_BASE = USART0_BASE;                           /* rs-485 */
        _N(USART1_BASE + US0_CR_OFFSET) = 1<<5 | 1<<7;      /* disable US1 */
        AIC_IECR = 1<<6;
        AIC_IDCR = 1<<7;
        tmp = 1;
    }
    else
    {
        USART_BASE = USART1_BASE;         /* rs-232 */
        _N(USART0_BASE + US0_CR_OFFSET) = 1<<5 | 1<<7;      /* disable US0 */
        AIC_IECR = 1<<7;
        AIC_IDCR = 1<<6;
    }

    _N(USART_BASE + US0_MR_OFFSET) = tmp | 3<<6 | config->parity<<9 | config->stop_bits<<12;
    _N(USART_BASE + US0_BRGR_OFFSET) = MASTERCLK/(baud_rate[config->baud_rate] * 16);
    _N(USART_BASE + US0_CR_OFFSET) = 1<<4 | 1<<6;		/* enable rx tx*/
    _N(USART_BASE + US0_TCR_OFFSET) = 0;	
    _N(USART_BASE + US0_TNCR_OFFSET) = 0;
    _N(USART_BASE + US0_PTCR_OFFSET) =  1<<8 | 1<<1;;
}