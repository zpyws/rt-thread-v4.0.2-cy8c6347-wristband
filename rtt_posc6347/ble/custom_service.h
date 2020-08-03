#ifndef CUSTOM_SERVICE_H
#define CUSTOM_SERVICE_H
//**************************************************************************************************************
enum
{
    GATT_EVENT_INDICATION = 1,
    GATT_EVENT_TIMEOUT = 2,
};
//**************************************************************************************************************
//from btstack.c
extern rt_event_t gatt_event;
//**************************************************************************************************************
//from custom_servie.c
extern int8_t gatt_write_request(cy_stc_ble_gatt_write_param_t *p);
extern int rt_ble_service_init(void);
//**************************************************************************************************************
#endif
