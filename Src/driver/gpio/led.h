
#ifndef __DRV_GPIO_LED_H_
#define __DRV_GPIO_LED_H_

#include "stm32f4xx.h"

void 	led_cfg();
void 	led_act(uint8_t led_idx, uint8_t status);
int8_t 	led_stat(uint8_t led_idx);

#endif /* __DRV_GPIO_LED_H_ */
