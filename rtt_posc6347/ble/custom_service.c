//created by yangwensen@20200729
#include <rtthread.h>
#include <project.h>
#include <rtdevice.h>
#include "ble_dev.h"

#define LOG_TAG                         "GATT"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>

#include "custom_service.h"
//***************************************************************************************************************************
#define ARRAY_SIZE(ar)     (sizeof(ar)/sizeof(ar[0]))
//***************************************************************************************************************************
enum
{
	BLE_UNIONPAY_TX_INDEX = 0,
	BLE_UNIONPAY_RX_INDEX,
};

typedef struct
{
	cy_ble_gatt_db_attr_handle_t cccdHandle;
	cy_ble_gatt_db_attr_handle_t attrHandle;
}ble_service_access_node_t;

struct ble_custom_service_config
{
    const char *name;
	const uint32_t rw_flag;
    const ble_service_access_node_t access_node;
};

struct ble_char_node
{
    const struct ble_custom_service_config *config;
    struct rt_ble_device ble;
};

static const struct ble_custom_service_config BLE_SERVIE_CONFIG[] =
{
	{
		"ble_up_tx", 
		RT_DEVICE_FLAG_WRONLY, 
		{CY_BLE_UNIONPAY_OUT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE,CY_BLE_UNIONPAY_OUT_CHAR_HANDLE}
	},
	{
		"ble_up_rx", 
		RT_DEVICE_FLAG_RDONLY|RT_DEVICE_FLAG_INT_RX, 
		{RT_NULL,CY_BLE_UNIONPAY_IN_CHAR_HANDLE}
	},
};

static struct ble_char_node ble_service_obj[sizeof(BLE_SERVIE_CONFIG) / sizeof(BLE_SERVIE_CONFIG[0])] = {0};

ble_reg_t ble_reg = {0};
//***************************************************************************************************************************
//function prototype
//from unionpay.c
extern int8_t cble_data_in(uint8_t *data, uint8_t size);
//from btstack.c
extern cy_stc_ble_conn_handle_t appConnHandle;
//***************************************************************************************************************************
//by yangwensen@20200729
int8_t gatt_write_request(cy_stc_ble_gatt_write_param_t *p)
{
	cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_gatt_err_param_t err_param =
    {
        .errInfo.opCode     = CY_BLE_GATT_WRITE_REQ,
        .errInfo.attrHandle = p->handleValPair.attrHandle,
        .connHandle         = p->connHandle
    };
	
	if(p==NULL)
		return -1;
	
	if(p->handleValPair.attrHandle == CY_BLE_UNIONPAY_OUT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
	{
		cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo = 
        {
            .handleValuePair = p->handleValPair,
            .connHandle      = p->connHandle,
            .offset          = 0u,
            .flags           = CY_BLE_GATT_DB_PEER_INITIATED
        };
		
		LOG_D("Write UNIONPAY CCCD:%02X", p->handleValPair.value.val[0]);
		gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
		if(gattErr != CY_BLE_GATT_ERR_NONE)
		{
			LOG_E("Cy_BLE_GATTS_WriteAttributeValueCCCD() Error: 0x%x", gattErr);
		}
	}
    else
    {
        uint8_t i;
        
        for(i=0; i<ARRAY_SIZE(BLE_SERVIE_CONFIG); i++)
        {
            if(!(ble_service_obj[i].config->rw_flag & RT_DEVICE_FLAG_RDONLY))
                continue;
            
            if(p->handleValPair.attrHandle != ble_service_obj[i].config->access_node.attrHandle)
                continue;
            
//    		LOG_D("Data in %d bytes", p->handleValPair.value.len);
            
            ble_reg.rd_index = 0;
            ble_reg.rx_len = (p->handleValPair.value.len > sizeof(ble_reg.rx_buff)) ? sizeof(ble_reg.rx_buff) : p->handleValPair.value.len;
            rt_memcpy(ble_reg.rx_buff, p->handleValPair.value.val, ble_reg.rx_len);
            rt_hw_ble_isr(&(ble_service_obj[i].ble), RT_BLE_EVENT_RX_IND);
        }
    }
//-----------------------------------------------------------------------------------------------------
	if(gattErr != CY_BLE_GATT_ERR_NONE)
	{
		LOG_E("Cy_BLE_GATTS_WriteAttributeValueCCCD() Error: 0x%x", gattErr);
		err_param.errInfo.errorCode  = gattErr;
		if( Cy_BLE_GATTS_ErrorRsp(&err_param) != CY_BLE_SUCCESS )
			LOG_E("Cy_BLE_GATTS_ErrorRsp() Error");
			
		return -2;
	}
	
	Cy_BLE_GATTS_WriteRsp(p->connHandle);
	return 0;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static int8_t ble_send_notification(cy_stc_ble_conn_handle_t connHandle, cy_ble_gatt_db_attr_handle_t attrHandle, uint8_t *buff, uint32_t len)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
#if 0    
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
		LOG_E("ble disconnected!");
        return -1;
    }
	
    if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cccdHandle))
	{
		LOG_E("notification not enabled");
		return -2;
	}
#endif	
	while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY)
	{
	    Cy_BLE_ProcessEvents();
	}

    cy_stc_ble_gatts_handle_value_ntf_t notify_handle =
    {
        .connHandle = connHandle,
        .handleValPair.attrHandle = attrHandle,
        .handleValPair.value.len = len,
        .handleValPair.value.val = buff,
    };

	apiResult = Cy_BLE_GATTS_Notification(&notify_handle);
    if(apiResult != CY_BLE_SUCCESS)
    {
        LOG_E("Cy_BLE_GATTS_Notification API Error: 0x%x", apiResult);
		return -3;
    }

	return 0;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static int8_t ble_send_indication(cy_stc_ble_conn_handle_t connHandle, cy_ble_gatt_db_attr_handle_t attrHandle, uint8_t *buff, uint32_t len)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    rt_uint32_t event = 0;
    rt_err_t error;
#if 0    
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
		LOG_E("ble disconnected!");
        return -1;
    }
	
    if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cccdHandle))
	{
		LOG_E("notification not enabled");
		return -2;
	}
#endif	
	while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY)
	{
	    Cy_BLE_ProcessEvents();
	}

    cy_stc_ble_gatts_handle_value_ind_t indicate_handle =
    {
        .connHandle = connHandle,
        .handleValPair.attrHandle = attrHandle,
        .handleValPair.value.len = len,
        .handleValPair.value.val = buff,
    };

	apiResult = Cy_BLE_GATTS_Indication(&indicate_handle);
    if(apiResult != CY_BLE_SUCCESS)
    {
        LOG_E("Cy_BLE_GATTS_Notification API Error: 0x%x", apiResult);
		return -3;
    }
//-----------------------------------------------------------------------------------------------------------------
    error = rt_event_recv(gatt_event, GATT_EVENT_INDICATION | GATT_EVENT_TIMEOUT, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 200, &event);
    if(error != RT_EOK)
    {
        LOG_E("rt_event_recv() timeout or error");
        return -4;
    }
    
    if(event & GATT_EVENT_TIMEOUT)
    {
        LOG_E("wait indication confirm timeouted");
        return -5;
    }
    
    if(event & GATT_EVENT_INDICATION)
    {
        LOG_D("gatt indication confirmed");
        return 0;
    }

    return -6;
}
//***************************************************************************************************************************
//by yangwensen@20200731
static int8_t ble_send_packet(const ble_service_access_node_t *node, uint8_t *buff, uint8_t len)
{
    #define CONN_HANDLE         appConnHandle
    
//	ulog_hexdump("[MCU->]", 20, buff, len);

    if(Cy_BLE_GetConnectionState(CONN_HANDLE) < CY_BLE_CONN_STATE_CONNECTED)
    {
		LOG_E("ble disconnected!");
        return -1;
    }

    if( CY_BLE_IS_NOTIFICATION_SUPPORTED(node->attrHandle) && CY_BLE_IS_NOTIFICATION_ENABLED(CONN_HANDLE.attId, node->cccdHandle) )
	{
        LOG_D("ble_send_notification[%d]", len);
        if( ble_send_notification(CONN_HANDLE, node->attrHandle, buff, len) )
        {
            LOG_E("ble_send_notification fail");
		    return -2;
        }
        return len;
	}
    
    if( CY_BLE_IS_INDICATION_SUPPORTED(node->attrHandle) && CY_BLE_IS_INDICATION_ENABLED(CONN_HANDLE.attId, node->cccdHandle) ) 
	{
        LOG_D("ble_send_indication[%d]", len);
        if( ble_send_indication(CONN_HANDLE, node->attrHandle, buff, len) )
        {
            LOG_E("ble_send_indication fail");
		    return -3;
        }
        return len;
	}

    LOG_E("notification & notification not enabled");
    return 0;
}
//***************************************************************************************************************************
int8_t unionpay_send(uint8_t *buff, uint8_t len)
{
    return ble_send_packet(&ble_service_obj[BLE_UNIONPAY_TX_INDEX].config->access_node, buff, len);
}
//***************************************************************************************************************************
//by yangwensen@20200730
static rt_err_t cyble_configure(struct rt_ble_device *ble, struct ble_configure *cfg)
{
    struct ble_char_node *node;
    RT_ASSERT(ble != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    node = rt_container_of(ble, struct ble_char_node, ble);
    
	LOG_D("ble_config[%s]", node->config->name);
    
    return RT_EOK;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static rt_err_t cyble_control(struct rt_ble_device *ble, int cmd, void *arg)
{
    struct ble_char_node *node;

    RT_ASSERT(ble != RT_NULL);
    node = rt_container_of(ble, struct ble_char_node, ble);

	LOG_D("ble_control[%s]", node->config->name);
	
    switch (cmd)
    {
    /* disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:
        break;
    /* enable interrupt */
    case RT_DEVICE_CTRL_SET_INT:
        break;

    case RT_DEVICE_CTRL_CONFIG:
        break;
    }
    return RT_EOK;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static int cyble_putc(struct rt_ble_device *ble, char c)
{
    struct ble_char_node *node;
    int8_t res;
    RT_ASSERT(ble != RT_NULL);

    node = rt_container_of(ble, struct ble_char_node, ble);
	
    ble_reg.tx_buff[ble_reg.wr_index] = c;
    if( (++ble_reg.wr_index >= sizeof(ble_reg.tx_buff)) || (ble_reg.tx_len==ble_reg.wr_index) )
    {
        res = ble_send_packet(&node->config->access_node, ble_reg.tx_buff, ble_reg.wr_index);     //do transmit
        if(res <= 0)     
        {
            LOG_E("cyble send packet fail: %d", res);
            return 0;
        }
        ble_reg.tx_len -= ble_reg.wr_index;
        ble_reg.wr_index = 0;
    }
    
    return 1;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static int cyble_getc(struct rt_ble_device *ble)
{
    int ch;
    struct ble_char_node *node;
    RT_ASSERT(ble != RT_NULL);
    node = rt_container_of(ble, struct ble_char_node, ble);
    (void)node;
    
    ch = -1;

	if(ble_reg.rd_index < ble_reg.rx_len)
		ch = ble_reg.rx_buff[ble_reg.rd_index++];
    return ch;
}
//***************************************************************************************************************************
//by yangwensen@20200730
static const struct rt_ble_ops cy8c63_ble_ops =
{
    .configure = cyble_configure,
    .control = cyble_control,
    .putc = cyble_putc,
    .getc = cyble_getc,
};
//***************************************************************************************************************************
//by yangwensen@20200730
int rt_ble_service_init(void)
{
    rt_size_t obj_num = sizeof(ble_service_obj) / sizeof(struct ble_char_node);
    struct ble_configure config = RT_BLE_CONFIG_DEFAULT;
    rt_err_t result = 0;

    for (rt_size_t i = 0; i < obj_num; i++)
    {
        ble_service_obj[i].config = &BLE_SERVIE_CONFIG[i];
        ble_service_obj[i].ble.ops    = &cy8c63_ble_ops;
        ble_service_obj[i].ble.config = config;

		result = rt_hw_ble_register(&ble_service_obj[i].ble, ble_service_obj[i].config->name, ble_service_obj[i].config->rw_flag, NULL);
        RT_ASSERT(result == RT_EOK);
    }
    return result;
}
//***************************************************************************************************************************
