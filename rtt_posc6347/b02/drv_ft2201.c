//created by yangwensen@20200713
#include "board.h"          
#include "drv_ft2201.h"

#define SPI_BUS_NAME                "spi1"
#define SPI_OLED_DEVICE_NAME        "lcd"
#define DC_PIN   					GET_PIN(12,3)   
/*RES_PIN reset signal input. When the pin is low,
initialization of the chip is executed.*/
#define RES_PIN						GET_PIN(12,1)    
#define CS_PIN						GET_PIN(10,3)    
#define PWR_PIN						GET_PIN(12,4)    

#define OLED_DEBUG

#ifdef  OLED_DEBUG
#define OLED_TRACE                  rt_kprintf
#else
#define OLED_TRACE(...)
#endif 

/*
pin defines for 4-wire spi

This pin is Data/Command control pin. When the pin is pulled high,
the input at D7~D0 is treated as display data.When the pin is pulled
low, the input at D7~D0 will be transferred to the command register.

When serial mode is selected, D1 will be the serial data input SDIN
and D0 will be the serial clock input SCLK.

*/

struct cy8c_hw_spi_cs
{
    uint8_t pin;
};

static struct rt_spi_device spi_dev_ft2201;
static struct cy8c_hw_spi_cs  spi_cs;


rt_err_t lcd_write_cmd(const rt_uint8_t cmd)
{
    rt_size_t len;
    
    rt_pin_write(DC_PIN, PIN_LOW);    /* 命令低电平 */

    len = rt_spi_send(&spi_dev_ft2201, &cmd, 1);

    if (len != 1)
    {
        OLED_TRACE("lcd_write_cmd error. %d\r\n", len); 
        return -RT_ERROR;
    }
    else       
    {
        return RT_EOK;
    }
    
}

rt_err_t lcd_write_data(const rt_uint8_t data)
{
    rt_size_t len;
    
    rt_pin_write(DC_PIN, PIN_HIGH);        /* 数据高电平 */
    
    len = rt_spi_send(&spi_dev_ft2201, &data, 1);
    
    if (len != 1)
    {
        OLED_TRACE("lcd_write_data error. %d\r\n",len); 
        return -RT_ERROR;
    }
    else       
    {
        return RT_EOK;
    }
}

static int rt_hw_ft2201_config(void)
{
    rt_err_t res;

    spi_cs.pin = CS_PIN;
    rt_pin_mode(spi_cs.pin, PIN_MODE_OUTPUT);    /* 设置片选管脚模式为输出 */

    res = rt_spi_bus_attach_device(&spi_dev_ft2201, SPI_OLED_DEVICE_NAME, SPI_BUS_NAME, (void*)&spi_cs);
    if (res != RT_EOK)
    {
        OLED_TRACE("rt_spi_bus_attach_device error!\r\n");
        return res;
    }

    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        cfg.max_hz = 20 * 1000 *1000; /* 20M,SPI max 42MHz,ssd1351 4-wire spi */

        rt_spi_configure(&spi_dev_ft2201, &cfg);
    }

    return RT_EOK;
}
//===============================================================================================================================
void ft2201_lcd_set_brightness(uint8_t level)
{
    lcd_write_cmd(0x51);
    lcd_write_data(level);
    lcd_write_data(0x01);
    lcd_write_data(0x06);
}
//===============================================================================================================================
void ft2201_lcd_goto_sleep(void)
{
    lcd_write_cmd(0x28); 
    lcd_write_cmd(0x10);
    OLED_TRACE("oled go to sleep mode!\n");
}
//===============================================================================================================================
static int rt_hw_ft2201_init(void)
{
    rt_hw_ft2201_config();

	rt_pin_mode(PWR_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(DC_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(RES_PIN, PIN_MODE_OUTPUT);

    rt_pin_write(PWR_PIN, PIN_HIGH);
    
    rt_pin_write(RES_PIN, PIN_HIGH);
    rt_thread_delay(1);
    rt_pin_write(RES_PIN, PIN_LOW);
    rt_thread_delay(80);                //more than 70ms
    rt_pin_write(RES_PIN, PIN_HIGH);
    rt_thread_delay(130);               //more than 120ms

    lcd_write_cmd(0x00);            
    lcd_write_data(0x00);
                    
    lcd_write_cmd(0xff);            
    lcd_write_data(0x22);
    lcd_write_data(0x01);
    lcd_write_data(0x01);            
    lcd_write_data(0x00);
                
    lcd_write_cmd(0x00);            
    lcd_write_data(0x80);
                            
    lcd_write_cmd(0xff);            
    lcd_write_data(0x22);
    lcd_write_data(0x01);            
    lcd_write_data(0x00);

    //OSC
    lcd_write_cmd(0x00);            
    lcd_write_data(0x90);
    
    lcd_write_cmd(0xc1);            
    lcd_write_data(0x1e);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x91);
    lcd_write_cmd(0xc1);                    
    lcd_write_data(0x1e);
    
    //rtn
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x80);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x81);
    lcd_write_cmd(0xc0);                
    lcd_write_data(0xf1);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x82);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x83);
    lcd_write_cmd(0xc0);                
    lcd_write_data(0x12);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x84);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x85);
    lcd_write_cmd(0xc0);                
    lcd_write_data(0x12);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x86);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x87);
    lcd_write_cmd(0xc0);                
    lcd_write_data(0xf1);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x88);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x89);
    lcd_write_cmd(0xc0);                
    lcd_write_data(0x12);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x8a);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x8b);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x12);
    
                
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x90);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x91);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0xf1);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x92);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x93);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x12);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x94);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x95);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x12);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x96);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x97);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0xf1);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x98);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x99);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x12);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9a);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x00);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9b);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x12);

    //Hacitve/Vactive
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa1);
    lcd_write_cmd(0xb3);            
    lcd_write_data(0x78);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa2);
    lcd_write_cmd(0xb3);
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa3);
    lcd_write_cmd(0xb3);
    lcd_write_data(0xF0);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa4);
    lcd_write_cmd(0xb3);
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa5);
    lcd_write_cmd(0xb3);
    lcd_write_data(0xF0);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa6);
    lcd_write_cmd(0xb3);
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa7);
    lcd_write_cmd(0xb3);
    lcd_write_data(0xF0);
            
                //source resolution
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x82);
    lcd_write_cmd(0xb2);                        
    lcd_write_data(0x66);
                            
                //120 setting
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x83);
    lcd_write_cmd(0xf3);                        
    lcd_write_data(0x60);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x84);
    lcd_write_cmd(0xf3);        
    lcd_write_data(0x80);
                                            
                //GOA
                //CKV1 STR END
    lcd_write_cmd(0x00);            
    lcd_write_data(0x90);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x83);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x91);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x01);    
        
    //CKV1
    lcd_write_cmd(0x00);            
    lcd_write_data(0x95);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0xe5);//0c    //e5
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x96);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x00);//00    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x97);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0xdd);//04    //db    
            
    //CKV2 STR END
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x98);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x82);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x99);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x01);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9a);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x01);
        
                //CKV2
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9d);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x98);//b0        //95
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9e);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x12);//00        //13    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9f);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x00);//08        
    
                //STE
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf0);
    lcd_write_cmd(0xc3);                    
    lcd_write_data(0x83);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf1);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x00);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf2);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x00);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf3);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x44);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf4);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x00);//db    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf5);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xdb);//20    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf6);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x20);//00    
    //STE width
    lcd_write_cmd(0x00);            
    lcd_write_data(0x0d);
    lcd_write_cmd(0xca);                    
    lcd_write_data(0x05);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x0e);
    lcd_write_cmd(0xca);
    lcd_write_data(0x05);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x0f);
    lcd_write_cmd(0xca);
    lcd_write_data(0x05);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x10);
    lcd_write_cmd(0xca);                    
    lcd_write_data(0x05);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x11);
    lcd_write_cmd(0xca);
    lcd_write_data(0x05);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x12);
    lcd_write_cmd(0xca);
    lcd_write_data(0x05);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x13);
    lcd_write_cmd(0xca);        
    lcd_write_data(0x05);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x14);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x05);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x15);
    lcd_write_cmd(0xca);
    lcd_write_data(0x05);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x16);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x40);//  3e  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x17);
    lcd_write_cmd(0xca);        
    lcd_write_data(0x40);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x18);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x40);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x19);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x40);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x1a);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x40);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x1b);
    lcd_write_cmd(0xca);
    lcd_write_data(0x40);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x1c);
    lcd_write_cmd(0xca);
    lcd_write_data(0x40);//
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x1d);
    lcd_write_cmd(0xca);    
    lcd_write_data(0x40);//  
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x1e);
    lcd_write_cmd(0xca);
    lcd_write_data(0x40);//  
    
                //STE in skipframe
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfa);
    lcd_write_cmd(0xc3);                    
    lcd_write_data(0x7b);//82    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfb);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x0b);//03    
    
    //GSTV
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb0);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb1);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x02);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb2);
    lcd_write_cmd(0xc2);            
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb3);
    lcd_write_cmd(0xc2);            
    lcd_write_data(0x8d);//a4    //89
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb4);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x00);        
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb5);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x21);//3a    //1f    
    //EN mode
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x80);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x81);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x82);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x83);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x84);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x85);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x86);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x87);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x88);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x89);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x07);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x8a);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x00);            
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x90);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x04);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x91);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x92);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x93);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x94);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x95);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x96);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x97);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x98);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x99);
    lcd_write_cmd(0xc3);
    lcd_write_data(0xfb);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x9a);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x04);
                        
        //GOA out sel
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa0);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa1);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa2);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa3);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa4);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x15);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa5);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x05);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa6);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x06);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa7);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa8);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa9);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xaa);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xab);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xac);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xad);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x04);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xae);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x05);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xaf);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x06);
            
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb0);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb1);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb2);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb3);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb4);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb5);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb6);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb7);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x11);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb8);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x09);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xb9);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x0a);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xba);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x01);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xbb);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x02);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xbc);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x03);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xbd);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xbe);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xbf);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x16);
    
    //SW out SET
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc0);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x32);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc1);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x54);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc2);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x10);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc3);
    lcd_write_cmd(0xc2);                    
    lcd_write_data(0x23);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc4);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x45);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc5);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x01);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc6);
    lcd_write_cmd(0xc2);                                
    lcd_write_data(0x35);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc7);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x24);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xc8);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x01);        
                            
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd0);
    lcd_write_cmd(0xc2);                                
    lcd_write_data(0x32);//23
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd1);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x54);//45    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd2);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x10);//01    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd3);
    lcd_write_cmd(0xc2);                
    lcd_write_data(0x23);//32    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd4);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x45);//54    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd5);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x01);//10    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd6);
    lcd_write_cmd(0xc2);                            
    lcd_write_data(0x35);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd7);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x24);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd8);
    lcd_write_cmd(0xc2);        
    lcd_write_data(0x01);                        
                //CKH
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf0);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x00);//04    //02
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf1);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x00);//06    //02    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf2);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x02);//05    //02
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf3);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x0f);//11    //11    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf4);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x02);//05    //02    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf5);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x0f);//11    //11    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf6);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x02);//04    //02  //06
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf7);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x0f);//11    //11    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf8);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x02);//05    //02    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf9);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x0f);//11    //11    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfa);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x02);//05    //02
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfb);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x0f);//11    //11    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfc);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x02);//07    //02   //06
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xfd);
    lcd_write_cmd(0xc2);
    lcd_write_data(0x0f);//11    //11    
            //MUX                   
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa0);
    lcd_write_cmd(0xc0);                        
    lcd_write_data(0x00);//0d    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa1);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x1a);//1a
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa2);
    lcd_write_cmd(0xc0);        
    lcd_write_data(0x1a);//1a
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa3);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa4);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa5);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa6);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa7);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x00);//0d    //idle
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa8);
    lcd_write_cmd(0xc0);
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa9);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xaa);
    lcd_write_cmd(0xc0);        
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xab);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xac);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xad);
    lcd_write_cmd(0xc0);    
    lcd_write_data(0x1a);//1a                                                        
                    
    //Source pch off    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x92);
    lcd_write_cmd(0xf5);                        
    lcd_write_data(0x20);    //off
    //PCH   
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xe1);
    lcd_write_cmd(0xc2);                        
    lcd_write_data(0x00);    //off
                                                                
    //EQ
    lcd_write_cmd(0x00);            
    lcd_write_data(0xe0);
    lcd_write_cmd(0xc3);                        
    lcd_write_data(0x00);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xe1);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x11);    //22  //11
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xe2);
    lcd_write_cmd(0xc3);    
    lcd_write_data(0x00);        
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xe3);
    lcd_write_cmd(0xc3);
    lcd_write_data(0x11);    //22    //11        
            
                //SAP   
    lcd_write_cmd(0x00);            
    lcd_write_data(0x44);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0xca);    
    
                
                //AP    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x40);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0x29);    
    
    
                //chop
    lcd_write_cmd(0x00);            
    lcd_write_data(0x65);
    lcd_write_cmd(0xc4);                        
    lcd_write_data(0xc0);    //0xc0
    
    //Gamma chop off
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x68);
    lcd_write_cmd(0xc4);                        
    lcd_write_data(0x01);    
            
    
    //VCL SET
    lcd_write_cmd(0x00);            
    lcd_write_data(0x14);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0x12);        
    //AVDD
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x11);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0x4a);    //5.5   
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x12);
    lcd_write_cmd(0xc5);            
    lcd_write_data(0x4a);        
    
    //TCON
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa1); 
    lcd_write_cmd(0xc1);                        
    lcd_write_data(0xc0); 
    //video mode HS follow external, CMD mode HS follow internal        
                
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa2);
    lcd_write_cmd(0xc1);                        
    lcd_write_data(0xe3);        
    
    
    //Timeout VS
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xa8);
    lcd_write_cmd(0xc1);                        
    lcd_write_data(0x0a);        
    
    
    //cke toggle all
    lcd_write_cmd(0x00);    
    lcd_write_data(0xa8);
    lcd_write_cmd(0xc2);    
    lcd_write_data(0x54);    //28    
    
    //mipi off
    lcd_write_cmd(0x00);            
    lcd_write_data(0x90);
    lcd_write_cmd(0xff);                        
    lcd_write_data(0x80);        
    
    //vrefp5 vrefn5
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x42);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0x33);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x43);
    lcd_write_cmd(0xc5);                
    lcd_write_data(0x44);        
    
    //voltageset VGMP VGSP
    lcd_write_cmd(0x00);            
    lcd_write_data(0x31);
    lcd_write_cmd(0xc5);            
    lcd_write_data(0xd6);//cf=5.1v, fe=5.8V, e3=5.4V
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x32);
    lcd_write_cmd(0xc5);
    lcd_write_data(0xbb);//VGSP ae=2.8 ca=3.225 dd=3.51  f0=3.8 e6=3.645 e9=3.69 eb=3.72 ea=3.705 d2=3.345 da=3.465
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x33);
    lcd_write_cmd(0xc5);
    lcd_write_data(0xd6);//cf = 5.1
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x34);
    lcd_write_cmd(0xc5);
    lcd_write_data(0xbb);//ca=3.225      //d8=3.435  //d5=3.39
    
    //NB
    lcd_write_cmd(0x00);//Cmd
    lcd_write_data(0xa0);//Data
    lcd_write_cmd(0xb3);//Cmd
    lcd_write_data(0x00);//Data      
                
    //EL
    lcd_write_cmd(0x00);            
    lcd_write_data(0x01);
    lcd_write_cmd(0xcb);            
    lcd_write_data(0x29);            
    
     //mirror x
    lcd_write_cmd(0x00);            
    lcd_write_data(0xd0);
    lcd_write_cmd(0xc0);            
    lcd_write_data(0x04);            
    
    //VGH/VGL
    lcd_write_cmd(0x00);            
    lcd_write_data(0x02);
    lcd_write_cmd(0xc5);            
    lcd_write_data(0x05);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x03);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0xc5);
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x04);
    lcd_write_cmd(0xc5);            
    lcd_write_data(0x24);        //24    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x05);
    lcd_write_cmd(0xc5);
    lcd_write_data(0x24);        //24    
                //VGH/VGL pump
    lcd_write_cmd(0x00);            
    lcd_write_data(0x00);
    lcd_write_cmd(0xc5);            
    lcd_write_data(0x5b);
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0x01);
    lcd_write_cmd(0xc5);                        
    lcd_write_data(0x5b);            
    
    //vrefp5 off
    lcd_write_cmd(0x00);            
    lcd_write_data(0x6c);
    lcd_write_cmd(0xf5);            
    lcd_write_data(0x00);            
    
    //bvp3d bvn3d off
    lcd_write_cmd(0x00);            
    lcd_write_data(0x6b);
    lcd_write_cmd(0xc4);            
    lcd_write_data(0xb6);            
    
    //skipframe
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf0);
    lcd_write_cmd(0xc0);                        
    lcd_write_data(0x26);    
    
    
    lcd_write_cmd(0x00);            
    lcd_write_data(0xf4);
    lcd_write_cmd(0xc0);                        
    lcd_write_data(0x03);    //0b:5Hz 03:15Hz
    
    lcd_write_cmd(0x3A);                        
    lcd_write_data(0x05);
    
    lcd_write_cmd(0x00);                        
    lcd_write_data(0x80);    
    
    lcd_write_cmd(0xf3);                        
    lcd_write_data(0x01);
        
    lcd_write_cmd(0x11);
    
    Cy_SysLib_Delay(50);
    
    lcd_write_cmd(0x29);

	ft2201_lcd_set_brightness(170);

    OLED_TRACE("ft2201_init done\r\n");

    return 0;
}
INIT_PREV_EXPORT(rt_hw_ft2201_init);

