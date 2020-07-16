
#include "sensor.h"

#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_SECTION_NAME  "sensor.cypress.dietemp"
#define DBG_COLOR
#include <rtdbg.h>

extern int16_t cy8c_die_read_temp(void);

static rt_size_t cy8c_die_temp_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    struct rt_sensor_data *data = (struct rt_sensor_data *)buf;
    
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        data->type = RT_SENSOR_CLASS_TEMP;
        data->data.temp = cy8c_die_read_temp() * 10;
        data->timestamp = rt_sensor_get_ts();
    }
    
    return 1;
}

static rt_err_t cy8c_die_temp_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
		case RT_SENSOR_CTRL_SET_POWER:
			result =  -RT_EINVAL;
		break;
		
		case RT_SENSOR_CTRL_SELF_TEST:
			result =  -RT_EINVAL;
        break;
		
        case RT_SENSOR_CTRL_GET_ID:
			result =  -RT_EINVAL;
        break;
            
		default:
			result = -RT_ERROR;
		break;
    }
    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    cy8c_die_temp_fetch_data,
    cy8c_die_temp_control
};


//by yangwensen@20200716
int cy8c_die_temp_port(void)
{
	rt_int8_t result;
	rt_sensor_t sensor = RT_NULL;

    sensor = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor == RT_NULL)
        return -1;

    sensor->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
    sensor->info.model      = "cy8c_die_temp";
    sensor->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
    sensor->info.range_max  = 85;
    sensor->info.range_min  = -40;
    sensor->info.period_min = 100;

    sensor->ops = &sensor_ops;
    
    result = rt_hw_sensor_register(sensor, "mcu", RT_DEVICE_FLAG_RDWR, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor);
        return -RT_ERROR;
    }

    LOG_I("die temp sensor init success");
    return RT_EOK;
}
INIT_APP_EXPORT(cy8c_die_temp_port);
