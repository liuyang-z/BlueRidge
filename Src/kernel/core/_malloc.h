
#ifndef __KERNEL_CORE_MALLOC_H_
#define __KERNEL_CORE_MALLOC_H_

#include "FreeRTOS.h"

#define Kernel_MALLOC(x)                pvPortMalloc(x)
#define Kernel_FREE(x)					vPortFree(x)


#endif /* __KERNEL_CORE_MALLOC_H_ */
