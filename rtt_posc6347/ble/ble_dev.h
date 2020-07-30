//created by yangwensen@20200730
#ifndef __YWS_BLE_H__
#define __YWS_BLE_H__
//****************************************************************************************	
#include <rtthread.h>

#ifndef RT_BLE_RB_BUFSZ
#define RT_BLE_RB_BUFSZ              64
#endif
	
#define RT_BLE_EVENT_RX_IND          0x01    /* Rx indication */
#define RT_BLE_EVENT_TX_DONE         0x02    /* Tx complete   */
#define RT_BLE_EVENT_RX_DMADONE      0x03    /* Rx DMA transfer done */
#define RT_BLE_EVENT_TX_DMADONE      0x04    /* Tx DMA transfer done */
#define RT_BLE_EVENT_RX_TIMEOUT      0x05    /* Rx timeout    */
#define RT_BLE_EVENT_TX_TIMEOUT      0x06    /* Tx timeout    */

#define RT_BLE_RX_INT                0x01
#define RT_BLE_TX_INT                0x02

#define RT_BLE_TX_DATAQUEUE_SIZE     2048
#define RT_BLE_TX_DATAQUEUE_LWM      30

#define RT_BLE_CONFIG_DEFAULT		\
{									\
    1000000,						\
    23,								\
	RT_BLE_RB_BUFSZ,				\
}
	
	
struct ble_configure
{
    rt_uint32_t speed;
	uint16_t mtu;
	uint16_t bufsz;
};

/*
 * Serial FIFO mode 
 */
struct rt_ble_rx_fifo
{
    /* software fifo */
    rt_uint8_t *buffer;

    rt_uint16_t put_index, get_index;

    rt_bool_t is_full;
};

struct rt_ble_tx_fifo
{
    struct rt_completion completion;
};

struct rt_ble_device
{
    struct rt_device          parent;

    const struct rt_ble_ops *ops;
    struct ble_configure   config;

    void *ble_rx;
    void *ble_tx;
};
typedef struct rt_ble_device rt_ble_t;

/**
 * uart operators
 */
struct rt_ble_ops
{
    rt_err_t (*configure)(struct rt_ble_device *ble, struct ble_configure *cfg);
    rt_err_t (*control)(struct rt_ble_device *ble, int cmd, void *arg);

    int (*putc)(struct rt_ble_device *serial, char c);
    int (*getc)(struct rt_ble_device *serial);
};

void rt_hw_ble_isr(struct rt_ble_device *ble, int event);

rt_err_t rt_hw_ble_register(struct rt_ble_device *ble,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data);
//****************************************************************************************	
#endif
