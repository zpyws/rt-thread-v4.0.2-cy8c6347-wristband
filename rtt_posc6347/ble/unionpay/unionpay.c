//by yangwensen@20200729
#include <rtthread.h>
//#include <project.h>

#define LOG_TAG                         "UP"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>
//***************************************************************************************************************************
#define CBLE_RX_SIZE					20

#define MQ_CBLE_RX_NUM					5
#define MQ_CBLE_RX_BUFF_SIZE			sizeof(struct cble_pack_msg)
//***************************************************************************************************************************
rt_mq_t queue_cble_rx;

__packed struct cble_pack_msg
{
	uint8_t size;
	uint8_t data[CBLE_RX_SIZE];
};
//***************************************************************************************************************************
//by yangwensen@20200729
static void unionpay_rx_thread(void* parameter)
{
	struct cble_pack_msg msg;
	
	LOG_D("unionpay_rx_thread ready");
    for (;;)
    {
	    if( rt_mq_recv(queue_cble_rx, &msg, sizeof(msg), RT_WAITING_FOREVER) != RT_EOK )
		{
			LOG_E("rt_mq_recv() Error");
			continue;
		}
		
		ulog_hexdump("UP Pack", CBLE_RX_SIZE, msg.data, msg.size);
    }
}
//***************************************************************************************************************************
//by yangwensen@20200729
int cble_init(void)
{
    rt_thread_t thread;

    queue_cble_rx = rt_mq_create("cble_rx", MQ_CBLE_RX_BUFF_SIZE, MQ_CBLE_RX_NUM, RT_IPC_FLAG_FIFO);

    thread = rt_thread_create("unionpay_rx", unionpay_rx_thread, RT_NULL, 512, 15, 10);

    if (thread == RT_NULL)
        return RT_ERROR;
	
    return rt_thread_startup(thread);
}
//***************************************************************************************************************************
//by yangwensen@20200729
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
//***************************************************************************************************************************
