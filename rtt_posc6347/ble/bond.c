/*******************************************************************************
* File Name: bond.c
*
* Version: 1.0
*
* Description:
*  This file contains bond list helper APIs.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <rtthread.h>

#define LOG_TAG                         "bond"
#define LOG_LVL                         LOG_LVL_DBG
#include <ulog.h>

#include <project.h>
//#include "common.h"
//#include "user_interface.h"

/* Static global variables */
//static bool removeBondListFlag = false;


/*******************************************************************************
* Function Name: App_DisplayBondList()
********************************************************************************
*
* Summary:
*  This function displays the bond list.
*
*******************************************************************************/
//by yangwensen@20200727
void App_DisplayBondList(void)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_MAX_BONDED_DEVICES];
    cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
    {
        .bdHandleAddrList = bondedDeviceInfo
    };
    uint8_t deviceCount;
    uint8_t i;
    uint8_t *p;
    
    /* Find out whether the device has bonded information stored already or not */
    apiResult = Cy_BLE_GAP_GetBondList(&bondedDeviceList);
    if(apiResult != CY_BLE_SUCCESS)
    {
        LOG_E("Cy_BLE_GAP_GetBondList API Error: 0x%x", apiResult);
        return;
    }
    
    deviceCount = bondedDeviceList.noOfDevices;

    
    LOG_D("Bond list: %d", deviceCount);
    
    for(i=0; i<deviceCount; i++)
    {
        if(bondedDeviceList.bdHandleAddrList[i].bdAddr.type == CY_BLE_GAP_ADDR_TYPE_RANDOM)
            rt_kprintf("Peer Random Address:");
        else
            rt_kprintf("Peer Public Address:");
            
        p = bondedDeviceList.bdHandleAddrList[i].bdAddr.bdAddr;
        rt_kprintf(" %02X-%02X-%02X-%02X-%02X-%02X", p[5],p[4],p[3],p[2],p[1],p[0]);
        rt_kprintf(", bdHandle: 0x%x \r\n", bondedDeviceList.bdHandleAddrList[i].bdHandle);
    }
}

MSH_CMD_EXPORT_ALIAS(App_DisplayBondList, ble_bondlist, show ble bond list);
/*******************************************************************************
* Function Name: App_RemoveDevicesFromBondList
********************************************************************************
*
* Summary:
*   Remove devices from the bond list.
*   
*******************************************************************************/
#if 0
void App_RemoveDevicesFromBondList(void)
{
#if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_bd_addr_t peerBdAddr = { .type = 0u };
    DBG_PRINTF("\r\nCleaning Bond List.....\r\n");  
    
    /* Remove all bonded devices in the list */
    apiResult = Cy_BLE_GAP_RemoveBondedDevice(&peerBdAddr);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_RemoveBondedDevice API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        DBG_PRINTF("Cy_BLE_GAP_RemoveBondedDevice done \r\n\r\n");
    }
#else
        DBG_PRINTF("\r\nBonding is disabled. Cleaning Bond List skipped.\r\n");  
#endif /* (CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES) */
    
    /* Clear flag  */
    removeBondListFlag = false;
}
#endif

/*******************************************************************************
* Function Name: App_GetCountOfBondedDevices()
********************************************************************************
*
* Summary:
*  This function returns the count of bonded devices
*
* Return:
*  uint32_t - The count of bonded devices
*
*******************************************************************************/
#if 0
uint32_t App_GetCountOfBondedDevices(void)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_MAX_BONDED_DEVICES];
    cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
    {
        .bdHandleAddrList = bondedDeviceInfo
    };
    uint32_t deviceCount = 0u;
    
    /* Find out whether the device has bonded information stored already or not */
    apiResult = Cy_BLE_GAP_GetBondList(&bondedDeviceList);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        deviceCount = bondedDeviceList.noOfDevices;
    }
    
    return (deviceCount);
}
#endif

/*******************************************************************************
* Function Name: App_IsDeviceInBondList()
********************************************************************************
*
* Summary:
*  This function check if device with bdHandle is in the bond list
*
* Parameters:
*   bdHandle - bonded device handle
*
* Return:
*   bool - true value when bdHandle exists in bond list
*
*******************************************************************************/
#if 0
bool App_IsDeviceInBondList(uint32_t bdHandle)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_MAX_BONDED_DEVICES];
    cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
    {
        .bdHandleAddrList = bondedDeviceInfo
    };
    bool deviceIsDetected = false;
    uint32_t deviceCount;
    
    /* Find out whether the device has bonding information stored already or not */
    apiResult = Cy_BLE_GAP_GetBondList(&bondedDeviceList);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        deviceCount = bondedDeviceList.noOfDevices;

        if(deviceCount != 0u)
        {
            do
            {
                deviceCount--;
                if(bdHandle == bondedDeviceList.bdHandleAddrList[deviceCount].bdHandle)
                {
                    deviceIsDetected = 1u;
                }
            } while(deviceCount != 0u);
        }
    }
    return(deviceIsDetected);
}
#endif

/*******************************************************************************
* Function Name: App_RemoveDevicesFromBondListBySW2Press()
********************************************************************************
* Summary:
*  Delete devices from the bond list if SW2 pressed during 4 (default) seconds 
*
* Parameters:
*   seconds - timeout to clear the bond list
*
*******************************************************************************/
#if 0
void App_RemoveDevicesFromBondListBySW2Press(uint32_t seconds)
{
    static uint32_t pressSw2Timer = 0u;
    uint32_t i;
    
    /* Press and hold the mechanical button (SW2) during 4 seconds to clear the bond list */
    if((SW2_Read() == 0u) && (pressSw2Timer == seconds))
    {
        /* User wants the bond to be removed */
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                /* Disconnect before delete devices from bond list */
                cy_stc_ble_gap_disconnect_info_t param =
                {
                    .bdHandle = cy_ble_connHandle[i].bdHandle,
                    .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER                          
                };
                Cy_BLE_GAP_Disconnect(&param); 
            }
        }
        
        /* Set flag to perform remove devices from the bond list */
        DBG_PRINTF("Triggered: removeBondListFlag\r\n");
        removeBondListFlag = true;
    }       
    pressSw2Timer = (SW2_Read() == 0u) ? (pressSw2Timer + 1u) : 0u;   /* Increment/reset pressSw2Timer */
}
#endif

/*******************************************************************************
* Function Name: App_SetRemoveBondListFlag()
********************************************************************************
* Summary:
*  Set flag for removing bond list
*
*******************************************************************************/
#if 0
void App_SetRemoveBondListFlag(void)
{
    removeBondListFlag = true;
}
#endif
/*******************************************************************************
* Function Name: App_IsRemoveBondListFlag()
********************************************************************************
* Summary:
*  Get value of remove bond list flag
*
* Return:
*   true  - remove bond list flag is set
*   false - remove bond list flag is clear
*
*******************************************************************************/
#if 0
bool App_IsRemoveBondListFlag(void)
{
    return removeBondListFlag;
}
#endif

/* [] END OF FILE */
