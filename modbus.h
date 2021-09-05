#ifndef ATTR_RUN_FROM_RAM 
#define ATTR_RUN_FROM_RAM __attribute__ ((section (".fast")))
#endif

#define _N(x) (*(volatile unsigned long *)(x))

typedef enum {MB_STATE_RESPONSE,  MB_STATE_ERROR,MB_STATE_IDLE,MB_STATE_BUSY} MB_FSM;
#define MB_ERROR_BAD_RESPONSE 0x10
#define MB_ERROR_TIMEOUT 0x20
#define MB_ERROR_CRC  0x30
#define MB_ERROR_EXCEPTION  0x40
#define MB_ERROR_UNKNOWN_CMD 0x50
#define MB_STATUS_READY 0x00000000
#define MB_STATUS_BUSY  0x00000001

typedef struct{
  unsigned char address;
  unsigned char function;
  unsigned char offset_hi;
  unsigned char offset_lo;
  unsigned char length_hi;
  unsigned char lentth_lo;
  char* out_buff;
  char* in_buff;
}_mb_query_struct;

void usart_end_tx(unsigned int USART_BASE);
void usart_rx_char(unsigned int USART_BASE);
void usart_rx_timeout(unsigned int USART_BASE);
void usart_error(unsigned int USART_BASE, unsigned int csr);

unsigned short crc16(char *data, unsigned int len);


typedef struct {
  unsigned char	  mb_slave_addr;	    // adres slave modbus
  unsigned char	  mb_function;	    // funkcja modbus
  unsigned char	  mb_offset_msb;	    // adres startowy przy odczycie
  unsigned char	  mb_offset_lsb;     
  unsigned char	  mb_xoffset_msb;	    // dla ramki  0x17 
  unsigned char	  mb_xoffset_lsb;
  unsigned char	  mb_data_len;	    // ilosc danych slowa
  unsigned char	  in_buff_offset;
  unsigned char	  out_buff_offset;	    // wskaznik na dane do zapisu (ramka 0x17)
  unsigned char	  retries_init;
  unsigned char	  retries_left;
  unsigned short	  mb_reply_timeout;
} _mb_job;

//unsigned char modbus_query(_mb_query_struct mb_query_struct);
