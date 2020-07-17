#include <rtthread.h>
#include <board.h>
//created by yangwensen@20200629
//************************************************************************************************************
#define ADC_VBAT_EN_PIN         GET_PIN(10, 5)

#define ADC_DEV_NAME            "adc1"
#define ADC_DEV_CHANNEL         0
#define ADC_FULL_SCALE          (1ul<<12)
#define ADC_REF_VOL             1800

#define VBAT_UP_RESISTOR        15
#define VBAT_DOWN_RESISTOR      10
#define ADC_MUL_FACTOR          2               //samples averaged
//************************************************************************************************************
static int hw_battery_adc_init(void)
{
    rt_pin_mode(ADC_VBAT_EN_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(ADC_VBAT_EN_PIN, PIN_LOW);
    
    return RT_EOK;
}
INIT_BOARD_EXPORT(hw_battery_adc_init);
//************************************************************************************************************
//by yangwensen@20200717
static void show_battery_voltage(void)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t ad;
    uint32_t mv;
    
    rt_pin_write(ADC_VBAT_EN_PIN, PIN_HIGH);
    rt_thread_mdelay(5);
    
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);
    ad = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
    mv = ad * ADC_REF_VOL / ADC_FULL_SCALE;
    mv = mv * ADC_MUL_FACTOR * (VBAT_UP_RESISTOR + VBAT_DOWN_RESISTOR) / VBAT_DOWN_RESISTOR;
    rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);
    
    rt_pin_write(ADC_VBAT_EN_PIN, PIN_LOW);
    rt_kprintf("battery voltage: %dmV, adc=%d\n", mv, ad);
}
MSH_CMD_EXPORT_ALIAS(show_battery_voltage, vbat, show battery voltage)
//************************************************************************************************************
#define SYS_POWER_EN_PIN         GET_PIN(5, 4)

static int hw_sys_power_en(void)
{
    rt_pin_mode(SYS_POWER_EN_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SYS_POWER_EN_PIN, PIN_HIGH);
    
    return RT_EOK;
}
INIT_BOARD_EXPORT(hw_sys_power_en);
//************************************************************************************************************
