
#include "kernel/core/_log.h"
#include "driver/gpio/led.h"


#include "FreeRTOS.h"
#include "task.h"
#include "../driver/gpio/led.h"

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


void task_code(void *arg) {

    led_cfg();

    float a=2.1,b;

	a=2.3*3.1;
	b=a*12;
	a=b/3.3;


    for ( ;; ) {
        vTaskDelay(100);

        if(led_stat(0)) {
        led_act(0, 1);
        } else {
        led_act(0, 0);
        }
        LOGI("hello freertos...");
    }
}

void task_recv(void *arg) {
    for ( ;; ) {
        vTaskDelay(3000);
    }
}

int main(void) {

    TaskHandle_t task_c;
    TaskHandle_t task_v;

    HAL_Init();

    xTaskCreate(    task_code,
                    "task code",
                    140,
                    NULL,
                    2,
                    &task_c );

    xTaskCreate(    task_recv,
                    "task recv",
                    140,
                    NULL,
                    3,
                    &task_v );

    vTaskStartScheduler();

    for ( ;; );

    return 0;
}

/* end of this file. */
