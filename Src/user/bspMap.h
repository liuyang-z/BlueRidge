
#ifndef __USER_BSPMAP_H_
#define __USER_BSPMAP_H_

/* Include ************************************************************************/
#include "FreeRTOS.h"

/* Global Defines *****************************************************************/
#if ( configUSE_CUSTUME_HEAP_TYPE == 5 )
	HeapRegion_t xHeapRegions[] =
	{
		{ ( uint8_t * ) 0x10000000UL, 0x010000 },		/* CCRAM ( 64KB ) 			*/

#if ( VECT_TAB_SRAM )
		// 在 RAM 中调试程序时，则 RAM 不能加入到动态内存分配
		{ ( uint8_t * ) 0x20000000UL, 0x020000 },		/* RAM-Internal ( 128KB )	*/
#endif

		{ ( uint8_t * ) 0x68000000UL, 0x200000 },		/* SRAM ( 2MB)				*/
		{ NULL, 0 }
	};
#endif

#endif /* __USER_BSPMAP_H_ */
