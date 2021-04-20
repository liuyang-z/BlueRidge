
/* Include ************************************************************************/
#include "./sram.h"

#include "stm32f4xx.h"
#include "kernel/hal-driver/Inc/stm32f4xx_hal_rcc.h"
#include "kernel/hal-driver/Inc/stm32f4xx_hal_gpio.h"
#include "kernel/hal-driver/Inc/stm32f4xx_hal_sram.h"

/* Functions prototypes ***********************************************************/
void Error_Handler(char *file, uint32_t line);

/* Global defines *****************************************************************/
SRAM_HandleTypeDef hsram1;

/* Hw_Sram_Init ********************************************************************
*   IS61WV102416BLL SRAM 硬件初始化
*	使用 STM32F4 片上 FSMC 接口，SRAM 地址映射到 BANK3 区域
***********************************************************************************/
void Hw_Sram_Init() {
	FSMC_NORSRAM_TimingTypeDef Timing = {0};

	__HAL_RCC_FSMC_CLK_ENABLE();

	hsram1.Instance 				 = FSMC_NORSRAM_DEVICE;
	/* hsram1.Init */
	hsram1.Init.NSBank               = FSMC_NORSRAM_BANK3;
	hsram1.Init.DataAddressMux       = FSMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType           = FSMC_MEMORY_TYPE_SRAM;
	hsram1.Init.MemoryDataWidth      = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.BurstAccessMode      = FSMC_BURST_ACCESS_MODE_DISABLE;
	hsram1.Init.WaitSignalPolarity   = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WrapMode             = FSMC_WRAP_MODE_DISABLE;
	hsram1.Init.WaitSignalActive     = FSMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation       = FSMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal           = FSMC_WAIT_SIGNAL_DISABLE;
	hsram1.Init.ExtendedMode         = FSMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait     = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst           = FSMC_WRITE_BURST_DISABLE;
	hsram1.Init.PageSize             = FSMC_PAGE_SIZE_NONE;

	/* Timing */
	Timing.AddressSetupTime        = 0;
	Timing.AddressHoldTime         = 0;
	Timing.DataSetupTime           = 1;
	Timing.BusTurnAroundDuration   = 0;
	Timing.CLKDivision             = 0;
	Timing.DataLatency             = 0;
	Timing.AccessMode              = FSMC_ACCESS_MODE_A;

	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/* ll init, called by HAL_SRAM_Init() */
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram) {
	GPIO_InitTypeDef io;

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/
	io.Pin 			=  	GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  | GPIO_PIN_5  |
                       	GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 |
                    	GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	io.Mode 		= GPIO_MODE_AF_PP;
	io.Pull 		= GPIO_NOPULL;
	io.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	io.Alternate 	= GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOD, &io);


	io.Pin 			=  	GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3 |
                    	GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7 |
                    	GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11|
                    	GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	io.Mode 		= GPIO_MODE_AF_PP;
	io.Pull 		= GPIO_NOPULL;
	io.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	io.Alternate 	= GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOE, &io);


	io.Pin 			=  	GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  |
                    	GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_12 | GPIO_PIN_13 |
                    	GPIO_PIN_14 | GPIO_PIN_15;
	io.Mode 		= GPIO_MODE_AF_PP;
	io.Pull 		= GPIO_NOPULL;
	io.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	io.Alternate 	= GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOF, &io);


	io.Pin 			=   GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3 |
                    	GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_9  | GPIO_PIN_10;
	io.Mode 		= GPIO_MODE_AF_PP;
	io.Pull 		= GPIO_NOPULL;
	io.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	io.Alternate 	= GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOG, &io);
}

/* end of this file */
