#ifndef __INFRARED_H
#define __INFRARED_H

#include "stm32f10x.h"

void Infrared_Init(void);
uint32_t Infrared_GetCount(void);
void Infrared_SetCount(uint32_t count);
void Infrared_ResetCount(void);

#endif
