
/* Include ************************************************************************/
#include "stm32f4xx.h"
#include "stm32f4xx_hal_tim.h"
#include "./hal_tick.h"


/* struct pro *********************************************************************/
typedef struct {
	TIM_HandleTypeDef 	handle;
	uint32_t 			priority;
	IRQn_Type			inter_type;

	int8_t				status;
} _hal_tick;


/* Global defines *****************************************************************/
_hal_tick _g_hal_tick = {

#define HAL_TICK_IRQHandler		TIM7_IRQHandler
#define HAL_TICK_CLK_ENABLE() 	__HAL_RCC_TIM7_CLK_ENABLE()

	.handle = {
		.Instance = TIM7,
	},
	.priority = 0,
	.inter_type = TIM7_IRQn,
	.status = 0
};


/* HAL_TICK_IRQHandler *************************************************************
*   TIM7 中断处理函数接口
***********************************************************************************/
void HAL_TICK_IRQHandler()
{
	HAL_TIM_IRQHandler(&_g_hal_tick.handle);
}

/* tim_period_elapse ***************************************************************
*   回调函数 called in irq handler
***********************************************************************************/
void tim_period_elapse(TIM_HandleTypeDef *htim)
{
	( void ) htim;
	HAL_IncTick();
}

/* tim_msp_init ********************************************************************
*   回调函数 called by HAL_TIM_Base_Init()
***********************************************************************************/
void tim_msp_init(TIM_HandleTypeDef *htim)
{
	HAL_TICK_CLK_ENABLE();

	HAL_NVIC_SetPriority(_g_hal_tick.inter_type, 1, _g_hal_tick.priority);
	HAL_NVIC_EnableIRQ(_g_hal_tick.inter_type);
}

/* HAL_InitTick ********************************************************************
*   强引用接口，用于初始化 Hal Tick 定时器，此接口需要初始化系统时钟后调用
***********************************************************************************/
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	uint32_t tim_clock = 0;
	uint32_t prescaler_value = 0;
	uint32_t period_value = 0;

	do {
		if(_g_hal_tick.status == 1)
			return (HAL_OK);

		_g_hal_tick.status = 1;
		break;

	} while(1);

	_g_hal_tick.priority = TickPriority;

	/*
		TIM6_CLK = 2 * PCLK1(max 42MHz)
		PCLK1 = HCLK(max 168MHz) / 4

		TIM6_CLK = 84MHz
	*/
	tim_clock = 2 * HAL_RCC_GetPCLK1Freq();
	prescaler_value = (tim_clock / 1000000U) - 1U;		/* have TIM7 counter clock equal to 1MHz */
	period_value = (1000000U / 1000U) - 1U;				/* have TIM7 Interrupt frequency to 1KHz */

	TIM_HandleTypeDef* hal_tick_tim = &_g_hal_tick.handle;

	hal_tick_tim->Init.Prescaler            = prescaler_value;
	hal_tick_tim->Init.Period               = period_value;
	hal_tick_tim->Init.ClockDivision        = TIM_CLOCKDIVISION_DIV1;
	hal_tick_tim->Init.CounterMode          = TIM_COUNTERMODE_UP;

	// 注意回调注册的顺序
	HAL_TIM_RegisterCallback(hal_tick_tim, HAL_TIM_BASE_MSPINIT_CB_ID, tim_msp_init);

	if(HAL_TIM_Base_Init(hal_tick_tim) != HAL_OK) {
		return HAL_ERROR;
	}

	// 中断相关的回调要在初始化之后进行
	HAL_TIM_RegisterCallback(hal_tick_tim, HAL_TIM_PERIOD_ELAPSED_CB_ID, tim_period_elapse);

	HAL_TIM_Base_Start_IT(hal_tick_tim);

	return HAL_OK;
}
