#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"

static volatile uint32_t g_Count = 0;      // 红外中断计数
static volatile int8_t   g_EncDir = 0;     // 编码器方向: 1/-1/0

/* ============ EXTI0: 红外DO ============ */
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        g_Count++;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/* ============ EXTI1: 编码器A相 ============ */
void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
            g_EncDir = 1;   // 顺时针
        else
            g_EncDir = -1;  // 逆时针
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

/* ============ 初始化 ============ */
static void HW_Init(void)
{
    GPIO_InitTypeDef  gpio;
    EXTI_InitTypeDef  exti;
    NVIC_InitTypeDef  nvic;

    /* 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO, ENABLE);

    /* PA0: 上拉输入 (红外DO) */
    gpio.GPIO_Mode  = GPIO_Mode_IPU;
    gpio.GPIO_Pin   = GPIO_Pin_0;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PB1(A相), PB11(B相): 上拉输入 */
    gpio.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &gpio);

    /* 中断分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* --- EXTI0: PA0 任意边沿（无论高低都能计数） --- */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    exti.EXTI_Line    = EXTI_Line0;
    exti.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 双边沿
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    nvic.NVIC_IRQChannel    = EXTI0_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority       = 0;
    NVIC_Init(&nvic);

    /* --- EXTI1: PB1 下降沿 --- */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    exti.EXTI_Line    = EXTI_Line1;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&exti);

    nvic.NVIC_IRQChannel    = EXTI1_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority       = 1;
    NVIC_Init(&nvic);
}

/* ============ 主程序 ============ */
int main(void)
{
    uint32_t disp      = 0;      // 显示值
    uint32_t lastCnt   = 0;      // 上次的中断计数
    uint8_t  pa0       = 0;
    int8_t   enc       = 0;

    OLED_Init();
    HW_Init();
    OLED_Clear();

    while (1)
    {
        /* 红外计数增量 */
        if (g_Count != lastCnt)
        {
            disp += (g_Count - lastCnt);
            lastCnt = g_Count;
        }

        /* 编码器调节 */
        enc = g_EncDir;
        g_EncDir = 0;
        if (enc == 1 && disp < 9999999)  disp++;
        if (enc == -1 && disp > 0)       disp--;

        /* 读PA0 */
        pa0 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

        /* 显示 */
        OLED_ShowString(1, 1, "COUNT:");
        OLED_ShowNum(1, 8, disp, 7);

        OLED_ShowString(2, 1, "PA0:");
        if (pa0) OLED_ShowString(2, 6, "HIGH  ");
        else     OLED_ShowString(2, 6, "LOW   ");

        OLED_ShowString(3, 1, "IR:");
        OLED_ShowNum(3, 5, g_Count, 5);

        OLED_ShowString(4, 1, "ENC:");
        if (enc == 1)      OLED_ShowString(4, 5, "+");
        else if (enc == -1) OLED_ShowString(4, 5, "-");
        else               OLED_ShowString(4, 5, " ");

        Delay_ms(50);
    }
}
