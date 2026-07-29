#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"

int main(void)
{
    uint32_t counter = 0;
    uint8_t pa0 = 0;
    uint8_t last_pa0 = 0;

    /* 必须开头就使能 GPIOA 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* PA0 配置为上拉输入 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    OLED_Init();
    OLED_Clear();

    while (1)
    {
        pa0 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
        if (pa0 != last_pa0)
        {
            counter++;
            last_pa0 = pa0;
        }

        OLED_ShowString(1, 1, "PA0 raw:");
        OLED_ShowNum(1, 10, pa0, 1);
        if (pa0)  OLED_ShowString(1, 12, "HIGH ");
        else      OLED_ShowString(1, 12, "LOW  ");

        OLED_ShowString(2, 1, "Change cnt:");
        OLED_ShowNum(2, 13, counter, 3);

        if (!pa0)
        {
            OLED_ShowString(3, 1, ">>> BLOCKED <<<");
        }
        else
        {
            OLED_ShowString(3, 1, "    Open OK    ");
        }

        OLED_ShowString(4, 1, "Tick:");
        OLED_ShowNum(4, 7, counter + (pa0 * 1000), 5);

        Delay_ms(100);
    }
}
