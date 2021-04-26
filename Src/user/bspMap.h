
#ifndef __USER_BSPMAP_H_
#define __USER_BSPMAP_H_

/* Include ************************************************************************/
#include "FreeRTOS.h"

/* Global Defines *****************************************************************/
#if ( configUSE_CUSTUME_HEAP_TYPE == 5 )
	HeapRegion_t xHeapRegions[] =
	{
		{ ( uint8_t * ) 0x10000000UL, 0x010000 },		/* CCRAM ( 64KB ) 			*/

#if !defined ( VECT_TAB_SRAM )
		// 在 RAM 中调试程序时，则 RAM 不能加入到动态内存分配
		// RAM-Internal(28KB + 100KB = 128KB)
		// 其中前 28KB 用于 bss 区域，后 100KB 用于动态内存分配
		{ ( uint8_t * ) 0x20007000UL, 0x019000 },		/* RAM-Internal ( 100KB )	*/
#endif

		{ ( uint8_t * ) 0x68000000UL, 0x200000 },		/* SRAM ( 2MB)				*/
		{ NULL, 0 }
	};
#endif

#endif /* __USER_BSPMAP_H_ */
