#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

void Encoder_Init(void);
int8_t Encoder_GetDirection(void);

#endif
