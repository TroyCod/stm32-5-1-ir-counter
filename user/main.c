#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Infrared.h"
#include "Encoder.h"

int main(void)
{
    uint32_t lastIrCount = 0;
    uint32_t displayCount = 0;
    int8_t encoderDir = 0;
    uint8_t pa0State = 0;

    OLED_Init();
    Infrared_Init();
    Encoder_Init();

    /* 开机画面 */
    OLED_ShowString(1, 1, "IR Counter V1.1");
    OLED_ShowString(2, 1, "PA0 debug mode");
    OLED_ShowString(3, 1, "Turn Pot to adj");
    OLED_ShowString(4, 1, "Sensor threshold");
    Delay_ms(2000);
    OLED_Clear();

    while (1)
    {
        uint32_t irCount = Infrared_GetCount();

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

        /* 同步红外计数 */
        displayCount += (irCount - lastIrCount);
        lastIrCount = irCount;

        /* PB10 清零 */
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

        /* 读PA0原始电平 */
        pa0State = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

        /* OLED显示 */
        OLED_ShowString(1, 1, "Count:");
        OLED_ShowNum(1, 8, displayCount, 7);

        /* PA0原始电平诊断行 */
        OLED_ShowString(2, 1, "PA0:");
        if (pa0State)
            OLED_ShowString(2, 5, "HIGH  ");
        else
            OLED_ShowString(2, 5, "LOW   ");

        /* IR中断计数 */
        OLED_ShowString(3, 1, "IR raw:");
        OLED_ShowNum(3, 9, irCount, 5);

        /* 遮挡状态 */
        OLED_ShowString(4, 1, "State:");
        if (pa0State)
            OLED_ShowString(4, 8, "Clear ");
        else
            OLED_ShowString(4, 8, "Block ");

        Delay_ms(50);
    }
}
