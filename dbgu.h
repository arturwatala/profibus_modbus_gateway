#include "AT91SAM7X256.h"

char* dbgu_tx_buff;

extern int dbgu_printf(char *, const char *, ...);
char* dbgu_init(unsigned int baud_rate, unsigned short buff_size);
void dbgu_print(char* dane);
void dbgu_printl(int dane);


