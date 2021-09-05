#include "modbus.h"
#include "AT91SAM7X256.h"

#define MODBUS_CHAR_DELAY_MAX 35

static _mb_job *mb_jobs[30];
static unsigned char curr_job;
static char frame_s, frame_r;

static char mb_out_buff[256];
static char mb_in_buff[256];



static char *mb_diag_buff;
static char *in_buff;
static char *out_buff;

static unsigned char rx_len;
static MB_FSM state;
static unsigned long USART_BASE = USART1_BASE;


/* odebrany pierwszy znak */
void usart_rx_char(unsigned int USART_BASE)
{
    mb_in_buff[0] = _N(USART_BASE + US0_RHR_OFFSET);	
    rx_len++;
    _N(USART_BASE + US0_IDR_OFFSET) = 1;	      /* disable rx ready interrupt*/
    _N(USART_BASE + US0_RTOR_OFFSET) = MODBUS_CHAR_DELAY_MAX; 
    _N(USART_BASE + US0_CR_OFFSET) = 1<<11;	      /* rearm timeout */
    _N(USART_BASE + US0_CR_OFFSET) = 1<<15;	      /* rearm and start timeout */
    _N(USART_BASE + US0_RCR_OFFSET) = 254;
    _N(USART_BASE + US0_RPR_OFFSET) = mb_in_buff + 1;
    _N(USART_BASE + US0_PTCR_OFFSET) = 1;
    
}

/* end of frame, or reply timeout*/
void usart_rx_timeout(unsigned int USART_BASE)
{
    _N(USART_BASE + US0_IDR_OFFSET) = US1_CSR_TIMEOUT_MASK;
    _N(USART_BASE + US0_PTCR_OFFSET) = 1<<1;	
    frame_r = 1;
}    

/* bledy typu par, overrun, framming*/
void usart_error(unsigned int USART_BASE, unsigned int csr)
{
    _N(USART_BASE + US1_CR_OFFSET) = 1<<8;	  /* reset flags */
}

/* trzeba wyzerowaæ dane wejœciowe, gdy b³edna odpowiedŸ, lub brak odpowiedzi */
int mb_check_response()
{
    unsigned short crc;
    unsigned char doffset;

    if(rx_len > 0)
        rx_len += 254 - _N(USART_BASE + US0_RCR_OFFSET);
    else
    {   
        mb_diag_buff[curr_job] = MB_ERROR_TIMEOUT;
        return -1;
    }

    if(rx_len < 5)		      /* timeout, za krótka ramka*/
    {
        mb_diag_buff[curr_job] = MB_ERROR_TIMEOUT;
        return -1;
    }
    else
        if(mb_in_buff[0] != mb_job[curr_job].mb_slave_addr)
        {
            mb_diag_buff[curr_job] = MB_ERROR_BAD_RESPONSE;
            return -1;
        }

    crc = crc16(mb_in_buff, rx_len-2);	                  /* obliczenie crc odpowiedzi */

    if(mb_in_buff[rx_len-1] != (char)(crc>>8) || mb_in_buff[rx_len-2] != (char)(crc))
    {
        mb_diag_buff[curr_job] = MB_ERROR_CRC;
        return -1;
    }

    if(mb_in_buff[1] != mb_job[curr_job].mb_function)	                  /* sprawdzenie kodu funkcji odpowiedzi */
    {
        if((mb_in_buff[1]&0x7F) == mb_job[curr_job].mb_function)                  /* je¿eli najstarszy bit=1 to wyj¹tek */
        {
            mb_diag_buff[curr_job] = MB_ERROR_EXCEPTION | (mb_in_buff[2] &0x0F); 
            return -1;
        }
        else
            mb_diag_buff[curr_job] = MB_ERROR_BAD_RESPONSE;	                  /* blêdna odpowiedŸ*/
        return -1;
    }  

    
    switch(mb_job[curr_job].mb_function)	                  /* sprawdzenie poprawnosci odp. */
    {
        case 0x01 :		                  /* Read Coils Status */
        case 0x02 :		                  /* Read Inputs Status */
        case 0x03 :		                  /* Read Holding Registers */
        case 0x04 :		                  /* Read Input Registers */
        case 0x17 :		                  /* Read/Write registers */
                    if((mb_in_buff[2]>>1)!=mb_job[curr_job].mb_data_len)
                    {
                        mb_diag_buff[curr_job] = MB_ERROR_BAD_RESPONSE;
                        break;
                    }
                    memcpy((in_buff + mb_job[curr_job].in_buff_offset), &mb_in_buff[3], mb_in_buff[2]);

                    break;
        case 0x05 :		  /* force single coil */
        case 0x06 :		  /* preset single register */
        case 0x0F :		  /* force multiple coils (bitow) */
        case 0x10 :		  /* preset multiple registers */
                    if(memcmp(mb_in_buff[3], mb_out_buff[3], 4))
                        mb_diag_buff[curr_job] = MB_ERROR_BAD_RESPONSE;
                    
                    break;
        
        case 0x07 :		  /* read status coils*/
                    *(in_buff + mb_job[curr_job].in_buff_offset) = mb_in_buff[2];
                        break;
        default : return -1;  // no such command erro
    }    

    return 0;
}

static char test_buff[] = "\x02\x00\xCC\xFF\xEE\xFF\x11\x22\x33\x44\x55\x66x\77\x88";

static char diag_bf[255];


int mb_send_query()
{
    unsigned short  mb_points_no;
    unsigned short  data_offset, crc;
    unsigned char   dlen, doffset, len;
  
    mb_out_buff[0] = mb_job[curr_job].mb_slave_addr;
    mb_out_buff[1] = mb_job[curr_job].mb_function;
    mb_out_buff[2] = mb_job[curr_job].mb_offset_msb;	        /* adres pierwszej cewki(bitu) lub rejestru*/
    mb_out_buff[3] = mb_job[curr_job].mb_offset_lsb;

    mb_points_no = mb_job[curr_job].mb_data_len;	        /* iloœæ s³ów do zaczytu/odczytu */

    switch(mb_job[curr_job].mb_function)
    {
        case 0x01 :		        /* Read Coils Status */
        case 0x02 : mb_points_no <<= 4;	        /* Read Inputs Status */
        case 0x03 :		        /* Read Holding Registers */
        case 0x04 :		        /* Read Input Registers */
                    mb_out_buff[4] = (char)(mb_points_no >> 8);	        /* dla ramek 1..4 skladnia jest taka sama */
                    mb_out_buff[5] = (char)(mb_points_no);	  
                    doffset = 6;
                    dlen = 0;
                    break;
        case 0x05 :		                            /* force single coil */
                    mb_out_buff[4] = *(out_buff + mb_job[curr_job].out_buff_offset);        /* ustawienie cewki 0xFF, lub zerowanie 0x00*/
                    mb_out_buff[5] = 0x00;	                            /* sta³a 0x00*/
                    doffset = 6;
                    dlen = 0;
                    break;
        case 0x06 :		        /* preset single register */
                    mb_out_buff[4] = *(out_buff + mb_job[curr_job].out_buff_offset);        /* wartoœæ do presetu MSB */
                    mb_out_buff[5] = *(out_buff + mb_job[curr_job].out_buff_offset);      /* wartoœæ do presetu LSB */
                    doffset = 6;
                    dlen = 0;
                    break;
        case 0x07 :		        /* read status coils*/
                    /* tylko pole fc*/
                    doffset = 2;
                    dlen = 0;
                    break;
        case 0x0F :		        /* force multiple coils (bitow) */
                    mb_out_buff[4] = (char)(mb_points_no>>4);	        /* ilosc slow -> ilosc bitow */
                    mb_out_buff[5] = (char)(mb_points_no<<4);	

                    mb_out_buff[6] = mb_points_no << 1;	        /* liczba bajtow danych */
                    doffset = 7;
                    dlen = mb_out_buff[6];
                    break;
        case 0x10 :		        /* preset multiple registers */
                    mb_out_buff[4] = 0x00;	        /* ilosc zapisywanych rejestrow MSB 0x00  */
                    mb_out_buff[5] = (char)(mb_points_no);	        /* ilosc zapisywanych rejestrow LSB       */
                    mb_out_buff[6] = mb_points_no << 1;	        /* liczba przesylanych bajtow	            */
                    doffset = 7;
                    dlen =  mb_out_buff[6];
                    break;
        case 0x17 :
                    mb_out_buff[4] = 0x00;	        /* ilosc odczyt rejestrow MSB 0x00  */
                    mb_out_buff[5] = (char)(mb_points_no);	        /* ilosc odczyt rejestrow LSB       */
                    mb_out_buff[6] = mb_job[curr_job].mb_xoffset_msb;   /* offset do zapisu msb */
                    mb_out_buff[7] = mb_job[curr_job].mb_xoffset_lsb;   /* offset do zapisu lsb */
                    mb_out_buff[8] = 0x00;	        /* ilosc slow do zapisu msb 0x00*/
                    mb_out_buff[9] = (char)(mb_points_no);	        /* ilosc slow do zapisu lsb */
                    mb_out_buff[10] = mb_points_no << 1;	        /* ilosc przesylanych bajtow danych */
                    doffset = 11;
                    dlen = mb_out_buff[10];
                    break;
        default : 
                    mb_diag_buff[curr_job] = MB_ERROR_UNKNOWN_CMD;
                    return -1;	  // no such command erro
    }

    
    len = dlen + doffset;
    memcpy(&mb_out_buff[doffset], (out_buff + mb_job[curr_job].out_buff_offset), dlen);

    crc = crc16(mb_out_buff, len);

    mb_out_buff[len+1] = (unsigned char)(crc>>8);
    mb_out_buff[len] = (unsigned char)(crc);

    len+=2;

    _N(USART_BASE + US0_TPR_OFFSET) = mb_out_buff;
    _N(USART_BASE + US0_RTOR_OFFSET) = mb_job[curr_job].mb_reply_timeout;
    _N(USART_BASE + US0_TCR_OFFSET) = len;                       /* start transmisji */
    _N(USART_BASE + US0_IDR_OFFSET) = 0xFFFFFFFF;
    _N(USART_BASE + US0_IER_OFFSET) = US0_IER_ENDTX_MASK;        /* przerwanie koniec transmisi (gdy TCR = 0) */
    rx_len = 0;

    return 0;
}



static char i;

void modbus_init(char *din, char *dout, char *diag_buff)
{
      in_buff = din;
      out_buff = test_buff;//dout;
      mb_diag_buff = diag_bf;//diag_buff;


      curr_job = 0;
      state = MB_STATE_IDLE;

    

     mb_job[0].mb_slave_addr = 1;
     mb_job[0].mb_function = 0x3;

     mb_job[0].mb_data_len = 2;  // 1 s³owa

     mb_job[0].mb_offset_msb = 0;
     mb_job[0].mb_offset_lsb = 10;
     mb_job[0].in_buff_offset = 0;


     mb_job[0].out_buff_offset = 0;    // write out buf p  
     mb_job[0].mb_xoffset_lsb = 0;        // write offset lsb
     mb_job[0].mb_xoffset_msb = 0;        // write offset msb

         
     mb_job[0].mb_reply_timeout = 1000;
     mb_job[0].retries_left = 10;
}





int mb_query(unsigned long usart, _mb_job *job, char *out_buff, char *in_buff)
{
    if(frame_s == 0)		      /* czy bylo wyslane zapytanie*/
    {
        mb_send_query();
            frame_s = 1;
    }
    if(frame_s & frame_r)
    {
          mb_check_response();
          frame_r = 0;
          frame_s = 0;

    }
    return 0;
}

unsigned short crc16(char *data, unsigned int len)
{
    unsigned short crc = 0xFFFF;
    char i;

    while(len--)
    {
        i = 8;
        crc^=(0xff&(*data++));
        while(i--)
        {
            if(crc&1)
            {
                crc>>=1;
                crc^=0xA001;
            }
            else
                (crc>>=1);
        }
    }
    return crc;
}