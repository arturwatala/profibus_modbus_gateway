#include "usart.h"
#include "parser.h"
#include "modbus.h"

#define HEADER_MODULE_PARAM_LEN      5
#define MODULE_PARAM_LEN	     8

static const unsigned char mb_fc_tab[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x07, 0x00, 0x05, 0x0F, 0x10, 0x00, 0x17, 0x00};
static const unsigned int baud_rate[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

/*
  Ramka parametrow zawiera co najmniej 7 bajtow [0..6] zdefiniowanych przez standard. Bajt 7 zwykle
  zawiera konfiguracjê dla SPC3. Parametry uzytkownika powinny zaczynac sie od bajtu 8.
  8    9   10	13            18
  |0x00|0x00|0xA1|...parametry...|...modul_1...|...modul_2..........
*/

/*********************************************************************************************
 * Funkcja parsuje parametry u¿ytkownika i umieszcza je w odpowiednich strukturach.
 * parametry: struktura usart_config [out] bêdzie zawiera³a konfiguracjê portu szeregowego
 *            tablica job_config [out] bêdzie zawiera³a aktualne zadania dla modbus master
 *            wskaznika na bufor parametrów [in] zawiera parametry u¿ytkownika 
 *            len - iloœæ parametrów u¿ytkownika
 * zwraca:    0 gdy blad, iloœæ bajtów i/o wymienianych
 ********************************************************************************************/           

unsigned short parse_params(_usart_config* usart_config, _mb_job* job_config, char* param_buff, unsigned char len)
{
    unsigned short in_len=0, out_len=0;

    in_len = 0;
    out_len = 0;

    if(len < HEADER_MODULE_PARAM_LEN + MODULE_PARAM_LEN)	              /* co najmniej 1 modul */
        return 0;
  
    if((param_buff[2] != 0xA1) || (param_buff[0] !=0) || (param_buff[1] != 0) || (param_buff[4] != 0x35))     /* kontrola sta³ych pól*/
        return 0; 


    usart_config->baud_rate = param_buff[3]&0x07;	/* baud rate  */

    usart_config->stop_bits = (param_buff[3]>>6)&0x03;	/* stop bits  */

    usart_config->parity = (param_buff[3]>>4 & 0x03);	/* parity     */
    if(usart_config->parity == 2)
        usart_config->parity <<= 1;


    if(param_buff[3]&0x08)		/* rs-232 / rs-485 */
        usart_config->port = 1;
    else
        usart_config->port = 0;
    
    len -= HEADER_MODULE_PARAM_LEN;

    if(len % MODULE_PARAM_LEN)         /* musi byæ wielokrotonoœæ d³ugoœci parametrów modu³u*/
        return 0;

    param_buff += HEADER_MODULE_PARAM_LEN;

    while(len > 0)
    {
        job_config->mb_data_len = 1 << (param_buff[0] & 0x0F);    /* iloœæ wymienianych s³ów */
        job_config->in_buff_offset = in_len<<1;
        job_config->out_buff_offset = out_len<<1;

        switch(param_buff[0] & 0xF0)
        {
            case 0xC0: in_len+=job_config->mb_data_len;	      /* odczyt wejœæ */
            break;
            case 0xA0: out_len+=job_config->mb_data_len;	      /* zapis wyjœæ */
            break;
            case 0x70:
                      in_len+=job_config->mb_data_len;
                      out_len+=job_config->mb_data_len;	
            break;
            default: return 0;
        }
        
        job_config->mb_slave_addr = param_buff[1];
        job_config->mb_offset_msb = param_buff[3];
        job_config->mb_offset_lsb = param_buff[4];
        job_config->mb_xoffset_msb = param_buff[5];
        job_config->mb_xoffset_lsb = param_buff[6];

        job_config->mb_function = mb_fc_tab[param_buff[2] & 0x1F]; 
        if(job_config->mb_function == 0) 
            return 0; 

        job_config->retries_init = ((param_buff[2] & 0xC0)>>6) + 1;
        job_config->retries_left =  job_config->retries_init ;

        job_config->mb_reply_timeout = (baud_rate[usart_config->baud_rate]/100) * param_buff[7];

        param_buff += MODULE_PARAM_LEN;
        job_config += 1;
        job_config->mb_function = 0;
        len-=MODULE_PARAM_LEN;
    }

    return in_len + (out_len<<8);
}

