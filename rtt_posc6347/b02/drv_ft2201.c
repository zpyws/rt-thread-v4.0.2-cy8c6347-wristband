//created by yangwensen@20200713
#include "board.h"          
#include "drv_ft2201.h"
#include "lcd_port.h"

#define DRV_DEBUG
#define LOG_TAG             "drv.lcd"
#include <drv_log.h>


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

struct drv_ft2201_device
{
    struct rt_device parent;
    struct rt_device_graphic_info lcd_info;
    struct rt_semaphore lcd_lock;
//    rt_uint8_t *framebuffer;
};

struct drv_ft2201_device _lcd;
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
//by yangwensen@20200721
rt_inline void set_xy(uint8 xs, uint8 xe, uint8 ys, uint8 ye)
{
    lcd_write_cmd(0x2a);        //set column address
    lcd_write_data(0);
    lcd_write_data(xs);
    lcd_write_data(0);
    lcd_write_data(xe);

    lcd_write_cmd(0x2b);        //set page address
    lcd_write_data(0);
    lcd_write_data(ys);
    lcd_write_data(0);
    lcd_write_data(ye);
}
//===============================================================================================================================
//by yangwensen@20200721
void ft2201_test(void)
{
#if 0
    uint32 i;
    
    set_xy(30, 90, 60, 239);
    
    lcd_write_cmd(0x2c);        //memory write
    for(i=0; i<240*60; i++)
    {
        lcd_write_data(0xff);
    }
#endif
}
//===============================================================================================================================
static int ft2201_init(void)
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
    
    ft2201_test();

    return 0;
}
//INIT_PREV_EXPORT(rt_hw_ft2201_init);

//by yangwensen@20200721
static void drv_lcd_rect_update(struct drv_ft2201_device *lcd, struct rt_device_rect_info *rect_info)
{
    uint32 bytes;
    uint8 *p;
#if 1    
    uint8 i,j;
    set_xy(rect_info->x, rect_info->x + rect_info->width -1, rect_info->y, rect_info->y + rect_info->height -1);
//    bytes = (rect_info->width * rect_info->height) * (lcd->lcd_info.bits_per_pixel / 8);
#else
//    set_xy(0, 119, 0, 239);
    bytes = LCD_BUF_SIZE;
#endif
    p = lcd->lcd_info.framebuffer;

    lcd_write_cmd(0x2c);        //memory write
#if 1
    p = &lcd->lcd_info.framebuffer[(rect_info->y * LCD_WIDTH + rect_info->x) * 2];       //起始座标
    for(i=0; i<rect_info->height; i++)
    {
        p += (LCD_WIDTH*2);
        for(j=0; j<rect_info->width; j++)
        {
            lcd_write_data(p[j*2+0]);
            lcd_write_data(p[j*2+1]);
        }
    }
#else    
    while(bytes--)
    {
        lcd_write_data(*p++);
    }
#endif
}
static rt_err_t drv_lcd_init(struct rt_device *device)
{
    struct drv_ft2201_device *lcd = (struct drv_ft2201_device *)device;
    /* nothing, right now */
    lcd = lcd;
    return RT_EOK;
}

static rt_err_t drv_lcd_control(struct rt_device *device, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;
    struct drv_ft2201_device *lcd = (struct drv_ft2201_device *)device;
    struct rt_device_rect_info* rect_info = (struct rt_device_rect_info*)args;
    
    switch (cmd)
    {
        case RTGRAPHIC_CTRL_RECT_UPDATE:
            if(!rect_info)
            {
                LOG_E("RTGRAPHIC_CTRL_RECT_UPDATE error args");
                return -RT_ERROR;
            }
            else
                LOG_D("====> rect_info : %d %d %d %d", rect_info->x, rect_info->y, rect_info->width, rect_info->height);
            drv_lcd_rect_update(lcd, rect_info);
            break;

        case RTGRAPHIC_CTRL_POWERON:
            rt_pin_write(PWR_PIN, PIN_HIGH);
            break;
            
        case RTGRAPHIC_CTRL_POWEROFF:
            rt_pin_write(PWR_PIN, PIN_LOW);
            break;
#if 0        
        case RTGRAPHIC_CTRL_GET_INFO:
        {
            struct rt_device_graphic_info *info = (struct rt_device_graphic_info *)args;

            RT_ASSERT(info != RT_NULL);
            info->pixel_format  = lcd->lcd_info.pixel_format;
            info->bits_per_pixel = lcd->lcd_info.bits_per_pixel;
            info->width         = lcd->lcd_info.width;
            info->height        = lcd->lcd_info.height;
            info->framebuffer   = lcd->lcd_info.framebuffer;
        }
        break;
#endif
        case RTGRAPHIC_CTRL_GET_INFO:
            *(struct rt_device_graphic_info *)args = lcd->lcd_info;
            break;

        case RTGRAPHIC_CTRL_SET_MODE:
            ret = -RT_ENOSYS;
            break;
            
        case RTGRAPHIC_CTRL_GET_EXT:
            ret = -RT_ENOSYS;
            break;

        default:
            LOG_E("drv_lcd_control cmd: %d", cmd);
            break;
    }
        
    return ret;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops lcd_ops =
{
    drv_lcd_init,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    drv_lcd_control
};
#endif

int drv_lcd_hw_init(void)
{
    rt_err_t result = RT_EOK;
    struct rt_device *device = &_lcd.parent;
    
    /* memset _lcd to zero */
    memset(&_lcd, 0x00, sizeof(_lcd));

    /* init lcd_lock semaphore */
    result = rt_sem_init(&_lcd.lcd_lock, "lcd_lock", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("init semaphore failed!\n");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* config LCD dev info */
    _lcd.lcd_info.height = LCD_HEIGHT;
    _lcd.lcd_info.width = LCD_WIDTH;
    _lcd.lcd_info.bits_per_pixel = LCD_BITS_PER_PIXEL;
    _lcd.lcd_info.pixel_format = LCD_PIXEL_FORMAT;

    /* malloc memory */
    _lcd.lcd_info.framebuffer = rt_malloc_align(LCD_BUF_SIZE, 8);
    if (_lcd.lcd_info.framebuffer == RT_NULL)
    {
        LOG_E("init frame buffer failed!\n");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* memset buff to 0xFF */
    memset(_lcd.lcd_info.framebuffer, 0xFF, LCD_BUF_SIZE);

    device->type    = RT_Device_Class_Graphic;
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &lcd_ops;
#else
    device->init    = drv_lcd_init;
    device->control = drv_lcd_control;
    device->user_data = (void *)&_lcd.lcd_info;     //by yangwensen
#endif
    
    /* register lcd device */
    rt_device_register(device, "lcd", RT_DEVICE_FLAG_RDWR);

    if (ft2201_init() != RT_EOK)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    else
    {
//        turn_on_lcd_backlight();
    }

__exit:
    if (result != RT_EOK)
    {
        rt_sem_delete(&_lcd.lcd_lock);

        if (_lcd.lcd_info.framebuffer)
        {
            rt_free(_lcd.lcd_info.framebuffer);
        }
    }
    return result;
}
INIT_DEVICE_EXPORT(drv_lcd_hw_init);

#ifdef DRV_DEBUG
#ifdef FINSH_USING_MSH
int lcd_test()
{
    struct drv_ft2201_device *lcd;
    lcd = (struct drv_ft2201_device *)rt_device_find("lcd");
    rt_uint8_t *ptr = lcd->lcd_info.framebuffer;
    struct rt_device_rect_info rect_info = {0, 0, LCD_WIDTH, LCD_HEIGHT};
    
    while (1)
    {
        /* red */
        for (unsigned long i = 0; i < LCD_BUF_SIZE/BYTES_PER_PIXEL; i++)
        {
            ptr[2 * i] = 0xf1;
            ptr[2 * i + 1] = 0x00;
        }
        rt_device_control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
        rt_thread_mdelay(1000);

        /* green */
        for (int i = 0; i < LCD_BUF_SIZE/BYTES_PER_PIXEL; i++)
        {
            ptr[2 * i] = 0x07;
            ptr[2 * i + 1] = 0xe0;
        }
        rt_device_control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
        rt_thread_mdelay(1000);

        /* blue */
        for (int i = 0; i < LCD_BUF_SIZE/BYTES_PER_PIXEL; i++)
        {
            ptr[2 * i] = 0x00;
            ptr[2 * i + 1] = 0x1f;
        }
        rt_device_control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
        rt_thread_mdelay(1000);
    }
}
MSH_CMD_EXPORT(lcd_test, lcd_test);

//draw a line in screen
void line()
{
    struct drv_ft2201_device *lcd;
    lcd = (struct drv_ft2201_device *)rt_device_find("lcd");
    rt_uint8_t *ptr = lcd->lcd_info.framebuffer;

        /* red */
        for (unsigned long long i = LCD_BUF_SIZE/4/2; i <LCD_BUF_SIZE/4/2+390; i++)
        {
                ptr[4 * i] = 0x00;
                ptr[4 * i + 1] = 0x00;
                ptr[4 * i + 2] = 0xFF;
                ptr[4 * i + 3] = 0xFF;
        }
        rt_device_control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);


}
MSH_CMD_EXPORT(line, line);

#endif /* FINSH_USING_MSH */
#endif /* DRV_DEBUG */
