
#ifndef __DRIVER_M25P16_H_
#define __DRIVER_M25P16_H_


#include "stm32f4xx.h"

void 				m25p16_drv_init		(void);
HAL_StatusTypeDef   m25p16_read_id      (uint32_t *id);
HAL_StatusTypeDef 	m25p16_erase_sector	(uint32_t address);
HAL_StatusTypeDef 	m25p16_erase_bulk	(void);
HAL_StatusTypeDef 	m25p16_read			(uint32_t address, uint8_t* buff, uint32_t len);
HAL_StatusTypeDef 	m25p16_write_page	(uint32_t address, uint8_t* buff, uint32_t len);
HAL_StatusTypeDef 	m25p16_write		(uint32_t address, uint8_t* buff, uint32_t len);


#endif /* __DRIVER_N25P16_H_ */
