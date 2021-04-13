
#include "led.h"
#include "kernel/hal-driver/Inc/stm32f4xx_hal_rcc.h"
#include "kernel/hal-driver/Inc/stm32f4xx_hal_gpio.h"

typedef struct {
	GPIO_InitTypeDef mio;
	GPIO_TypeDef *port;
} led_t;

#define G_LED_MAX_CNT	4
static led_t _G_LEDS[G_LED_MAX_CNT] = {
	{
		.port = GPIOF,
		.mio = {
			.Mode    = GPIO_MODE_OUTPUT_PP,
			.Pull    = GPIO_PULLDOWN,
			.Speed   = GPIO_SPEED_HIGH,
			.Pin     = GPIO_PIN_10
		}
	},
	{
		.port = GPIOC,
		.mio = {
			.Mode    = GPIO_MODE_OUTPUT_PP,
			.Pull    = GPIO_PULLDOWN,
			.Speed   = GPIO_SPEED_HIGH,
			.Pin     = GPIO_PIN_0
		}
	},
	{
		.port = GPIOC,
		.mio = {
			.Mode    = GPIO_MODE_OUTPUT_PP,
			.Pull    = GPIO_PULLDOWN,
			.Speed   = GPIO_SPEED_HIGH,
			.Pin     = GPIO_PIN_2
		}
	},
	{
		.port = GPIOC,
		.mio = {
			.Mode    = GPIO_MODE_OUTPUT_PP,
			.Pull    = GPIO_PULLDOWN,
			.Speed   = GPIO_SPEED_HIGH,
			.Pin     = GPIO_PIN_3
		}
	}
};

static void _hal_io_cfg(led_t* led) {
	HAL_GPIO_Init(led->port, &led->mio);
}

static void _hal_io_clk_cfg() {
	/* enable the gpio clk */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
}

void led_cfg() {
	_hal_io_clk_cfg();

	int idx = 0;
	for(idx = 0; idx < G_LED_MAX_CNT; idx++) {
		_hal_io_cfg(&_G_LEDS[idx]);
	}
}

void led_act(uint8_t led_idx, uint8_t status) {
	if(led_idx >= G_LED_MAX_CNT)
		return;

	if(status < 0)
		return;

	led_t* led = &_G_LEDS[led_idx];
	if(status) {
		HAL_GPIO_WritePin(led->port, led->mio.Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(led->port, led->mio.Pin, GPIO_PIN_SET);
	}
}

int8_t led_stat(uint8_t led_idx) {
	if(led_idx >= G_LED_MAX_CNT)
		return -1;

	led_t* led = &_G_LEDS[led_idx];
	return HAL_GPIO_ReadPin(led->port, led->mio.Pin);
}

/* end of this file */
