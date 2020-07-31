//created by yangwensen@20200729
#include <rtthread.h>
#include <project.h>
#include <rtdevice.h>
#include "ble_dev.h"

#define LOG_TAG                         "GATT"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>
//***************************************************************************************************************************
enum
{
	BLE_UNIONPAY_TX_INDEX,
	BLE_UNIONPAY_RX_INDEX,
};

typedef struct
{
	cy_stc_ble_conn_handle_t connHandle;
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
		RT_DEVICE_FLAG_WRONLY|RT_DEVICE_FLAG_INT_TX, 
		{RT_NULL,CY_BLE_UNIONPAY_OUT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE,CY_BLE_UNIONPAY_OUT_CHAR_HANDLE}
	},
	{
		"ble_up_rx", 
		RT_DEVICE_FLAG_RDONLY|RT_DEVICE_FLAG_INT_RX, 
		{RT_NULL,RT_NULL,CY_BLE_UNIONPAY_IN_CHAR_HANDLE}
	},
};

static struct ble_char_node ble_service_obj[sizeof(BLE_SERVIE_CONFIG) / sizeof(BLE_SERVIE_CONFIG[0])] = {0};

//用于模拟蓝牙硬件寄存器
static struct
{
	uint8_t rx_len;
	uint8_t rd_index;
	uint8_t rx_buff[20];
}ble_reg
 = {0};
//***************************************************************************************************************************
//function prototype
//from unionpay.c
extern int8_t cble_data_in(uint8_t *data, uint8_t size);
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
	
	if(p->handleValPair.attrHandle == CY_BLE_UNIONPAY_IN_CHAR_HANDLE)
	{
//		LOG_D("Data in %d bytes", p->handleValPair.value.len);
//		cble_data_in(p->handleValPair.value.val, p->handleValPair.value.len);
        ble_reg.rd_index = 0;
        ble_reg.rx_len = p->handleValPair.value.len;
        rt_memcpy(ble_reg.rx_buff, p->handleValPair.value.val, ble_reg.rx_len);
        rt_hw_ble_isr(&(ble_service_obj[BLE_UNIONPAY_RX_INDEX].ble), RT_BLE_EVENT_RX_IND);
	}
	else if(p->handleValPair.attrHandle == CY_BLE_UNIONPAY_OUT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
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
int8_t ble_send_notification(cy_stc_ble_conn_handle_t connHandle, cy_ble_gatt_db_attr_handle_t cccdHandle, cy_ble_gatt_db_attr_handle_t attrHandle, uint8_t *buff, uint32_t len)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
		LOG_E("ble disconnected!");
        return -1;
    }
	
    if(CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cccdHandle))
	{
		LOG_E("notification not enabled");
		return -2;
	}
	
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
int8_t ble_send_indication(cy_stc_ble_conn_handle_t connHandle, cy_ble_gatt_db_attr_handle_t cccdHandle, cy_ble_gatt_db_attr_handle_t attrHandle, uint8_t *buff, uint32_t len)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
		LOG_E("ble disconnected!");
        return -1;
    }
	
    if(CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cccdHandle))
	{
		LOG_E("notification not enabled");
		return -2;
	}
	
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

	return 0;
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
    RT_ASSERT(ble != RT_NULL);

    node = rt_container_of(ble, struct ble_char_node, ble);
	
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
