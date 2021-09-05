#include "main.h"
#include "asic_io.h"
#include "dp.h"
#include "modbus.h"
#include "usart.h"
//#include "parser.h"

#define MAINCK              18400000
#define MASTERCLK           48000000

#define DP_USER_WATCHDOG    0xFFFF
#define DP_SLAVE_IDENT      0xABCD
#define DP_SLAVE_ADDRESS    5

#define MB_MAX_JOBS         10


const unsigned char test_conf[] = {0x00,0x00,0xA1,0x0F,0x35, 0xC0,0x01,0x01,0x00,0x04,0x00,0x00,0x0A, 0xA0,0x01,0x07,0x00,0x00,0x00,0x02,0x0A, 0x72,0x02,0x0B,0x00,0x00,0x00,0x00,0x0A, 0x74,0x03,0x4B,0x00,0x16,0x00,0x21,0xff};
extern unsigned long uptime;

/******************************************************
*    Konfiguracja portow 2011.02.06
*******************************************************/
void io_setup()
{
    PIOA_ODR  =  0xffffffff;    // PIOA output disable register 
    PIOA_OER  =  0x03000004;    // PIOA output enable register
    PIOA_PUDR =  0x03000004;    // PIOA pull-up off
    PIOA_PER  =  0x03807384;    // PIOA enable register
    PIOA_PDR  =  0x781F8C6B;    // PIOA PER A select register
    PIOA_ASR  =  0x781F8C6B;    // PIOA PER A select register


    PIOB_ODR  = 0xffffffff;    // PIOB output disable register
    PIOB_OER  = 0x7807BC00;    // PIOB output enable register
    PIOB_PUDR = 0x7807BC00;    // PIOB pull-up off
    PIOB_PER  = 0x7FFFFC00;    // PIOB enable register
    PIOB_PDR  = 0x000003FF;    // PIOB PER A select register
    PIOB_ASR  = 0x000003FF;    // PIOB PER A select register
    PIOB_OWER = 0x7FF80000;    // maska, dla szyny adresow/danych dla PIOA_ODSR

    PMC_PCER = 1<<2 | 1<<3;  // enable PIO clocks
}

void irq_setup()
{
    //AIC_SVR0 =  (*(volatile unsigned long *)asic_irq);    //  FIQ
    //AIC_SMR0 = 1<<5;
    AIC_IECR = 1;
    AIC_SVR1 = (unsigned long)sys_irq;     //  sys. timer irq from TC0
    AIC_SVR6 = (unsigned long)US0_IRQ;     //  usart485 irq
    AIC_SVR7 = (unsigned long)US1_IRQ;     //  usart232 i
    AIC_IECR = 1<<7;
}

void sys_timer_setup(unsigned int freq)
{  
    PIT_MR = (48000000/(16*freq))&0xFFFFF;
    PIT_MR |= 3<<24; 
    AIC_IECR = 1<<1;
}

void dp_init()
{
    dp_io_buff_alloc(248, 248);
    dp_diag_buff_alloc(6);
    dp_cnfg_param_buff_alloc(64,64);

    DP_SET_SLAVE_ID(DP_SLAVE_IDENT);
    DP_SET_SLAVE_ADDRESS(read_addr_switch()); 

    ASIC_INT_ACK(0xFFFF);
    ASIC_INT_MASK(0xFFFF);
    ASIC_END_IRQ();
    ASIC_SET_BAUD_WDG(20);
    DP_SET_USER_WDG(20);
}


int main()
{
    unsigned char pb_slave_address = 0;
    volatile int i = 1000000;
    unsigned char diag_buff[30];
    char in_buff[256];
    char out_buff[256];
    _mb_job mb_job[MB_MAX_JOBS+1];
    _usart_config usart_conf;

  
    parse_params(&usart_conf, mb_job, test_conf, 37);

    while(i) i--;     // zwloka na ustabilizowanie

    io_setup();
    AIC_IDCR = 0xFFFFFFFF;
   
    
    irq_setup();
    led_off(LED1A|LED2A|LED2B|LED3A|LED3B);
    dbgu_init(115200, 128);
    asic_reset(1000000);
    dp_init();
    

    dbgu_print("\n\n\n\r*****************************************\n\r");
    dbgu_print("*********PROFIBUS MODBUS GATE************\n\r");
    dbgu_print("*****************************************\n\r");
    dbgu_printf(0, "Kompilacja: %s %s \n\r", __DATE__, __TIME__);


    if(ASIC_VERSION != 0x0d)
    {
        led_on(LED1A);
        dbgu_print("\n\rASIC Error!\n\r");
    }
    else  dbgu_print("\n\rASIC OK..\n\r");

    if(profibus_voltage_chck())
    {
        dbgu_print("Profibus voltage fault!!\n\r");
        led_on(LED2A);
    }
    else
    {
        profibus_drv_enable();
        dbgu_print("Profibus driver OK..\n\r");
    }
    if(modbus_voltage_chck())
    {
        dbgu_print("Modbus voltage fault!!\n\r");
        led_on(LED2A);
    }
    else  dbgu_print("Modbus driver OK..\n\r");
  

    dbgu_print("Run..\n\r");
    led_on(LED3B);
  
    sys_timer_setup(20);
    blink_mask = LED3B;


    AIC_SVR7 =  US1_IRQ;     //  usart232 i
    AIC_IECR = 1<<7;
   __ENA_IRQ();
    
    usart_config(9600, 0, 0);

    


    memset(in_buff,0, 256);
    memset(out_buff,0, 256);
    memset(diag_buff,0, 256);
    modbus_init(USART1_BASE, in_buff, out_buff, mb_job, diag_buff);
    
    led_on(LED2A);




    // ASIC MODE REG 0 
    asic_byte_write(ASIC_MR0L, 0x00);
    asic_byte_write(ASIC_MR0H, 0x01);       // dp mode

    // ASIC MODE REG 1
    asic_byte_write(ASIC_MR1S, 0x27);       // go online

    
    unsigned char tmp_irrl, tmp_irrh, irrl, irrh, len, tmp;

    char params[32], bufft[10];
    char config[32];

    tmp = asic_byte_read(0x30);

    dbgu_printf(0, "-%x \r\n", tmp);


    
      config[0] = 0xa; // <== W#16#0A0B (MSB first)
      config[1] = 0xb; //

    while(1){        
    
    irrl = asic_byte_read(0x0);
    irrh = asic_byte_read(0x1);


    
    //if(irrl != tmp_irrl || irrh != tmp_irrh)
    {
        dbgu_printf(0,"irrl: %x irrh: %x\n\r", bufft[0], bufft[1]);
    }

    dp_write_to_master(config, 0, 2);

    if(irrh & 0x08)
    {
     
       //mb_stop_dexch
       len =  dp_read_usr_params(params);
       parse_params(&usart_conf, mb_job, params, len);

       if(params[2] == 0)
          asic_byte_read(0x0f);
       else
          asic_byte_read(0x0E);

       ASIC_INT_ACK(0x0800);
    }

    if(irrh & 0x04)
    {
  //  len =      dp_read_config(config);
        asic_byte_read(0x10);
       

        dp_set_io_len(2,2);
      
       ASIC_INT_ACK(0x0400);
    }
    DP_ACK_USER_WDG();

    dp_read_from_master(bufft, 0, 2);


 //  mb_state();

//    if(uptime & 1)
    dp_diag_buff_write("\xaa\xbb\xcc\xdd",4);
    

    }
    return 0;
}


void stop()
{
    blink_mask = LED1A|LED2A|LED3A;

    profibus_drv_disable();
    modbus_drv_disable();

  while(1);
}
