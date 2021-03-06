//created by yangwensen@20200723
#include <rtthread.h>
#include <project.h>

#define LOG_TAG                         "CYBLE"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>

#include <stdlib.h>
#include "ancsc.h"
#include "custom_service.h"
//***************************************************************************************************************************
#define CY_BLE_CONN_INTRV_TO_MS      (5 / 4)
//***************************************************************************************************************************
typedef enum
{
    STACK_EV_DISPATCH = 1,
    STACK_EV_FLASH_WRITE_REQ = 2,
    STACK_EV_ANS = 4,
} STACK_EV_E;

rt_event_t stack_event;
rt_event_t gatt_event;

cy_stc_ble_conn_handle_t appConnHandle;
//***************************************************************************************************************************
//外部函数
extern void App_DisplayBondList(void);
extern bool App_IsDeviceInBondList(uint32_t bdHandle);
//from unionpay.c
extern int cble_init(void);
//***************************************************************************************************************************
void static BleControllerInterruptEventHandler(void)
{
    rt_event_send(stack_event, STACK_EV_DISPATCH);
}
//***************************************************************************************************************************
//by yangwensen@20200728
void ble_ansc_process_request(void)
{
    rt_event_send(stack_event, STACK_EV_ANS);
}
//***************************************************************************************************************************
//by yangwensen@20200723
void static StackEventHandler(uint32_t event, void *eventParam)
{
    uint16_t i;
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_auth_info_t *authInfo;
    static cy_stc_ble_gap_sec_key_info_t keyInfo =
    {
        .localKeysFlag    = CY_BLE_GAP_SMP_INIT_ENC_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_IRK_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_CSRK_KEY_DIST,
        .exchangeKeysFlag = CY_BLE_GAP_SMP_INIT_ENC_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_IRK_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_CSRK_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_ENC_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_IRK_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_CSRK_KEY_DIST,
    };
    uint8_t discIdx = Cy_BLE_GetDiscoveryIdx(appConnHandle);
    
    (void)eventParam;
    
    switch (event)
    {
        /**********************************************************
        *                       Generic and HCI Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event received when component is started */
            LOG_D("CY_BLE_EVT_STACK_ON");
            
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                LOG_E("Cy_BLE_GAPP_StartAdvertisement() Error: %d", apiResult);
            }

            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
                LOG_E("Cy_BLE_GAP_GenerateKeys() Error: 0x%x", apiResult);
            
            App_DisplayBondList();
            break;
            
        case CY_BLE_EVT_TIMEOUT:
            LOG_E("CY_BLE_EVT_TIMEOUT: 0x%x", (((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode));
            if( ((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GATT_RSP_TO )
            {
                LOG_E("connection handle: %d", ((cy_stc_ble_timeout_param_t *)eventParam)->CY_BLE_HANDLE.connHandle);
                rt_event_send(stack_event, GATT_EVENT_TIMEOUT);
            }
            break;
        
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred */
            LOG_E("CY_BLE_EVT_HARDWARE_ERROR");
            break;
            
        case CY_BLE_EVT_STACK_BUSY_STATUS:
            LOG_D("CY_BLE_EVT_STACK_BUSY_STATUS: %x", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            LOG_D("CY_BLE_EVT_SET_TX_PWR_COMPLETE");
            break;
            
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            LOG_D("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE");
            break;
            
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            LOG_D("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE");
            apiResult = Cy_BLE_GAP_GetBdAddress();
            if(apiResult != CY_BLE_SUCCESS)
            {   
                LOG_E("Cy_BLE_GAP_GetBdAddress() Error: 0x%x", apiResult);
            }
            break;

        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
        {
            uint8_t *addr;
            
            addr = ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->publicBdAddr;
            LOG_I("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: %02X-%02X-%02X-%02X-%02X-%02X", addr[5],addr[4],addr[3],addr[2],addr[1],addr[0]);
        }
        break;

        case CY_BLE_EVT_GET_RSSI_COMPLETE:
            if( ((cy_stc_ble_events_param_generic_t*)eventParam)->status == 0 ) 
                LOG_D("CY_BLE_EVT_GET_RSSI_COMPLETE: %d", ((cy_stc_ble_rssi_info_t*)((cy_stc_ble_events_param_generic_t*)eventParam)->eventParams)->rssi );
            else
                LOG_E("get rssi error");
            break;
        
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            LOG_D("CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE");
            break; 
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if( Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING )
                LOG_D("CY_BLE_ADV_STATE_ADVERTISING");
            else
                LOG_D("CY_BLE_ADV_STATE_STOPPED");
            break;

        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            LOG_D("CY_BLE_EVT_GAP_DEVICE_CONNECTED: %x, %x(%.2f ms), %x, %x",   
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->status,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv,
                (float)((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->supervisionTO);

            /* Set security keys for new device which is not already bonded */
            if(App_IsDeviceInBondList((*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle) == 0u)
            {
                keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                    LOG_E("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
            }
            ancsFlag = 0;
            break;
        
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            LOG_E("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED");
            
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                LOG_E("StartAdvertisement API Error: %d", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GAP_AUTH_REQ:
            LOG_D("CY_BLE_EVT_GAP_AUTH_REQ: security=0x%x, bonding=0x%x, ekeySize=0x%x, err=0x%x",
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).security,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr = CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }
            
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;
            
            apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
            if(apiResult != CY_BLE_SUCCESS)
            {
                LOG_E("Cy_BLE_GAPP_AuthReqReply() Error: %x, call Cy_BLE_GAP_RemoveOldestDeviceFromBondedList", apiResult);
                apiResult = Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                if(apiResult != CY_BLE_SUCCESS)
                    LOG_E("Cy_BLE_GAP_RemoveOldestDeviceFromBondedList API Error: %x", apiResult);
                else
                {
                    apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        LOG_E("Cy_BLE_GAPP_AuthReqReply API Error: %x", apiResult);
                    }
                }
            }
            break;
           
        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            LOG_D("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO: security:%x, bonding:%x, ekeySize:%x, authErr %x", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            authInfo = (cy_stc_ble_gap_auth_info_t *)eventParam;
            LOG_I("CY_BLE_EVT_GAP_AUTH_COMPLETE: security: 0x%x, bonding: 0x%x, ekeySize: 0x%x, authErr 0x%x \r\n",
                                    authInfo->security, authInfo->bonding, authInfo->ekeySize, authInfo->authErr);

            LOG_W("StartDiscovery");
            apiResult = Cy_BLE_GATTC_StartDiscovery(appConnHandle);
            if(apiResult != CY_BLE_SUCCESS)
                LOG_E("StartDiscovery API Error: 0x%x", apiResult);
            break;
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            LOG_D("CY_BLE_EVT_GAP_AUTH_FAILED: 0x%x", ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;
            
        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            LOG_W("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: encryption %s", 
                ((cy_stc_ble_gap_encrypt_change_param_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->encryption ? "on" : "off");
            break;
            
        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            LOG_D("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);

            apiResult = Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            if(apiResult != CY_BLE_SUCCESS)
            {
                LOG_E("Cy_BLE_GAP_SetIdAddress API Error: 0x%x", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            LOG_D("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT");
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATT_CONNECT_IND:
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            LOG_I("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x", appConnHandle.attId, appConnHandle.bdHandle);
            break;

        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            LOG_E("CY_BLE_EVT_GATT_DISCONNECT_IND:");
            appConnHandle.attId = 0u;
            appConnHandle.bdHandle = 0u;
            break;
        
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        {
            cy_stc_ble_gatt_xchg_mtu_param_t mtu;

            Cy_BLE_GATT_GetMtuSize(&mtu);
            LOG_D("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ, final mtu= %d", mtu.mtu);
            LOG_D("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ: GATT Client RX MTU = %d", ((cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam)->mtu);
        }
        break;

        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            LOG_D("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ, attHandle: %d", ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            LOG_D("CY_BLE_EVT_GATTS_INDICATION_ENABLED");
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
            LOG_D("CY_BLE_EVT_GATTS_INDICATION_DISABLED");
            break;
            
        case CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE:
            LOG_D("CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE");
            break;
            
        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            LOG_D("CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE");
            LOG_D("The discovered services:");
            for(i = 0u; i < CY_BLE_SRVI_COUNT; i++)
            {
                rt_kprintf("Service with UUID 0x%x has handle range from 0x%x to 0x%x\r\n",
                       cy_ble_serverInfo[discIdx][i].uuid,
                       cy_ble_serverInfo[discIdx][i].range.startHandle,
                       cy_ble_serverInfo[discIdx][i].range.endHandle);
                
                if(cy_ble_serverInfo[discIdx][i].uuid == 0x0000u)
                {
                    if(cy_ble_serverInfo[discIdx][i].range.startHandle < cy_ble_serverInfo[discIdx][i].range.endHandle)
                    {
                        LOG_I("The peer device supports ANS");
                        ancsFlag |= CY_BLE_ANCS_FLG_START;
                        ble_ansc_process_request();
                    }
                    else
                        LOG_W("The peer device doesn't support ANS");
                }
            }
        break;

        case CY_BLE_EVT_GATTS_WRITE_REQ:
            LOG_D("CY_BLE_EVT_GATTS_WRITE_REQ");
			gatt_write_request((cy_stc_ble_gatt_write_param_t *)eventParam);
            break;

        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            LOG_D("CY_BLE_EVT_GATTS_WRITE_CMD_REQ");
			break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            LOG_D("CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF");
            rt_event_send(gatt_event, GATT_EVENT_INDICATION);
            break;

    /**********************************************************
    *                       Other Events
    ***********************************************************/
        case CY_BLE_EVT_PENDING_FLASH_WRITE:
            LOG_D("CY_BLE_EVT_PENDING_FLASH_WRITE");
            rt_event_send(stack_event, STACK_EV_FLASH_WRITE_REQ);
            break;
            
        default:
            LOG_D("Other event: 0x%x", event);
    }
} 
//***************************************************************************************************************************
//by yangwensen@20200723
static int8_t ble_stack_init(void)
{
    cy_en_ble_api_result_t bleApiResult;
    cy_stc_ble_stack_lib_version_t stackVersion;
    
//-------start ble stack----------------------------------------------------------------------    
    bleApiResult = Cy_BLE_Start(StackEventHandler);
    if(bleApiResult != CY_BLE_SUCCESS)
    {
        LOG_E("Stack initialization Failure! Error Code:", bleApiResult);
        return -1;
    }
//-------打印蓝牙协议栈版本号-----------------------------------------------------------------    
    bleApiResult = Cy_BLE_GetStackLibraryVersion(&stackVersion);
    if(bleApiResult != CY_BLE_SUCCESS)
    {
        LOG_E("Cy_BLE_GetStackLibraryVersion() Error: 0x%2.2x", bleApiResult);
        return -2;
    }
    LOG_I("Stack Version: %d.%d.%d.%d", stackVersion.majorVersion, 
                                         stackVersion.minorVersion,
                                         stackVersion.patch,
                                         stackVersion.buildNumber);
//----------------------------------------------------------------------------------------------
    Cy_BLE_RegisterAppHostCallback(BleControllerInterruptEventHandler);
    Cy_BLE_ANCS_RegisterAttrCallback(AncsCallBack);

    Cy_BLE_ProcessEvents();
    
    return 0;
}
//***************************************************************************************************************************
//by yangwensen@20200724
static void ble_adv(uint8_t argc, char **argv)
{
    uint32_t action;
    cy_en_ble_api_result_t apiResult;
    cy_en_ble_adv_state_t state;
    
    if(argc != 2)
    {
        rt_kprintf("[Y]bad parameter! ble_adv (0|1)\n");
        return;
    }
    
    action = strtol(argv[1], NULL, 0);
    if(action > 1)
    {
        rt_kprintf("[Y]bad parameter! ble_adv (0|1)\n");
        return;
    }
    
    state = Cy_BLE_GetAdvertisementState();
    LOG_D("Cy_BLE_GetAdvertisementState()=%d", state);
    
    if( (action==0) && (state == CY_BLE_ADV_STATE_ADVERTISING) )
    {
        apiResult = Cy_BLE_GAPP_StopAdvertisement();
        if(apiResult != CY_BLE_SUCCESS)
            LOG_E("Cy_BLE_GAPP_StopAdvertisement() Error: %x", apiResult);
    }
    else if( (action==1) && (state == CY_BLE_ADV_STATE_STOPPED) )
    {
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
            LOG_E("Cy_BLE_GAPP_StartAdvertisement() Error: %x", apiResult);
    }
}
MSH_CMD_EXPORT(ble_adv, ble_advertisement[enable|disable]);
//***************************************************************************************************************************
//by yangwensen@20200728
static void ble_disconnect(void)
{
    cy_en_ble_api_result_t res;
    cy_stc_ble_gap_disconnect_info_t info;
    
    info.bdHandle = appConnHandle.bdHandle;
    info.reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER;
    
    res = Cy_BLE_GAP_Disconnect(&info);
    if(res!=CY_BLE_SUCCESS)
        LOG_E("Cy_BLE_GAP_Disconnect() Error: 0x%x", res);
}
MSH_CMD_EXPORT(ble_disconnect, terminate the LE connection with with peer device);
//***************************************************************************************************************************
//by yangwensen@20200728
static void ble_rssi(void)
{
    cy_en_ble_api_result_t res;
    
    res = Cy_BLE_GetRssiPeer(appConnHandle.bdHandle);
    if(res!=CY_BLE_SUCCESS)
        LOG_E("Cy_BLE_GetRssiPeer() Error: 0x%x", res);
}
MSH_CMD_EXPORT(ble_rssi, reads Received Signal Strength Indicator (RSSI) );
//***************************************************************************************************************************
//by yangwensen@20200723
static rt_bool_t _stack_init(void)
{
    stack_event = rt_event_create("stackev", RT_IPC_FLAG_FIFO);
    gatt_event = rt_event_create("gattev", RT_IPC_FLAG_FIFO);
    if (!stack_event || !gatt_event)
    {
        LOG_E("sem create failure");
        return RT_FALSE;
    }

	rt_ble_service_init();
	cble_init();

    ble_stack_init();

    return RT_TRUE;
}
//***************************************************************************************************************************
//by yangwensen@20200723
static void _softdevice_thread(void* parameter)
{
    for (;;)
    {
        rt_thread_mdelay(10);
    }
}
//***************************************************************************************************************************
//by yangwensen@20200723
static void task_ble(void *parameter)
{
    rt_uint32_t event = 0;
    
    for (;;)
    {
        rt_event_recv(stack_event, STACK_EV_FLASH_WRITE_REQ | STACK_EV_DISPATCH | STACK_EV_ANS,
                    RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &event);

        if (event & STACK_EV_DISPATCH)
        {
            Cy_BLE_ProcessEvents();
        }
        
        if(event & STACK_EV_FLASH_WRITE_REQ)
        {
            cy_en_ble_api_result_t apiResult;
            
        #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
            if(cy_ble_pendingFlashWrite != 0u) 
            {   
                apiResult = Cy_BLE_StoreBondingData();
                if(apiResult==CY_BLE_SUCCESS)
                    LOG_I("Store bonding data success");
                else if(apiResult==CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS)
                {
                    LOG_W("Store bonding data in process");
                    if(cy_ble_pendingFlashWrite)
                        rt_event_send(stack_event, STACK_EV_FLASH_WRITE_REQ);
                }
                else
                    LOG_E("Store bonding data, status: %x, pending: %x", apiResult, cy_ble_pendingFlashWrite);
            }
        #endif /* CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES */
        }
        
        if(event & STACK_EV_ANS)
        {
            AncsProcess();
        }
    }
}
//***************************************************************************************************************************
//by yangwensen@20200723
int ble_init(void)
{
    rt_thread_t thread;

    _stack_init();

    thread = rt_thread_create("softdevice", _softdevice_thread, RT_NULL, 512, 0, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        return RT_ERROR;
    }

    thread = rt_thread_create("ble", task_ble, RT_NULL, 2048, 1, 10);

    if (thread != RT_NULL)
    {
        return rt_thread_startup(thread);
    }

    return RT_ERROR;
}

INIT_ENV_EXPORT(ble_init);
//***************************************************************************************************************************
