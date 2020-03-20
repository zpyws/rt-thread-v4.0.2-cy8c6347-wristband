/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-06     balanceTWK        first version
 * 2019-04-23     WillianChan       Fix GPIO serial number disorder
 */

#include <board.h>
#include "drv_gpio.h"

#ifdef RT_USING_PIN

//by yangwensen@20200319
static const struct pin_index pins[] = 
{
#if defined(GPIO_PRT0)
    __CY8C_PIN(0 ,  0, 0 ),
    __CY8C_PIN(1 ,  0, 1 ),
    __CY8C_PIN(2 ,  0, 2 ),
    __CY8C_PIN(3 ,  0, 3 ),
    __CY8C_PIN(4 ,  0, 4 ),
    __CY8C_PIN(5 ,  0, 5 ),
    __CY8C_PIN(6 ,  0, 6 ),
    __CY8C_PIN(7 ,  0, 7 ),
#if defined(GPIO_PRT1)
    __CY8C_PIN(8 ,  1, 0 ),
    __CY8C_PIN(9 ,  1, 1 ),
    __CY8C_PIN(10,  1, 2 ),
    __CY8C_PIN(11,  1, 3 ),
    __CY8C_PIN(12,  1, 4 ),
    __CY8C_PIN(13,  1, 5 ),
    __CY8C_PIN(14,  1, 6 ),
    __CY8C_PIN(15,  1, 7 ),
#if defined(GPIO_PRT2)
    __CY8C_PIN(16,  2, 0 ),
    __CY8C_PIN(17,  2, 1 ),
    __CY8C_PIN(18,  2, 2 ),
    __CY8C_PIN(19,  2, 3 ),
    __CY8C_PIN(20,  2, 4 ),
    __CY8C_PIN(21,  2, 5 ),
    __CY8C_PIN(22,  2, 6 ),
    __CY8C_PIN(23,  2, 7 ),
#if defined(GPIO_PRT3)
    __CY8C_PIN(24,  3, 0 ),
    __CY8C_PIN(25,  3, 1 ),
    __CY8C_PIN(26,  3, 2 ),
    __CY8C_PIN(27,  3, 3 ),
    __CY8C_PIN(28,  3, 4 ),
    __CY8C_PIN(29,  3, 5 ),
    __CY8C_PIN(30,  3, 6 ),
    __CY8C_PIN(31,  3, 7 ),
#if defined(GPIO_PRT4)
    __CY8C_PIN(32,  4, 0 ),
    __CY8C_PIN(33,  4, 1 ),
    __CY8C_PIN(34,  4, 2 ),
    __CY8C_PIN(35,  4, 3 ),
    __CY8C_PIN(36,  4, 4 ),
    __CY8C_PIN(37,  4, 5 ),
    __CY8C_PIN(38,  4, 6 ),
    __CY8C_PIN(39,  4, 7 ),
#if defined(GPIO_PRT5)
    __CY8C_PIN(40,  5, 0 ),
    __CY8C_PIN(41,  5, 1 ),
    __CY8C_PIN(42,  5, 2 ),
    __CY8C_PIN(43,  5, 3 ),
    __CY8C_PIN(44,  5, 4 ),
    __CY8C_PIN(45,  5, 5 ),
    __CY8C_PIN(46,  5, 6 ),
    __CY8C_PIN(47,  5, 7 ),
#if defined(GPIO_PRT6)
    __CY8C_PIN(48,  6, 0 ),
    __CY8C_PIN(49,  6, 1 ),
    __CY8C_PIN(50,  6, 2 ),
    __CY8C_PIN(51,  6, 3 ),
    __CY8C_PIN(52,  6, 4 ),
    __CY8C_PIN(53,  6, 5 ),
    __CY8C_PIN(54,  6, 6 ),
    __CY8C_PIN(55,  6, 7 ),
#if defined(GPIO_PRT7)
    __CY8C_PIN(56,  7, 0 ),
    __CY8C_PIN(57,  7, 1 ),
    __CY8C_PIN(58,  7, 2 ),
    __CY8C_PIN(59,  7, 3 ),
    __CY8C_PIN(60,  7, 4 ),
    __CY8C_PIN(61,  7, 5 ),
    __CY8C_PIN(62,  7, 6 ),
    __CY8C_PIN(63,  7, 7 ),
#if defined(GPIO_PRT8)
    __CY8C_PIN(64,  8, 0 ),
    __CY8C_PIN(65,  8, 1 ),
    __CY8C_PIN(66,  8, 2 ),
    __CY8C_PIN(67,  8, 3 ),
    __CY8C_PIN(68,  8, 4 ),
    __CY8C_PIN(69,  8, 5 ),
    __CY8C_PIN(70,  8, 6 ),
    __CY8C_PIN(71,  8, 7 ),
#if defined(GPIO_PRT9)
    __CY8C_PIN(72,  9, 0 ),
    __CY8C_PIN(73,  9, 1 ),
    __CY8C_PIN(74,  9, 2 ),
    __CY8C_PIN(75,  9, 3 ),
    __CY8C_PIN(76,  9, 4 ),
    __CY8C_PIN(77,  9, 5 ),
    __CY8C_PIN(78,  9, 6 ),
    __CY8C_PIN(79,  9, 7 ),
#if defined(GPIO_PRT10)
    __CY8C_PIN(80, 10, 0 ),
    __CY8C_PIN(81, 10, 1 ),
    __CY8C_PIN(82, 10, 2 ),
    __CY8C_PIN(83, 10, 3 ),
    __CY8C_PIN(84, 10, 4 ),
    __CY8C_PIN(85, 10, 5 ),
    __CY8C_PIN(86, 10, 6 ),
    __CY8C_PIN(87, 10, 7 ),
#if defined(GPIO_PRT11)
    __CY8C_PIN(88, 11, 0 ),
    __CY8C_PIN(89, 11, 1 ),
    __CY8C_PIN(90, 11, 2 ),
    __CY8C_PIN(91, 11, 3 ),
    __CY8C_PIN(91, 11, 4 ),
    __CY8C_PIN(93, 11, 5 ),
    __CY8C_PIN(94, 11, 6 ),
    __CY8C_PIN(95, 11, 7 ),
#if defined(GPIO_PRT12)
    __CY8C_PIN(96, 12, 0 ),
    __CY8C_PIN(97, 12, 1 ),
    __CY8C_PIN(98, 12, 2 ),
    __CY8C_PIN(99, 12, 3 ),
    __CY8C_PIN(100, 12, 4 ),
    __CY8C_PIN(101, 12, 5 ),
    __CY8C_PIN(102, 12, 6 ),
    __CY8C_PIN(103, 12, 7 ),
#if defined(GPIO_PRT13)
    __CY8C_PIN(104, 13, 0 ),
    __CY8C_PIN(105, 13, 1 ),
    __CY8C_PIN(106, 13, 2 ),
    __CY8C_PIN(107, 13, 3 ),
    __CY8C_PIN(108, 13, 4 ),
    __CY8C_PIN(109, 13, 5 ),
    __CY8C_PIN(110, 13, 6 ),
    __CY8C_PIN(111, 13, 7 ),
#if defined(GPIO_PRT14)
    __CY8C_PIN(112, 14, 0 ),
    __CY8C_PIN(113, 14, 1 ),
    __CY8C_PIN(114, 14, 2 ),
    __CY8C_PIN(115, 14, 3 ),
    __CY8C_PIN(116, 14, 4 ),
    __CY8C_PIN(117, 14, 5 ),
    __CY8C_PIN(118, 14, 6 ),
    __CY8C_PIN(119, 14, 7 ),
#endif /* defined(GPIOK) */
#endif /* defined(GPIOJ) */
#endif /* defined(GPIOI) */
#endif /* defined(GPIOH) */
#endif /* defined(GPIOG) */
#endif /* defined(GPIOF) */
#endif /* defined(GPIOE) */
#endif /* defined(GPIOD) */
#endif /* defined(GPIOC) */
#endif /* defined(GPIOB) */
#endif /* defined(GPIOA) */
#endif /* defined(GPIO_PRT3) */
#endif /* defined(GPIO_PRT2) */
#endif /* defined(GPIO_PRT1) */
#endif /* defined(GPIO_PRT0) */
};

static const struct port_irq_map port_irq_map[] =
{
    __CY8C_PORT_INT_MAP__(0),
    __CY8C_PORT_INT_MAP__(1),
    __CY8C_PORT_INT_MAP__(2),
    __CY8C_PORT_INT_MAP__(3),
    __CY8C_PORT_INT_MAP__(4),
    __CY8C_PORT_INT_MAP__(5),
    __CY8C_PORT_INT_MAP__(6),
    __CY8C_PORT_INT_MAP__(7),
    __CY8C_PORT_INT_MAP__(8),
    __CY8C_PORT_INT_MAP__(9),
    __CY8C_PORT_INT_MAP__(10),
    __CY8C_PORT_INT_MAP__(11),
    __CY8C_PORT_INT_MAP__(12),
    __CY8C_PORT_INT_MAP__(13),
    __CY8C_PORT_INT_MAP__(14)
};

static struct rt_pin_irq_hdr pin_irq_hdr_tab[] =
{
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
};

//static uint32_t pin_irq_enable_mask=0;

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])
static const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
        if (index->index == -1)
            index = RT_NULL;
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};

//by yangwensen@20200319
static void cy8c_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    Cy_GPIO_Write(index->gpio, index->pin, (uint32_t)value);
}

//by yangwensen@20200319
static int cy8c_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }

    value = Cy_GPIO_Read(index->gpio, index->pin);

    return value;
}

//by yangwensen@20200319
static void cy8c_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        Cy_GPIO_SetDrivemode(index->gpio, index->pin, CY_GPIO_DM_STRONG_IN_OFF);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        //Digital High-Z. Input buffer on
        Cy_GPIO_SetDrivemode(index->gpio, index->pin, CY_GPIO_DM_HIGHZ);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        //Resistive Pull-Up. Input buffer on
        Cy_GPIO_SetDrivemode(index->gpio, index->pin, CY_GPIO_DM_PULLUP);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        //Resistive Pull-Down. Input buffer on
        Cy_GPIO_SetDrivemode(index->gpio, index->pin, CY_GPIO_DM_PULLDOWN);
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        //Open Drain, Drives High.
        Cy_GPIO_SetDrivemode(index->gpio, index->pin, CY_GPIO_DM_OD_DRIVESHIGH_IN_OFF);
    }
}

rt_inline rt_int32_t bit2bitno(rt_uint32_t bit)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if ((0x01 << i) == bit)
        {
            return i;
        }
    }
    return -1;
}

#if 0
rt_inline const struct pin_irq_map *get_pin_irq_map(uint32_t pinbit)
{
    rt_int32_t mapindex = bit2bitno(pinbit);
    if (mapindex < 0 || mapindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_NULL;
    }
    return &pin_irq_map[mapindex];
};
#endif

#if 0
static rt_err_t stm32_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    rt_base_t level;
    rt_int32_t irqindex = -1;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_ENOSYS;
    }

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == pin &&
            pin_irq_hdr_tab[irqindex].hdr == hdr &&
            pin_irq_hdr_tab[irqindex].mode == mode &&
            pin_irq_hdr_tab[irqindex].args == args)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EBUSY;
    }
    pin_irq_hdr_tab[irqindex].pin = pin;
    pin_irq_hdr_tab[irqindex].hdr = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}
#endif

#if 0
static rt_err_t stm32_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    const struct pin_index *index;
    rt_base_t level;
    rt_int32_t irqindex = -1;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_ENOSYS;
    }

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    pin_irq_hdr_tab[irqindex].pin = -1;
    pin_irq_hdr_tab[irqindex].hdr = RT_NULL;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = RT_NULL;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}
#endif

#if 0
static rt_err_t stm32_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled)
{
    const struct pin_index *index;
    const struct pin_irq_map *irqmap;
    rt_base_t level;
    rt_int32_t irqindex = -1;
//    GPIO_InitTypeDef GPIO_InitStruct;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = bit2bitno(index->pin);
        if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
        {
            return RT_ENOSYS;
        }

        level = rt_hw_interrupt_disable();

        if (pin_irq_hdr_tab[irqindex].pin == -1)
        {
            rt_hw_interrupt_enable(level);
            return RT_ENOSYS;
        }

        irqmap = &pin_irq_map[irqindex];
#if 0
        /* Configure GPIO_InitStructure */
        GPIO_InitStruct.Pin = index->pin;        
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        switch (pin_irq_hdr_tab[irqindex].mode)
        {
        case PIN_IRQ_MODE_RISING:
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
            break;
        case PIN_IRQ_MODE_FALLING:
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
            break;
        }
        HAL_GPIO_Init(index->gpio, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(irqmap->irqno, 5, 0);
        HAL_NVIC_EnableIRQ(irqmap->irqno);
        pin_irq_enable_mask |= irqmap->pinbit;
#endif

        rt_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = get_pin_irq_map(index->pin);
        if (irqmap == RT_NULL)
        {
            return RT_ENOSYS;
        }

        level = rt_hw_interrupt_disable();

        HAL_GPIO_DeInit(index->gpio, index->pin);

        pin_irq_enable_mask &= ~irqmap->pinbit;
#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32G0)
        if (( irqmap->pinbit>=GPIO_PIN_0 )&&( irqmap->pinbit<=GPIO_PIN_1 ))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_0|GPIO_PIN_1)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if (( irqmap->pinbit>=GPIO_PIN_2 )&&( irqmap->pinbit<=GPIO_PIN_3 ))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_2|GPIO_PIN_3)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if (( irqmap->pinbit>=GPIO_PIN_4 )&&( irqmap->pinbit<=GPIO_PIN_15 ))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|
                                      GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }    
        else
        {
            HAL_NVIC_DisableIRQ(irqmap->irqno);
        }         
#else      
    #if 0
        if (( irqmap->pinbit>=GPIO_PIN_5 )&&( irqmap->pinbit<=GPIO_PIN_9 ))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if (( irqmap->pinbit>=GPIO_PIN_10 )&&( irqmap->pinbit<=GPIO_PIN_15 ))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(irqmap->irqno);
        }        
    #endif
#endif          
        rt_hw_interrupt_enable(level);  
    }
    else
    {
        return -RT_ENOSYS;
    }

    return RT_EOK;
}
#endif

const static struct rt_pin_ops _cy8c_pin_ops =
{
    .pin_mode = cy8c_pin_mode,
    .pin_write = cy8c_pin_write,
    .pin_read = cy8c_pin_read,
    .pin_attach_irq = 0,
    .pin_detach_irq = 0,
    .pin_irq_enable = 0
};

rt_inline void pin_irq_hdr(int irqno)
{
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

#if defined(SOC_SERIES_STM32G0)
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}
#else
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}
#endif

#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32L0)
void EXTI0_1_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    rt_interrupt_leave();
}

void EXTI2_3_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    rt_interrupt_leave();
}
void EXTI4_15_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
    rt_interrupt_leave();
}

#else
    
#if 0
void EXTI0_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    rt_interrupt_leave();
}

void EXTI1_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    rt_interrupt_leave();
}

void EXTI2_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    rt_interrupt_leave();
}

void EXTI3_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    rt_interrupt_leave();
}

void EXTI4_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    rt_interrupt_leave();
}

void EXTI9_5_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    rt_interrupt_leave();
}

void EXTI15_10_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
    rt_interrupt_leave();
}
#endif
#endif

int rt_hw_pin_init(void)
{
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
    
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
    
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
    
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
    #ifdef SOC_SERIES_STM32L4
        HAL_PWREx_EnableVddIO2();
    #endif
    __HAL_RCC_GPIOG_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    __HAL_RCC_GPIOH_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    __HAL_RCC_GPIOI_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    __HAL_RCC_GPIOJ_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    __HAL_RCC_GPIOK_CLK_ENABLE();
#endif

    return rt_device_pin_register("pin", &_cy8c_pin_ops, RT_NULL);
}

#endif /* RT_USING_PIN */
