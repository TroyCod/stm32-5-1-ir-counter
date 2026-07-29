#include "Infrared.h"

static volatile uint32_t Infrared_Count = 0;

/**
  * @brief  对射式红外传感器初始化
  * @param  无
  * @retval 无
  * @note   使用PA0作为红外接收引脚，EXTI0下降沿触发计数
  *         红外发射管常供电，接收管被遮挡时输出低电平
  */
void Infrared_Init(void)
{
    /* 时钟配置 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    /* PA0配置为上拉输入 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 将PA0映射到EXTI0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* EXTI0配置 */
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  // 遮挡时下降沿触发
    EXTI_Init(&EXTI_InitStructure);

    /* NVIC配置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  获取当前红外计数
  * @param  无
  * @retval 计数值
  */
uint32_t Infrared_GetCount(void)
{
    return Infrared_Count;
}

/**
  * @brief  设置红外计数
  * @param  count 新的计数值
  * @retval 无
  */
void Infrared_SetCount(uint32_t count)
{
    Infrared_Count = count;
}

/**
  * @brief  重置红外计数为0
  * @param  无
  * @retval 无
  */
void Infrared_ResetCount(void)
{
    Infrared_Count = 0;
}

/**
  * @brief  EXTI0中断服务函数
  * @param  无
  * @retval 无
  * @note   每遮挡一次红外对射管，计数+1
  */
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        Infrared_Count++;               // 计数+1
        EXTI_ClearITPendingBit(EXTI_Line0);  // 清除中断标志位
    }
}
