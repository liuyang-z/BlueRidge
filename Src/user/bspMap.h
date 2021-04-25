
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
		// �� RAM �е��Գ���ʱ���� RAM ���ܼ��뵽��̬�ڴ����
		// RAM-Internal(28KB + 100KB = 128KB)
		// ����ǰ 28KB ���� bss ���򣬺� 100KB ���ڶ�̬�ڴ����
		{ ( uint8_t * ) 0x20007000UL, 0x019000 },		/* RAM-Internal ( 100KB )	*/
#endif

		{ ( uint8_t * ) 0x68000000UL, 0x200000 },		/* SRAM ( 2MB)				*/
		{ NULL, 0 }
	};
#endif

#endif /* __USER_BSPMAP_H_ */
