
#include "kernel/core/_log.h"
#include "driver/gpio/led.h"


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType        = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState              = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState          = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource         = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM              = 25;
  RCC_OscInitStruct.PLL.PLLN              = 336;
  RCC_OscInitStruct.PLL.PLLP              = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ              = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

#include "FreeRTOS.h"
#include "task.h"
#include "../driver/gpio/led.h"

void task_code(void *arg) {

  led_cfg();

  float tmath = 0.1;

  for(;;) {
    vTaskDelay(300);

    tmath += 1.1;

    if(led_stat(0)) {
      led_act(0, 1);
    } else {
      led_act(0, 0);
    }
    LOGI("hello freertos...");
  }
}

void task_recv(void *arg) {
  for(;;) {


    vTaskDelay(100);
  }
}

int main(void) {

  TaskHandle_t task_c;
  TaskHandle_t task_v;

	HAL_Init();
	SystemClock_Config();

  xTaskCreate(
    task_code,
    "task code",
    140,
    NULL,
    2,
    &task_c
  );

  xTaskCreate(
    task_recv,
    "task recv",
    140,
    NULL,
    3,
    &task_v
  );

  vTaskStartScheduler();

  for(;;) {

  }

	return 0;
}

/* end of this file. */
