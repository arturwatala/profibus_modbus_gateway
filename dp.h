#include "asic_io.h"

/* zmiejszany o 1 przy kazdym pakiecie DATA EXCH*/
#define DP_SET_USER_WDG(n)	    asic_byte_write(ASIC_UWRL, (unsigned char)n); \
	    asic_byte_write(ASIC_UWRH, (unsigned char)n >> 8);

#define DP_SET_SLAVE_ID(id)	    asic_byte_write(ASIC_IDRL, (unsigned char)id); \
	    asic_byte_write(ASIC_IDRH, (unsigned char)(id >> 8));

#define DP_ACK_USER_WDG()	    asic_byte_write(ASIC_MR1S, 0x20);
#define DP_SET_SLAVE_ADDRESS(addr)  asic_byte_write(ASIC_TSAR, ((unsigned char)addr) & 0x7f);

/****************************************************************************************** 
 * Funkcja nale¿y wywo³aæ, gdy odebrane parametry s¹ poprawne, natêpuje wtedy przejœcie
 * do stanu oczekiwania na konfiguracjê.
 * parametry: brak 
 * zwraca: 0 -nie potrzebne dodatkowe dzia³ania, 1 -w miêdzyczasie zosta³a odebrana 
 *         nowa ramka parametrow nale¿y wykonaæ sprawdzenie parametrów ponownie, 3 - b³¹d
 ******************************************************************************************/
#define DP_USR_PRM_OK()     asic_byte_read(0x0E);
#define DP_USR_PRM_NOT_OK() asic_byte_read(0x0F);
#define DP_CNFG_OK()        asic_byte_read(0x10);
#define DP_CNFG_NOT_OK()    asic_byte_read(0x11);




int dp_io_buff_alloc(unsigned char master_in_buff_len, unsigned char master_out_buff_len);
void dp_set_io_len(unsigned char master_in_len, unsigned char master_out_len);

int dp_diag_buff_alloc(unsigned char buff_len);

int dp_cnfg_param_buff_alloc(unsigned char cnfg_buff_len, unsigned char param_buff_len);
void dp_set_config_len(unsigned char out_cnfg_len);
void dp_read_from_master(char *out_buff, unsigned char offset, unsigned char len);
void dp_write_to_master(char *in_buff, unsigned char offset, unsigned char len);
unsigned char dp_read_usr_params(char *buff);
unsigned char dp_read_config(char *buff);

