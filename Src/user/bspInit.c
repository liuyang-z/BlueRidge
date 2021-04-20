
/* Include ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "bspMap.h"
#include "FreeRTOS_POSIX/pthread.h"

#include "user_main.h"
#include "driver/fsmc/sram.h"
#include "driver/gpio/led.h"

/* Functions prototypes ***********************************************************/
void Error_Handler(char *file, uint32_t line);

/* SystemClock_Config **************************************************************
*   配置系统时钟，
*	包括 PLL (M N P Q), AHB, APB1, APB2
***********************************************************************************/
static void SystemClock_Config(void)
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
    Error_Handler(__FILE__, __LINE__);
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
    Error_Handler(__FILE__, __LINE__);
  }
}

/* HAL_MspInit *********************************************************************
*   初始化 BSP 硬件外设，弱引用调用(called by HAL_Init)
*	包括系统时钟、SRAM、以及其他硬件外设驱动
***********************************************************************************/
void HAL_MspInit(void)
{

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    SystemClock_Config();
    /* System interrupt init*/

    // init sram
    Hw_Sram_Init();

    /* init led (gpio) */
    led_cfg();
}

/* OS_Init *************************************************************************
*   操作系统初始化，主要包括 Heap 的初始化
***********************************************************************************/
void OS_Init( void )
{
#if ( configUSE_CUSTUME_HEAP_TYPE == 5 )
	vPortDefineHeapRegions( xHeapRegions );
#endif
}

/* pthread_portal ******************************************************************
*   操作系统初始化入口
* 	此入口用于将 FreeRTOS 接口对用户隐藏
*	进入 user_main() 之后的用户多线程、互斥信号量、队列等操作使用 POSIX 接口即可。
***********************************************************************************/
void pthread_portal(void *arg) {
    pthread_t thread_main;

    pthread_create(&thread_main, NULL, user_main, NULL);

    pthread_join(thread_main, NULL);

    vTaskDelete(NULL);
}

/* main ****************************************************************************
*   系统上电初始化 BSS 节区之后，便进入用户此用户入口
***********************************************************************************/
int main(void) {

	/* initial chip clock */
    HAL_Init();

	/* initial os */
    OS_Init();

    xTaskCreate(pthread_portal, "pthread_portal", 160, NULL, 4, NULL);

    vTaskStartScheduler();

	/* never rech this line */
    for( ;; );

    return 0;
}

/* Error_Handler *******************************************************************
*   错误处理回调，通过 _weak 调用此函数
*   in: file            错误所在文件，由 __FILE__ 指定
*   in: line            错误所处的行号，由 __LINE__ 指定
***********************************************************************************/
void Error_Handler(char *file, uint32_t line)
{
  ( void ) file;
  ( void ) line;

  taskDISABLE_INTERRUPTS();
  for( ;; );
}

/* assert_failed *******************************************************************
*   STM32 硬件抽象层断言错误处理接口
***********************************************************************************/
#ifdef  USE_FULL_ASSERT
  void assert_failed(uint8_t *file, uint32_t line)
  {
    ( void ) file;
    ( void ) line;
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    taskDISABLE_INTERRUPTS();
		for( ;; );
  }
#endif

/* vApplicationMallocFailedHook ****************************************************
*   FreeRTOS 系统接口，用于系统动态内存分配错误后的回调处理
***********************************************************************************/
#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
	void vApplicationMallocFailedHook( void )
	{
		/* vApplicationMallocFailedHook() will only be called if
		configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
		function that will get called if a call to pvPortMalloc() fails.
		pvPortMalloc() is called internally by the kernel whenever a task, queue,
		timer or semaphore is created.  It is also called by various parts of the
		demo application.  If heap_1.c or heap_2.c are used, then the size of the
		heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
		FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
		to query the size of free heap space that remains (although it does not
		provide information on how the remaining heap might be fragmented). */
		taskDISABLE_INTERRUPTS();
		for( ;; );
	}
#endif

/* vApplicationIdleHook ************************************************************
*   FreeRTOS 系统接口，用于 IDLE 进程的用户回调处理
***********************************************************************************/
#if ( configUSE_IDLE_HOOK == 1 )
	void vApplicationIdleHook( void )
	{
		/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
		to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
		task.  It is essential that code added to this hook function never attempts
		to block in any way (for example, call xQueueReceive() with a block time
		specified, or call vTaskDelay()).  If the application makes use of the
		vTaskDelete() API function (as this demo application does) then it is also
		important that vApplicationIdleHook() is permitted to return to its calling
		function, because it is the responsibility of the idle task to clean up
		memory allocated by the kernel to any task that has since been deleted. */
	}
#endif

/* vApplicationStackOverflowHook ***************************************************
*   FreeRTOS 系统接口，用于线程栈出现栈溢出后的回调处理
***********************************************************************************/
#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
	void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
	{
		( void ) pcTaskName;
		( void ) pxTask;

		/* Run time stack overflow checking is performed if
		configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
		function is called if a stack overflow is detected. */
		taskDISABLE_INTERRUPTS();
		for( ;; );
	}
#endif

/* vApplicationTickHook ************************************************************
*   FreeRTOS 系统接口，用于 Systick 中断时的用户回调处理
***********************************************************************************/
#if  (  configUSE_TICK_HOOK > 0 )
	void vApplicationTickHook( void )
	{
		/* This function will be called by each tick interrupt if
		configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
		added here, but the tick hook is called from an interrupt context, so
		code must not attempt to block, and only the interrupt safe FreeRTOS API
		functions can be used (those that end in FromISR()). */
	}
#endif

/* vApplicationGetIdleTaskMemory ***************************************************
*   FreeRTOS 系统接口，用于静态分配时所调用的 IDLE 栈内存分配
***********************************************************************************/
#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
    /**
     * task.h
     * <pre>void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize ) </pre>
     *
     * This function is used to provide a statically allocated block of memory to FreeRTOS to hold the Idle Task TCB.  This function is required when
     * configSUPPORT_STATIC_ALLOCATION is set.  For more information see this URI: https://www.FreeRTOS.org/a00110.html#configSUPPORT_STATIC_ALLOCATION
     *
     * @param ppxIdleTaskTCBBuffer A handle to a statically allocated TCB buffer
     * @param ppxIdleTaskStackBuffer A handle to a statically allocated Stack buffer for thie idle task
     * @param pulIdleTaskStackSize A pointer to the number of elements that will fit in the allocated stack buffer
     */
    void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
										StackType_t ** ppxIdleTaskStackBuffer,
										uint32_t * pulIdleTaskStackSize ) {
		static StaticTask_t idle_task_tcb;
		static StackType_t	idle_statck_buffer[configMINIMAL_STACK_SIZE];

		*ppxIdleTaskTCBBuffer   = &idle_task_tcb;
		*ppxIdleTaskStackBuffer = idle_statck_buffer;
		*pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
	}
#endif

/* vApplicationGetTimerTaskMemory **************************************************
*   FreeRTOS 系统接口，用于静态分配时所调用的 TIMER 栈内存分配
***********************************************************************************/
#if ( (configSUPPORT_STATIC_ALLOCATION == 1) && (configUSE_TIMERS == 1) )

    /**
     * task.h
     * <pre>void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize ) </pre>
     *
     * This function is used to provide a statically allocated block of memory to FreeRTOS to hold the Timer Task TCB.  This function is required when
     * configSUPPORT_STATIC_ALLOCATION is set.  For more information see this URI: https://www.FreeRTOS.org/a00110.html#configSUPPORT_STATIC_ALLOCATION
     *
     * @param ppxTimerTaskTCBBuffer   A handle to a statically allocated TCB buffer
     * @param ppxTimerTaskStackBuffer A handle to a statically allocated Stack buffer for thie idle task
     * @param pulTimerTaskStackSize   A pointer to the number of elements that will fit in the allocated stack buffer
     */
    void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                         StackType_t ** ppxTimerTaskStackBuffer,
                                         uint32_t * pulTimerTaskStackSize ) {
		static StaticTask_t tmr_task_tcb;
		static StackType_t	tmr_statck_buffer[configMINIMAL_STACK_SIZE];

		*ppxTimerTaskTCBBuffer   = &tmr_task_tcb;
		*ppxTimerTaskStackBuffer = tmr_statck_buffer;
		*pulTimerTaskStackSize   = configMINIMAL_STACK_SIZE;
	}

#endif

