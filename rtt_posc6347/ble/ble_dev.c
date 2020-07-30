#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "ble_dev.h"

#define DBG_TAG    "BLE"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>
//****************************************************************************************************************************
#if 0
rt_inline int _ble_poll_rx(struct rt_ble_device *ble, rt_uint8_t *data, int length)
{
    int ch;
    int size;

    RT_ASSERT(ble != RT_NULL);
    size = length;

    while (length)
    {
        ch = ble->ops->getc(ble);
        if (ch == -1) break;

        *data = ch;
        data ++; length --;

//        if (ch == '\n') break;
    }

    return size - length;
}
#endif
//****************************************************************************************************************************
rt_inline int _ble_poll_tx(struct rt_ble_device *ble, const rt_uint8_t *data, int length)
{
    int size;
    RT_ASSERT(ble != RT_NULL);

    size = length;
    while (length)
    {
        /*
         * to be polite with serial console add a line feed
         * to the carriage return character
         */
        if (*data == '\n' && (ble->parent.open_flag & RT_DEVICE_FLAG_STREAM))
        {
            ble->ops->putc(ble, '\r');
        }

        ble->ops->putc(ble, *data);

        ++ data;
        -- length;
    }

    return size - length;
}
//****************************************************************************************************************************
rt_inline int _ble_int_rx(struct rt_ble_device *ble, rt_uint8_t *data, int length)
{
    int size;
    struct rt_ble_rx_fifo* rx_fifo;

    RT_ASSERT(ble != RT_NULL);
    size = length;

    rx_fifo = (struct rt_ble_rx_fifo*) ble->ble_rx;
    RT_ASSERT(rx_fifo != RT_NULL);

    /* read from software FIFO */
    while (length)
    {
        int ch;
        rt_base_t level;

        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == RT_FALSE))
        {
            /* no data, enable interrupt and break out */
            rt_hw_interrupt_enable(level);
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= ble->config.bufsz) rx_fifo->get_index = 0;

        if (rx_fifo->is_full == RT_TRUE)
        {
            rx_fifo->is_full = RT_FALSE;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        *data = ch & 0xff;
        data ++; length --;
    }

    return size - length;
}
//****************************************************************************************************************************
rt_inline int _ble_int_tx(struct rt_ble_device *ble, const rt_uint8_t *data, int length)
{
    int size;
    struct rt_ble_tx_fifo *tx;

    RT_ASSERT(ble != RT_NULL);

    size = length;
    tx = (struct rt_ble_tx_fifo*) ble->ble_tx;
    RT_ASSERT(tx != RT_NULL);

    while (length)
    {
        if (ble->ops->putc(ble, *(char*)data) == -1)
        {
            rt_completion_wait(&(tx->completion), RT_WAITING_FOREVER);
            continue;
        }

        data ++; length --;
    }

    return size - length;
}
//****************************************************************************************************************************
static void _ble_check_buffer_size(void)
{
    static rt_bool_t already_output = RT_FALSE;
		
    if (already_output == RT_FALSE)
    {
        LOG_W("Warning: There is no enough buffer for saving data,"
              " please increase the RT_SERIAL_RB_BUFSZ option.");
        already_output = RT_TRUE;
    }
}	
//****************************************************************************************************************************
static rt_err_t rt_ble_init(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    ble = (struct rt_ble_device *)dev;

    /* initialize rx/tx */
    ble->ble_rx = RT_NULL;
    ble->ble_tx = RT_NULL;

    /* apply configuration */
    if (ble->ops->configure)
        result = ble->ops->configure(ble, &ble->config);

    return result;
}
//****************************************************************************************************************************
static rt_err_t rt_ble_open(struct rt_device *dev, rt_uint16_t oflag)
{
    rt_uint16_t stream_flag = 0;
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    ble = (struct rt_ble_device *)dev;

    LOG_D("open ble device: 0x%08x with open flag: 0x%04x",
        dev, oflag);
    /* check device flag with the open flag */
    if ((oflag & RT_DEVICE_FLAG_DMA_RX) && !(dev->flag & RT_DEVICE_FLAG_DMA_RX))
        return -RT_EIO;
    if ((oflag & RT_DEVICE_FLAG_DMA_TX) && !(dev->flag & RT_DEVICE_FLAG_DMA_TX))
        return -RT_EIO;
    if ((oflag & RT_DEVICE_FLAG_INT_RX) && !(dev->flag & RT_DEVICE_FLAG_INT_RX))
        return -RT_EIO;
    if ((oflag & RT_DEVICE_FLAG_INT_TX) && !(dev->flag & RT_DEVICE_FLAG_INT_TX))
        return -RT_EIO;

    /* keep steam flag */
    if ((oflag & RT_DEVICE_FLAG_STREAM) || (dev->open_flag & RT_DEVICE_FLAG_STREAM))
        stream_flag = RT_DEVICE_FLAG_STREAM;

    /* get open flags */
    dev->open_flag = oflag & 0xff;

    /* initialize the Rx/Tx structure according to open flag */
    if (ble->ble_rx == RT_NULL)
    { 
        if (oflag & RT_DEVICE_FLAG_INT_RX)
        {
            struct rt_ble_rx_fifo* rx_fifo;

            rx_fifo = (struct rt_ble_rx_fifo*) rt_malloc (sizeof(struct rt_ble_rx_fifo) +
                ble->config.bufsz);
            RT_ASSERT(rx_fifo != RT_NULL);
            rx_fifo->buffer = (rt_uint8_t*) (rx_fifo + 1);
            rt_memset(rx_fifo->buffer, 0, ble->config.bufsz);
            rx_fifo->put_index = 0;
            rx_fifo->get_index = 0;
            rx_fifo->is_full = RT_FALSE;

            ble->ble_rx = rx_fifo;
            dev->open_flag |= RT_DEVICE_FLAG_INT_RX;
            /* configure low level device */
            ble->ops->control(ble, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_RX);
        }
        else
        {
            ble->ble_rx = RT_NULL;
        }
    }
    else
    {
        if (oflag & RT_DEVICE_FLAG_INT_RX)
            dev->open_flag |= RT_DEVICE_FLAG_INT_RX;
    }

    if (ble->ble_tx == RT_NULL)
    {
        if (oflag & RT_DEVICE_FLAG_INT_TX)
        {
            struct rt_ble_tx_fifo *tx_fifo;

            tx_fifo = (struct rt_ble_tx_fifo*) rt_malloc(sizeof(struct rt_ble_tx_fifo));
            RT_ASSERT(tx_fifo != RT_NULL);

            rt_completion_init(&(tx_fifo->completion));
            ble->ble_tx = tx_fifo;

            dev->open_flag |= RT_DEVICE_FLAG_INT_TX;
            /* configure low level device */
            ble->ops->control(ble, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_TX);
        }
        else
        {
            ble->ble_tx = RT_NULL;
        }
    }
    else
    {
        if (oflag & RT_DEVICE_FLAG_INT_TX)
            dev->open_flag |= RT_DEVICE_FLAG_INT_TX;
    }

    /* set stream flag */
    dev->open_flag |= stream_flag;

    return RT_EOK;
}
//****************************************************************************************************************************
static rt_err_t rt_ble_close(struct rt_device *dev)
{
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    ble = (struct rt_ble_device *)dev;

    /* this device has more reference count */
    if (dev->ref_count > 1) return RT_EOK;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_RX)
    {
        struct rt_ble_rx_fifo* rx_fifo;

        rx_fifo = (struct rt_ble_rx_fifo*)ble->ble_rx;
        RT_ASSERT(rx_fifo != RT_NULL);

        rt_free(rx_fifo);
        ble->ble_rx = RT_NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_INT_RX;
        /* configure low level device */
        ble->ops->control(ble, RT_DEVICE_CTRL_CLR_INT, (void*)RT_DEVICE_FLAG_INT_RX);
    }
    
    if (dev->open_flag & RT_DEVICE_FLAG_INT_TX)
    {
        struct rt_ble_tx_fifo* tx_fifo;

        tx_fifo = (struct rt_ble_tx_fifo*)ble->ble_tx;
        RT_ASSERT(tx_fifo != RT_NULL);

        rt_free(tx_fifo);
        ble->ble_tx = RT_NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_INT_TX;
        /* configure low level device */
        ble->ops->control(ble, RT_DEVICE_CTRL_CLR_INT, (void*)RT_DEVICE_FLAG_INT_TX);
    }
    return RT_EOK;
}
//****************************************************************************************************************************
static rt_size_t rt_ble_read(struct rt_device *dev,
                                rt_off_t          pos,
                                void             *buffer,
                                rt_size_t         size)
{
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    ble = (struct rt_ble_device *)dev;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_RX)
    {
        return _ble_int_rx(ble, (rt_uint8_t *)buffer, size);
    }

//    return _ble_poll_rx(ble, (rt_uint8_t *)buffer, size);
	return 0;
}
//****************************************************************************************************************************
static rt_size_t rt_ble_write(struct rt_device *dev,
                                 rt_off_t          pos,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    ble = (struct rt_ble_device *)dev;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_TX)
    {
        return _ble_int_tx(ble, (const rt_uint8_t *)buffer, size);
    }
    else
    {
        return _ble_poll_tx(ble, (const rt_uint8_t *)buffer, size);
    }
}
//****************************************************************************************************************************
static rt_err_t rt_ble_control(struct rt_device *dev,
                                  int              cmd,
                                  void             *args)
{
    rt_err_t ret = RT_EOK;
    struct rt_ble_device *ble;

    RT_ASSERT(dev != RT_NULL);
    ble = (struct rt_ble_device *)dev;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_SUSPEND:
            /* suspend device */
            dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
            break;

        case RT_DEVICE_CTRL_RESUME:
            /* resume device */
            dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
            break;

        case RT_DEVICE_CTRL_CONFIG:
            if (args)
            {
                struct ble_configure *pconfig = (struct ble_configure *) args;
                if (pconfig->bufsz != ble->config.bufsz && ble->parent.ref_count)
                {
                    /*can not change buffer size*/
                    return RT_EBUSY;
                }
                /* set serial configure */
                ble->config = *pconfig;
                if (ble->parent.ref_count)
                {
                    /* serial device has been opened, to configure it */
                    ble->ops->configure(ble, (struct ble_configure *) args);
                }
            }

            break;

        default :
            /* control device */
            ret = ble->ops->control(ble, cmd, args);
            break;
    }

    return ret;
}
//****************************************************************************************************************************
rt_err_t rt_hw_ble_register(struct rt_ble_device *ble,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data)
{
    rt_err_t ret;
    struct rt_device *device;
    RT_ASSERT(ble != RT_NULL);

    device = &(ble->parent);

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &ble_ops;
#else
    device->init        = rt_ble_init;
    device->open        = rt_ble_open;
    device->close       = rt_ble_close;
    device->read        = rt_ble_read;
    device->write       = rt_ble_write;
    device->control     = rt_ble_control;
#endif
    device->user_data   = data;

    /* register a character device */
    ret = rt_device_register(device, name, flag);

#if defined(RT_USING_POSIX)
    /* set fops */
    device->fops        = &_ble_fops;
#endif

    return ret;
}
//****************************************************************************************************************************
void rt_hw_ble_isr(struct rt_ble_device *ble, int event)
{
    switch (event & 0xff)
    {
        case RT_BLE_EVENT_RX_IND:
        {
            int ch = -1;
            rt_base_t level;
            struct rt_ble_rx_fifo* rx_fifo;

            /* interrupt mode receive */
            rx_fifo = (struct rt_ble_rx_fifo*)ble->ble_rx;
            RT_ASSERT(rx_fifo != RT_NULL);

            while (1)
            {
                ch = ble->ops->getc(ble);
                if (ch == -1) break;


                /* disable interrupt */
                level = rt_hw_interrupt_disable();

                rx_fifo->buffer[rx_fifo->put_index] = ch;
                rx_fifo->put_index += 1;
                if (rx_fifo->put_index >= ble->config.bufsz) rx_fifo->put_index = 0;

                /* if the next position is read index, discard this 'read char' */
                if (rx_fifo->put_index == rx_fifo->get_index)
                {
                    rx_fifo->get_index += 1;
                    rx_fifo->is_full = RT_TRUE;
                    if (rx_fifo->get_index >= ble->config.bufsz) rx_fifo->get_index = 0;

                    _ble_check_buffer_size();
                }

                /* enable interrupt */
                rt_hw_interrupt_enable(level);
            }

            /* invoke callback */
            if (ble->parent.rx_indicate != RT_NULL)
            {
                rt_size_t rx_length;

                /* get rx length */
                level = rt_hw_interrupt_disable();
                rx_length = (rx_fifo->put_index >= rx_fifo->get_index)? (rx_fifo->put_index - rx_fifo->get_index):
                    (ble->config.bufsz - (rx_fifo->get_index - rx_fifo->put_index));
                rt_hw_interrupt_enable(level);

                if (rx_length)
                {
                    ble->parent.rx_indicate(&ble->parent, rx_length);
                }
            }
            break;
        }
        case RT_BLE_EVENT_TX_DONE:
        {
            struct rt_ble_tx_fifo* tx_fifo;

            tx_fifo = (struct rt_ble_tx_fifo*)ble->ble_tx;
            rt_completion_done(&(tx_fifo->completion));
            break;
        }
    }
}
//****************************************************************************************************************************
