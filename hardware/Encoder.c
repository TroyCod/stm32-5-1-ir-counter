#include "Encoder.h"
#include "Delay.h"

static volatile int8_t Encoder_Direction = 0;  // 0=无旋转, 1=顺时针, -1=逆时针

/**
  * @brief  旋转编码器初始化
  * @param  无
  * @retval 无
  * @note   A相: PB1 (EXTI1中断检测)
  *         B相: PB11 (方向判断)
  *         按钮: PB10 (保留，可扩展)
  */
void Encoder_Init(void)
{
    /* 时钟配置 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* PB1(A相), PB11(B相), PB10(按钮) 配置为上拉输入 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 将PB1映射到EXTI1 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);

    /* EXTI1配置 (A相，下降沿触发) */
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    /* NVIC配置 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  获取编码器旋转方向
  * @param  无
  * @retval 1=顺时针, -1=逆时针, 0=无操作
  * @note   读取后自动清零
  */
int8_t Encoder_GetDirection(void)
{
    int8_t dir = Encoder_Direction;
    Encoder_Direction = 0;
    return dir;
}

/**
  * @brief  EXTI1中断服务函数 (编码器A相)
  * @param  无
  * @retval 无
  */
void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        Delay_ms(2);  // 消抖
        /* 读取B相电平判断旋转方向 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
        {
            Encoder_Direction = 1;   // 顺时针 (A相下降沿时B相为低)
        }
        else
        {
            Encoder_Direction = -1;  // 逆时针
        }
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
