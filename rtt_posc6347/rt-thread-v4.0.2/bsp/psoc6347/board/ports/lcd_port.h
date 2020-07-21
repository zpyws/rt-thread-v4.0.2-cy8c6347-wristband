 #ifndef __LCD_PORT_DSI_H__
 #define __LCD_PORT_DSI_H__

#define LCD_HEIGHT              240
#define LCD_WIDTH               120
#define LCD_BITS_PER_PIXEL      16
#define LCD_PIXEL_FORMAT        RTGRAPHIC_PIXEL_FORMAT_RGB565
#define BYTES_PER_PIXEL         (LCD_BITS_PER_PIXEL>>3)
#define LCD_BUF_SIZE            (LCD_HEIGHT*LCD_WIDTH*BYTES_PER_PIXEL)
//#define LCD_DSI_BUF_SIZE_ROUND  365040

//#define LAYER_ADDRESS           GFXMMU_VIRTUAL_BUFFER0_BASE
//#define BRIGHTNESS_MIN          50
//#define BRIGHTNESS_NORMAL       200

//#define LCD_BL_GPIO_NUM         GET_PIN(B, 14)

#define SPI_BUS_NAME                "spi1"
#define SPI_OLED_DEVICE_NAME        "ft2201"
#define DC_PIN   					GET_PIN(12,3)   
#define RES_PIN						GET_PIN(12,1)    
#define CS_PIN						GET_PIN(10,3)    
#define PWR_PIN						GET_PIN(12,4)    

#endif
