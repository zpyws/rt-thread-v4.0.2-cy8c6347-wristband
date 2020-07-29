//created by yangwensen@20200729
#include <rtthread.h>
#include <project.h>

#define LOG_TAG                         "GATT"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>
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
		cble_data_in(p->handleValPair.value.val, p->handleValPair.value.len);
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
