
#ifndef __USER_BSPMAP_H_
#define __USER_BSPMAP_H_

/* Include ************************************************************************/
#include "FreeRTOS.h"

/* Global Defines *****************************************************************/
#if ( configUSE_CUSTUME_HEAP_TYPE == 5 )
	HeapRegion_t xHeapRegions[] =
	{
		{ ( uint8_t * ) 0x10000000UL, 0x010000 },
		{ ( uint8_t * ) 0x68000000UL, 0x200000 },
		{ NULL, 0 }
	};
#endif

#endif /* __USER_BSPMAP_H_ */
