#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"

volatile uint32_t IR_Count = 0;      // 红外计数
volatile int8_t   Enc_Dir  = 0;      // 编码器方向 1/-1/0

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        IR_Count++;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
            Enc_Dir = 1;
        else
            Enc_Dir = -1;
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

int main(void)
{
    uint32_t display = 0, lastIR = 0;
    GPIO_InitTypeDef g;
    EXTI_InitTypeDef e;
    NVIC_InitTypeDef n;

    /* 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* PA0 红外DO — 上拉输入 */
    g.GPIO_Mode = GPIO_Mode_IPU;
    g.GPIO_Pin = GPIO_Pin_0;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &g);

    /* PB1编码器A相, PB11编码器B相 — 上拉输入 */
    g.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &g);

    /* EXTI0: PA0下降沿 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    e.EXTI_Line = EXTI_Line0;
    e.EXTI_Mode = EXTI_Mode_Interrupt;
    e.EXTI_Trigger = EXTI_Trigger_Falling;
    e.EXTI_LineCmd = ENABLE;
    EXTI_Init(&e);
    n.NVIC_IRQChannel = EXTI0_IRQn;
    n.NVIC_IRQChannelCmd = ENABLE;
    n.NVIC_IRQChannelPreemptionPriority = 2;
    n.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&n);

    /* EXTI1: PB1下降沿 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    e.EXTI_Line = EXTI_Line1;
    EXTI_Init(&e);
    n.NVIC_IRQChannel = EXTI1_IRQn;
    n.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&n);

    OLED_Init();
    OLED_ShowString(1,1,"IR Counter");
    OLED_ShowString(2,1,"Init done");
    Delay_ms(1000);
    OLED_Clear();

    while (1)
    {
        if (IR_Count != lastIR)
        {
            display += (IR_Count - lastIR);
            lastIR = IR_Count;
        }

        if (Enc_Dir == 1 && display < 9999999) { display++; Enc_Dir = 0; }
        if (Enc_Dir == -1 && display > 0)      { display--; Enc_Dir = 0; }

        OLED_ShowString(1, 1, "Count:");
        OLED_ShowNum(1, 8, display, 6);
        OLED_ShowString(3, 1, "Turn Encoder");

        Delay_ms(30);
    }
}
