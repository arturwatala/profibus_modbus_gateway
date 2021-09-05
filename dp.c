#include "dp.h"
#include "asic_io.h"

static unsigned short p_buff_out[3];        /* trzy bufory danych wyjœciowych master in -> slave out*/
static unsigned short p_buff_in[3];         /* trzy bufory danych wejœciowych slave out -> master in*/
static unsigned short p_buff_config;        /* adres bufora konfiguracji master -> slave */
static unsigned short p_buff_param;         /* adres bufora parametrów   master -> slave */
static unsigned short p_buff_diag[2];       /* adresy buforów diagnostycznych */
static unsigned short p_buff_out_config;    /* adres bufora aktualnej konfiguracji dla mastera*/


/***************************************************************************************
 *  Funkcja allokuje po trzy bufory IO wewnarz ukladu ASIC, 
 *  i ustawia wskaŸniki na te bufory w odpowiednich rejestrach uk³adu asic.
 *  argumenty: rozmiar bufora I, oraz rozmiar bufora O
 *  zwraca: 0 - je¿eli allokacja OK lub -1 - je¿eli zabrak³o pamiêci.
 ***************************************************************************************/
int dp_io_buff_alloc(unsigned char master_in_buff_len, unsigned char master_out_buff_len)
{
  
    asic_byte_write(0x1A, 0);		
    asic_byte_write(0x1E, 0);	

    p_buff_out[0] = asic_malloc(ASIC_DO1PT, master_out_buff_len);  /* bufory O wyjsciowe master->slave */
    p_buff_out[1] = asic_malloc(ASIC_DO2PT, master_out_buff_len);
    p_buff_out[2] = asic_malloc(ASIC_DO3PT, master_out_buff_len);

    if(p_buff_out[0]==0 || p_buff_out[1]==0 || p_buff_out[2]== 0)
        return -1;
        	     
    p_buff_in[0] = asic_malloc(ASIC_DIN1PT, master_in_buff_len);   /* bufory I wejsciowe slave->master */
    p_buff_in[1] = asic_malloc(ASIC_DIN2PT, master_in_buff_len);
    p_buff_in[2] = asic_malloc(ASIC_DIN3PT, master_in_buff_len);

    if(p_buff_in[0]==0 || p_buff_in[1]==0 || p_buff_in[2]==0)
        return -1;
	         
    return 0;
}
/**************************************************************************************
 *  Funkcja ustawia iloœæ wymienianych danych I oraz O ze stacj¹ master.
 *  Iloœæ wymienianych danych musi byæ dok³adnie równa tej wynikaj¹cej z konfigu.
 *  argumenty: iloœæ wymienianych danych I, iloœæ wymienianych danych O
 *  zwraca: brak
 **************************************************************************************/
void dp_set_io_len(unsigned char master_in_len, unsigned char master_out_len)
{     
      asic_byte_write(0x1E, master_in_len);		     
      asic_byte_write(0x1A, master_out_len);		
}
/**************************************************************************************
 *  Funkcja allokuje dwa jednakowe bufory dla diagnostyki, zapis do jednego z buforów
 *  jest zawsze dostêpny dla u¿ytkownika.
 *  argumenty: d³ugoœæ bufora
 **************************************************************************************/
int dp_diag_buff_alloc(unsigned char usr_diag_len)
{
                            /* bufor diagnostyczny musi zawierac co najmniej 6 bajtow*/

    p_buff_diag[0] = asic_malloc(ASIC_DIAG1PT, usr_diag_len+6);
    p_buff_diag[1] = asic_malloc(ASIC_DIAG2PT, usr_diag_len+6); 

    if(p_buff_diag[0] == 0 || p_buff_diag[1] == 0)
      return -1;

    asic_byte_write(ASIC_DIAG1LN, usr_diag_len+6);    /* iloœæ danych w buforach diagn. */             
    asic_byte_write(ASIC_DIAG1LN, usr_diag_len+6);
    
    return 0;
}

/*****************************************************************************************
 *  Funkcja allokuje bufory dla przychodz¹cych parametrów, przychodz¹cego konfigu, oraz
 *  wychodz¹cego konfigu. Bufor wychodz¹cego konfigu jest domyœlnie równy d³ugoœci bufora
 *  przychodz¹cego konfigu.
 *  argumenty: rozmiar bufora przychodz¹cego konifgu, rozmiar bufora parametrów
 *****************************************************************************************/
int dp_cnfg_param_buff_alloc(unsigned char cnfg_buff_len, unsigned char param_buff_len)
{
    /*  bufory AUX s¹ przypisane do buforów parametrów i konfiguracji
     *  bufor AUX1 musi mieæ rozmiar równy buforowi konfiguracji
     *  bufor AUX2 musi mieæ rozmiar równy buforowi parametrów
     */
     
    if(asic_malloc(ASIC_AUX1PT, cnfg_buff_len)==0 || asic_malloc(ASIC_AUX2PT, param_buff_len)==0)
        return -1;
       
    asic_byte_write(ASIC_AUX1LN, cnfg_buff_len);	    /* d³ugoœci buforów cnfg i param */
    asic_byte_write(ASIC_AUX2LN, param_buff_len);	

    p_buff_param = asic_malloc(ASIC_PRMPT, param_buff_len);	    /* allokacja buforów cnfg i param */
    p_buff_config = asic_malloc(ASIC_CNFPT, cnfg_buff_len);
    p_buff_out_config = asic_malloc(ASIC_OUTCNFPT, cnfg_buff_len);  /* do tego bufora nalezy wpisac aktualna
		       konfiguracje i ustawic jego d³goœæ */
    if(p_buff_config==0 || p_buff_param==0 || p_buff_out_config==0)
        return -1;

    asic_byte_write(ASIC_AUXSEL, 2);	/* przyporz¹dkowanie buforów AUX1->cnfg
		 * AUX2->param */
    return 0;
}
/************************************************************************************
 *  Funkcja ustawia rzeczywist¹ d³ugoœæ konfiguracji w buforze wychodz¹cym konifgu
 *  d³ugoœæ powinna byæ dok³adnie równa d³ugoœci rzeczywistej konfiguracji.
 ************************************************************************************/
void dp_set_config_len(unsigned char out_cnfg_len)
{
    asic_byte_write(ASIC_OUTCNFLN , out_cnfg_len);  
}

/**************************************************************************************
 *  Dane io dla stacji slave od master, beda skopiowane z aktywnego bufora ASIC+offset
 *  do bufora out_buff w ilosci len
 **************************************************************************************/
void dp_read_from_master(char *out_buff, unsigned char offset, unsigned char len)
{
    unsigned char tmp = asic_byte_read(0x0B);
    if(tmp&4)
        asic_read(out_buff, p_buff_out[(tmp&3)-1]+offset, len);
    
    asic_byte_write(ASIC_IARH, 0x20); /* int ack.*/
}

/**************************************************************************************
 *  Dane dla stacji master, beda skopiowane z bufora in_buff do aktualnego 
 *  bufora ASIC+offset w ilosci len
 **************************************************************************************/
void dp_write_to_master(char *in_buff, unsigned char offset, unsigned char len)
{
    static unsigned char curr_buff_index;

   
    asic_write(in_buff, p_buff_in[curr_buff_index] + offset, len);
    curr_buff_index = (asic_byte_read(0x09)&3)-1; 
    
}

/**************************************************************************************
 *  Funkcja kopiuje parametry u¿ytkownika z bufora uk³adu asic do wskazanego bufora 
 *  i zwraca iloœæ skopiowanych bajtów
 **************************************************************************************/
unsigned char dp_read_usr_params(char *buff)
{
    unsigned char len;
    
    len = asic_byte_read(0x2F)-8;
    asic_read(buff, p_buff_param+8, len); /* parametry uzytkownika zaczynaj¹ siê od [8]*/

    return len;
}

/**************************************************************************************
 *  Funkcja kopiuje zawartoœæ bufora konfigu z uk³adu asic do wskazanego bufora 
 *  i zwraca iloœæ skopiowanych bajtów
 **************************************************************************************/
unsigned char dp_read_config(char *buff)
{
    unsigned char len;
    
    len = asic_byte_read(0x31);
    asic_read(buff, p_buff_config, len);

    return len;
}

/**************************************************************************************
 *  Funkcja zapisuje dane diagnostyczne u¿ytkownika do bufora diagnostycznego ASIC
 *  pocz¹wszy od bajtu 6. Bajty 0..5 s¹ zarezerwowane dla ASIC.
 **************************************************************************************/
int dp_diag_buff_write(char *diag_data, char data_len)
{
    unsigned char diag_buff_len = asic_byte_read(ASIC_DIAG1LN);
    static unsigned char  curr_buff_index;

    if(data_len > diag_buff_len - 6)
        return -1;
    
    if(curr_buff_index == 0)
        return 0;
    else
        asic_write(diag_data, p_buff_diag[curr_buff_index]+6, data_len);

    curr_buff_index = asic_byte_read(0x0D); /* nastepny bufor dla diagnostyki */

    return data_len;
}