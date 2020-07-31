//by yangwensen@20200729
#include <rtthread.h>
#include <rtdevice.h>
#include "ble_dev.h"
//#include <project.h>

#define LOG_TAG                         "UP"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>
//***************************************************************************************************************************
#define CBLE_RX_SIZE					20

#define MQ_CBLE_RX_NUM					5
#define MQ_CBLE_RX_BUFF_SIZE			sizeof(struct cble_pack_msg)

#define CBLE_RX_DEV_NAME                "ble_up_rx"
#define CBLE_RX_BUFF_SIZE				200
//***************************************************************************************************************************
static struct rt_semaphore rx_sem;
static rt_device_t cble_rx_device = RT_NULL;
static uint8_t cble_rx_buff[CBLE_RX_BUFF_SIZE];
//***************************************************************************************************************************
//function prototype
//***************************************************************************************************************************
//by yangwensen@20200730
static rt_err_t cble_rx_ind(rt_device_t dev, rt_size_t size)
{
    if (size > 0)
    {
        rt_sem_release(&rx_sem);
    }
    return RT_EOK;
}
//***************************************************************************************************************************
static int cble_rx_init(void)
{
    cble_rx_device = rt_device_find(CBLE_RX_DEV_NAME);
    
    if(cble_rx_device == RT_NULL )
    {
        LOG_E("find device %s failed!", CBLE_RX_DEV_NAME);
        return -1;
    }
	
    struct ble_configure rx_use_config = 
    {
		1000000ul,
		23,
        CBLE_RX_BUFF_SIZE, /* Buffer size */
    };
    
    if (RT_EOK != rt_device_control(cble_rx_device, RT_DEVICE_CTRL_CONFIG,(void *)&rx_use_config))
    {
        LOG_E("cble rx node config failed.");
        return -2;
    }
    
    rt_sem_init(&rx_sem, "up_rx_sem", 0, RT_IPC_FLAG_FIFO);
    
    if (rt_device_open(cble_rx_device, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        LOG_E("cble rx node open error.");
        return -3;
    }    
    
    rt_device_set_rx_indicate(cble_rx_device, cble_rx_ind);

	return RT_EOK;
}
//***************************************************************************************************************************
//by yangwensen@20200729
static void unionpay_rx_thread(void* parameter)
{
    rt_size_t res;
	
	cble_rx_init();
	
    for (;;)
    {
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        res = rt_device_read(cble_rx_device, -1, cble_rx_buff, sizeof(cble_rx_buff));
        
		ulog_hexdump("UP-PACK", CBLE_RX_SIZE, cble_rx_buff, res);
    }
}
//***************************************************************************************************************************
//by yangwensen@20200729
int cble_init(void)
{
    rt_thread_t thread;

    thread = rt_thread_create("unionpay_rx", unionpay_rx_thread, RT_NULL, 1024, 15, 10);

    if (thread == RT_NULL)
        return RT_ERROR;
	
    return rt_thread_startup(thread);
}
//***************************************************************************************************************************
//by yangwensen@20200729
#if 0
int8_t cble_data_in(uint8_t *data, uint8_t size)
{
	struct cble_pack_msg msg;
	rt_err_t err;
	
	if(size > sizeof(msg.data))
	{
		LOG_E("pack size exceed %d bytes", sizeof(msg.data));
		msg.size = sizeof(msg.data);
	}
	else
		msg.size = size;
	
	rt_memcpy(msg.data, data, msg.size);
	
	err = rt_mq_send(queue_cble_rx, &msg, sizeof(msg));
	if( err != RT_EOK )
	{
		LOG_E("rt_mq_send() Error: %d", err);
		return -1;
	}
	
	return 0;
}
#endif
//***************************************************************************************************************************
