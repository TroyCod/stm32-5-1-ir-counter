#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Infrared.h"
#include "Encoder.h"

int main(void)
{
    uint32_t lastIrCount = 0;    // 上一轮的红外计数值
    uint32_t displayCount = 0;   // 显示用计数值
    int8_t encoderDir = 0;

    OLED_Init();
    Infrared_Init();
    Encoder_Init();

    /* 开机画面 */
    OLED_ShowString(1, 1, "IR Counter V1.0");
    OLED_ShowString(2, 1, "STM32F103C8T6");
    OLED_ShowString(3, 1, "Encoder: Adjust");
    OLED_ShowString(4, 1, "PB10: Reset");
    Delay_ms(1500);
    OLED_Clear();

    while (1)
    {
        uint32_t irCount = Infrared_GetCount();   // 从ISR获取红外计数

        /* 红外计数增量（自动累加） */
        if (irCount != lastIrCount)
        {
            displayCount += (irCount - lastIrCount);
            lastIrCount = irCount;
        }

        /* 编码器调整显示值 */
        encoderDir = Encoder_GetDirection();
        if (encoderDir == 1 && displayCount < 9999999)
        {
            displayCount++;
        }
        else if (encoderDir == -1 && displayCount > 0)
        {
            displayCount--;
        }

        /* PB10按下：清零复位 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
        {
            Delay_ms(20);
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
            {
                displayCount = 0;
                lastIrCount = 0;
                Infrared_SetCount(0);
                OLED_Clear();
                while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0);
                Delay_ms(20);
            }
        }

        /* OLED显示刷新 */
        OLED_ShowString(1, 1, "IR Counter");
        OLED_ShowString(2, 1, "Count:");
        OLED_ShowNum(2, 8, displayCount, 7);

        /* 红外状态指示 */
        OLED_ShowString(4, 1, "State:");
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
            OLED_ShowString(4, 8, "Blocked ");   // 被遮挡
        else
            OLED_ShowString(4, 8, "Clear   ");   // 畅通

        Delay_ms(50);
    }
}
