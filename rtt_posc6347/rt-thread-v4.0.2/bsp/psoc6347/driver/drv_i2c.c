/*
 * File      : drv_i2c.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-06-05     tanek        first implementation.
 * 2018-04-19     misonyo      Porting for gd32f30x
 */

#include "board.h"
#include "drv_i2c.h"
#include "drv_config.h"
//#include <rtthread.h>
//#include "gd32f30x.h"

#ifdef RT_USING_I2C

#include <rtdevice.h>

#define DBG_TAG               "drv.I2C"
#ifdef RT_I2C_DEBUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif
#include <rtdbg.h>

//by yangwensen@20200320
enum
{
#ifdef BSP_USING_I2C0
    I2C0_INDEX,
#endif
#ifdef BSP_USING_I2C1
    I2C1_INDEX,
#endif
#ifdef BSP_USING_I2C2
    I2C2_INDEX,
#endif
#ifdef BSP_USING_I2C3
    I2C3_INDEX,
#endif
#ifdef BSP_USING_I2C4
    I2C4_INDEX,
#endif
};

//by yangwensen@20200320
static const struct cy8c63_i2c_config i2c_config[] =
{
#ifdef BSP_USING_I2C0
    I2C0_CONFIG,
#endif
#ifdef BSP_USING_I2C1
    I2C1_CONFIG,
#endif
#ifdef BSP_USING_I2C2
    I2C2_CONFIG,
#endif
#ifdef BSP_USING_I2C3
    I2C3_CONFIG,
#endif
#ifdef BSP_USING_I2C4
    I2C4_CONFIG,
#endif
};

static struct cy8c63_i2c i2c_obj[sizeof(i2c_config) / sizeof(i2c_config[0])] = {0};

static int cy8c63_i2c_read(rt_uint32_t i2c_periph, rt_uint16_t slave_address, rt_uint8_t* p_buffer, rt_uint16_t data_byte)
{
#if 0
    /* wait until I2C bus is idle */
    while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, slave_address<<1, I2C_RECEIVER);

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

    if(1 == data_byte){
        /* disable acknowledge */
        i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(i2c_periph);
    }

    /* while there is data to be read */
    while(data_byte)
    {
        /* wait until the RBNE bit is set and clear it */
        if(i2c_flag_get(i2c_periph, I2C_FLAG_RBNE))
        {
            /* read a byte from the EEPROM */
            *p_buffer = i2c_data_receive(i2c_periph);

            /* point to the next location where the byte read will be saved */
            p_buffer++; 

            /* decrement the read bytes counter */
            data_byte--;
            if(1 == data_byte)
            {
                /* disable acknowledge */
                i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
                /* send a stop condition to I2C bus */
                i2c_stop_on_bus(i2c_periph);
            }
        }
    }

    /* wait until the stop condition is finished */
    while(I2C_CTL0(i2c_periph)&0x0200);

    /* enable acknowledge */
    i2c_ack_config(i2c_periph,I2C_ACK_ENABLE);

    i2c_ackpos_config(i2c_periph,I2C_ACKPOS_CURRENT);
#endif

    return 0;
}

static int cy8c63_i2c_write(rt_uint32_t i2c_periph, uint16_t slave_address, uint8_t* p_buffer, uint16_t data_byte)
{
#if 0
    /* wait until I2C bus is idle */
    while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, slave_address<<1, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while(SET != i2c_flag_get( i2c_periph , I2C_FLAG_TBE));

    /* while there is data to be read */
    while(data_byte)
    {
        i2c_data_transmit(i2c_periph, *p_buffer);

        /* point to the next byte to be written */
        p_buffer++;

        /* decrement the write bytes counter */
        data_byte --;

        /* wait until BTC bit is set */
        while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));
    }

        /* send a stop condition to I2C bus */
    i2c_stop_on_bus(i2c_periph);

    /* wait until the stop condition is finished */
    while(I2C_CTL0(i2c_periph)&0x0200);
#endif

    return 0;
}

static rt_size_t cy8c63_i2c_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct rt_i2c_msg *msg;
    rt_uint32_t i;
    rt_err_t ret = RT_ERROR;

//    struct gd32_i2c_bus *gd32_i2c = (struct gd32_i2c_bus *)bus;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];

        if (msg->flags & RT_I2C_ADDR_10BIT)
        {
//            i2c_mode_addr_config(gd32_i2c->i2c_periph,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_10BITS,0);
        }
        else
        {
//            i2c_mode_addr_config(gd32_i2c->i2c_periph,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,0);
        }
        if (msg->flags & RT_I2C_RD)
        {
//            if (gd32_i2c_read(gd32_i2c->i2c_periph, msg->addr, msg->buf, msg->len) != 0)
            {
                LOG_E("i2c bus write failed,i2c bus stop!");
                goto out;
            }
        }
        else
        {
//            if (gd32_i2c_write(gd32_i2c->i2c_periph, msg->addr, msg->buf, msg->len) != 0)
            {
                LOG_E("i2c bus write failed,i2c bus stop!");
                goto out;
            }
        }
    }
    
    ret = i;

out:
    LOG_E("send stop condition\n");

    return ret;
}

static const struct rt_i2c_bus_device_ops cy8c63_i2c_ops =
{ 
    cy8c63_i2c_xfer,
    RT_NULL,
    RT_NULL
};

int rt_hw_i2c_init(void)
{
    rt_size_t obj_num = sizeof(i2c_obj) / sizeof(struct cy8c63_i2c);
    rt_err_t result = 0;

    for (rt_size_t i = 0; i < obj_num; i++)
    {
        i2c_obj[i].config = &i2c_config[i];
        i2c_obj[i].config->i2c_init();
        
        i2c_obj[i].i2c.ops    = &cy8c63_i2c_ops;
        
        result = rt_i2c_bus_device_register(&i2c_obj[i].i2c, i2c_obj[i].config->name);

        RT_ASSERT(result == RT_EOK);
    }
    
    return result;
}
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif
/* end of i2c driver */
