
#include "./include/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void OS_Init( void )
{
#if ( configUSE_CUSTUME_HEAP_TYPE == 5 )
        HeapRegion_t xHeapRegions[] =
        {
            { ( uint8_t * ) 0x10000000UL, 0x10000 },
            { NULL, 0 }
        };
        vPortDefineHeapRegions( xHeapRegions );
#endif
}

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
/*-----------------------------------------------------------*/

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
/*-----------------------------------------------------------*/

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
/*-----------------------------------------------------------*/

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
/*-----------------------------------------------------------*/

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

