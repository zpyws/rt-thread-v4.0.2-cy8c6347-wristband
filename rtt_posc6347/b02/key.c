#include <rtthread.h>
#include <board.h>
//created by yangwensen@20200326
//************************************************************************************************************
void home_key_isr(void)
{
    if (Cy_GPIO_GetInterruptStatus(HOME_KEY_PORT, HOME_KEY_NUM) )
    {
        Cy_GPIO_ClearInterrupt(HOME_KEY_PORT, HOME_KEY_NUM );
    }
    rt_kprintf("port7 int\n");
}
//************************************************************************************************************
void home_key_init(void)
{
    Cy_SysInt_Init(&SysInt_homekey_cfg, home_key_isr);
    NVIC_EnableIRQ(SysInt_homekey_cfg.intrSrc);
}
//************************************************************************************************************
