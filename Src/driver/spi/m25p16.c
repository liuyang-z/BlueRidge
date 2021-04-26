

/* Include ************************************************************************/
#include "./m25p16.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "kernel/core/_log.h"


/* SPI & Flash struct pro *********************************************************/
typedef struct _base_func_t {
	HAL_StatusTypeDef (* init)(SPI_HandleTypeDef *hspi);
	HAL_StatusTypeDef (* send)(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	HAL_StatusTypeDef (* recv)(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	void (* cs_low)();
	void (* cs_high)();
} _base_func;

typedef struct _spi_bus_t {
	SPI_HandleTypeDef 	handle;
	_base_func 			func;
	GPIO_TypeDef		*port;
	GPIO_InitTypeDef	mio;
} _spi_bus;

typedef struct _m25p16_dev_t {
	_spi_bus			spi_bus;

	HAL_StatusTypeDef (* command_send)(uint8_t, uint8_t*, uint8_t);
	HAL_StatusTypeDef (* write_enable)();
	HAL_StatusTypeDef (* read_stat_reg)(uint8_t*);
	HAL_StatusTypeDef (* wait_until_to)(uint32_t);

	HAL_StatusTypeDef (* read_id)(uint8_t*, uint8_t);
	HAL_StatusTypeDef (* read_data)(uint8_t*, uint8_t*, uint32_t);
	HAL_StatusTypeDef (* erase_sector)(uint8_t*);
	HAL_StatusTypeDef (* erase_bulk)();
	HAL_StatusTypeDef (* page_program)(uint8_t*, uint8_t*, uint32_t);
} _m25p16_dev;


/* M25P16 Command and Reg define **************************************************/
#define M25P16_PAGE_SIZE					( 256 )				/* 页大小 256B */
#define M25P16_PAGE_MASK					( 0xFFFFFF00 )
#define M25P16_SECTOR_SIZE					( 0x00010000 )		/* 扇区大小 64KB */
#define M25P16_SECTOR_MASK					( 0xFFFF0000 )
#define M25P16_MAX_SIZE						( 0x00200000 )

#define COMMAND_INSTR_WREN					( 0x06 )
#define COMMAND_INSTR_WRDI					( 0x04 )
#define COMMAND_INSTR_RDID					( 0x9F )
#define COMMAND_INSTR_RDSR					( 0x05 )
#define COMMAND_INSTR_WRSR					( 0x01 )
#define COMMAND_INSTR_READ					( 0x03 )
#define COMMAND_INSTR_FAST_READ				( 0x0B )
#define COMMAND_INSTR_PP					( 0x02 )
#define COMMAND_INSTR_SE					( 0xD8 )
#define COMMAND_INSTR_BE					( 0xC7 )
#define COMMAND_INSTR_DP					( 0xB9 )
#define COMMAND_INSTR_RES					( 0xAB )

#define STATUS_REG_WIP_Pos					( 0 )
#define STATUS_REG_WIP_Mask					( 1 << STATUS_REG_WIP_Pos )
#define STATUS_REG_WIP						( STATUS_REG_WIP_Mask )
#define STATUS_REG_WEL_Pos					( 1 )
#define STATUS_REG_WEL_Mask					( 1 << STATUS_REG_WEL_Pos )
#define STATUS_REG_WEL						( STATUS_REG_WEL_Mask )
#define STATUS_REG_BP0_Pos					( 2 )
#define STATUS_REG_BP0_Mask					( 1 << STATUS_REG_BP0_Pos )
#define STATUS_REG_BP0						( STATUS_REG_BP0_Mask )
#define STATUS_REG_BP1_Pos					( 3 )
#define STATUS_REG_BP1_Mask					( 1 << STATUS_REG_BP1_Pos )
#define STATUS_REG_BP1						( STATUS_REG_BP1_Mask )
#define STATUS_REG_BP2_Pos					( 4 )
#define STATUS_REG_BP2_Mask					( 1 << STATUS_REG_BP2_Pos )
#define STATUS_REG_BP2						( STATUS_REG_BP2_Mask )
#define STATUS_REG_SRWD_Pos					( 7 )
#define STATUS_REG_SRWD_Mask				( 1 << STATUS_REG_SRWD_Pos )
#define STATUS_REG_SRWD						( STATUS_REG_SRWD_Mask )


/* Global defines *****************************************************************/
static _m25p16_dev _g_m26p16_flash;


static void _spi_mio_cfg(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	_spi_bus* spi_bus = &_g_m26p16_flash.spi_bus;

	spi_bus->port            = GPIOA;
	spi_bus->mio.Pin         = GPIO_PIN_5;
	spi_bus->mio.Pull        = GPIO_NOPULL;
	spi_bus->mio.Mode        = GPIO_MODE_OUTPUT_PP;
	spi_bus->mio.Speed       = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(spi_bus->port, &spi_bus->mio);

	spi_bus->port            = GPIOB;
	spi_bus->mio.Pin         = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	spi_bus->mio.Mode        = GPIO_MODE_AF_PP;
	spi_bus->mio.Pull        = GPIO_NOPULL;
	spi_bus->mio.Speed       = GPIO_SPEED_FREQ_VERY_HIGH;
	spi_bus->mio.Alternate   = GPIO_AF5_SPI2;
	HAL_GPIO_Init(spi_bus->port, &spi_bus->mio);
}

static void _spi_mio_cs_low()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
}

static void _spi_mio_cs_high()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
}

static void _spi_hal_cfg(void)
{
	__HAL_RCC_SPI2_CLK_ENABLE();

	_spi_bus* spi_bus = &_g_m26p16_flash.spi_bus;

	spi_bus->handle.Instance                 = SPI2;
	spi_bus->handle.Init.Mode                = SPI_MODE_MASTER;
	spi_bus->handle.Init.DataSize            = SPI_DATASIZE_8BIT;
	spi_bus->handle.Init.Direction           = SPI_DIRECTION_2LINES;
	spi_bus->handle.Init.CLKPolarity         = SPI_POLARITY_HIGH;
	spi_bus->handle.Init.CLKPhase            = SPI_PHASE_2EDGE;
	spi_bus->handle.Init.NSS                 = SPI_NSS_SOFT;
	spi_bus->handle.Init.BaudRatePrescaler   = SPI_BAUDRATEPRESCALER_2;
	spi_bus->handle.Init.FirstBit            = SPI_FIRSTBIT_MSB;
	spi_bus->handle.Init.TIMode              = SPI_TIMODE_DISABLE;
	spi_bus->handle.Init.CRCCalculation      = SPI_CRCCALCULATION_DISABLE;

	if(spi_bus->func.init(&spi_bus->handle)
		!= HAL_OK) {
		LOGE("spi init is error.");
		return;
	}

	/* enable the spi2 */
	__HAL_SPI_ENABLE(&spi_bus->handle);
}


/* Low level Interface ************************************************************/

/* __m25p16_data_recv **************************************************************
*   Low Level 接口，调用 SPI 总线接收数据
***********************************************************************************/
HAL_StatusTypeDef __m25p16_data_recv(uint8_t* buff, uint32_t len)
{
	if(_g_m26p16_flash.spi_bus.func.recv(&_g_m26p16_flash.spi_bus.handle, buff, len, 300) != HAL_OK) {
		LOGE("[SPI] Fail to receive data.");
		return (HAL_ERROR);
	}

	return (HAL_OK);
}

/* __m25p16_data_send **************************************************************
*   Low Level 接口，调用 SPI 总线发送数据
***********************************************************************************/
HAL_StatusTypeDef __m25p16_data_send(uint8_t* buff, uint32_t len)
{
	if(_g_m26p16_flash.spi_bus.func.send(&_g_m26p16_flash.spi_bus.handle, buff, len, 300)
		!= HAL_OK) {
		LOGE("[SPI] Fail to send data.");
		return (HAL_ERROR);
	}

	return (HAL_OK);
}

/* Internal Interface *************************************************************/

/* _m25p16_command_send ************************************************************
*   内部接口，通过 SPI 总线发送指令以及后续的偏移地址
*	in: command				Flash 指令
*	in: data				3Byte 长度的地址信息，可选
* 	in: len					地址长度，可选
***********************************************************************************/
HAL_StatusTypeDef _m25p16_command_send(uint8_t command, uint8_t* data, uint8_t len)
{
	uint8_t comm[1] = { command };

	if(__m25p16_data_send(comm, 1) != HAL_OK) {
		goto _error;
	}

	if(data == NULL || len == 0) {
		goto _success;
	}

	if(__m25p16_data_send(data, len) != HAL_OK)  {
		goto _error;
	}

_success:
	return (HAL_OK);

_error:
	LOGE("%s, failed to send data", __func__);
	return (HAL_ERROR);
}

/* _m25p16_write_enable ************************************************************
*   内部接口，发送写使能指令，在擦除、写页等指令执行前需要先发送写使能指令
***********************************************************************************/
HAL_StatusTypeDef _m25p16_write_enable(void)
{
	_g_m26p16_flash.spi_bus.func.cs_low();

	uint8_t command = COMMAND_INSTR_WREN;
	if(_m25p16_command_send(command, NULL, 0)  != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_ERROR);
}

/* _m25p16_read_stat_reg ***********************************************************
*   内部接口，读取 Flash 状态寄存器
* 	out: data				读取、存储状态寄存器内容
***********************************************************************************/
HAL_StatusTypeDef _m25p16_read_stat_reg(uint8_t* data)
{
	if(data == NULL) {
		// debug info
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_low();

	uint8_t command = COMMAND_INSTR_RDSR;
	if(_m25p16_command_send(command, NULL, 0) != HAL_OK) {
		LOGE("%s, failed to send command.", __func__);
		goto _error;
	}

	if(__m25p16_data_recv(data, 1) != HAL_OK) {
		LOGE("%s, failed to receive data.", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_ERROR);
}

/* _m25p16_wait_flash_until_to *****************************************************
*   内部接口，读取 Flash 状态寄存器，判断 Flash 是否正忙并带有超时限制
* 	out: timeout			超时事件，单位毫秒
***********************************************************************************/
HAL_StatusTypeDef _m25p16_wait_flash_until_to(uint32_t timeout)
{
	uint8_t status_reg = 0;
	uint32_t tick_old = HAL_GetTick();
	uint32_t to = timeout;

	do {
		if(_m25p16_read_stat_reg(&status_reg) != HAL_OK) {
			LOGE("%s, failed to read stat reg.", __func__);
			goto _error;
		}

		if((status_reg & STATUS_REG_WIP_Mask) == 0) {
			break;
		}

	} while(HAL_GetTick() - tick_old < to);

	if(HAL_GetTick() - tick_old >= to) {
		LOGE("%s, check flash busy, and timeout.", __func__);
		goto _error;
	}

	return (HAL_OK);

_error:
	return (HAL_ERROR);
}

/* _m25p16_read_id *****************************************************************
*   内部接口，用于读取 Flash ID
* 	out: buff				缓冲指针
* 	out: len				读取的数据长度，长度 <= 3Byte
***********************************************************************************/
HAL_StatusTypeDef _m25p16_read_id(uint8_t* buff, uint8_t len)
{
	_g_m26p16_flash.spi_bus.func.cs_low();

	// send command
	uint8_t command = COMMAND_INSTR_RDID;
	if(_m25p16_command_send(command, NULL, 0) != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	// read data
	if(__m25p16_data_recv(buff, len) != HAL_OK) {
		LOGE("%s, failed to receive data.", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_ERROR);
}

/* _m25p16_read_data ***************************************************************
*   内部接口，用于读取指定偏移地址的数据
* 	in: address				偏移地址，地址长度为 3Byte
* 	in: buff				缓冲指针
* 	in: len					读取的数据长度
***********************************************************************************/
HAL_StatusTypeDef _m25p16_read_data(uint8_t* address, uint8_t* buff, uint32_t len)
{
	_g_m26p16_flash.spi_bus.func.cs_low();

	// 发送快速读取指令，并附带有三个字节的 Flash 偏移地址
	uint8_t command = COMMAND_INSTR_FAST_READ;
	if(_m25p16_command_send(command, address, 3) != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	// 发送一个字节的 Dummy 时钟
	uint8_t dummy = { 0 };
	if(__m25p16_data_send(&dummy, 1) != HAL_OK) {
		LOGE("%s, failed to send data.", __func__);
		goto _error;
	}

	// 开始接收指定长度的数据
	if(__m25p16_data_recv(buff, len) != HAL_OK) {
		LOGE("%s, failed to receive data.", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
	return (HAL_ERROR);
}

/* _m25p16_erase_sector ************************************************************
*   内部接口，擦除指定地址所在的扇区（64KB）
***********************************************************************************/
HAL_StatusTypeDef _m25p16_erase_sector(uint8_t* address)
{
	if(_m25p16_write_enable() != HAL_OK) {
		LOGE("%s, failed to write enable.");
		goto _error2;
	}

	_g_m26p16_flash.spi_bus.func.cs_low();

	uint8_t command = COMMAND_INSTR_SE;
	if(_m25p16_command_send(command, address, 3) != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();

	if(_m25p16_wait_flash_until_to(400) != HAL_OK) {
		LOGE("%s, check flash busy, and timeout.", __func__);
		goto _error2;
	}

	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
_error2:
	return (HAL_ERROR);
}

/* _m25p16_erase_bulk **************************************************************
*   内部接口，对 Flash 进行全片擦除
***********************************************************************************/
HAL_StatusTypeDef _m25p16_erase_bulk(void)
{
	if(_m25p16_write_enable() != HAL_OK) {
		LOGE("%s, failed to write enable.");
		goto _error2;
	}

	_g_m26p16_flash.spi_bus.func.cs_low();

	uint8_t command = COMMAND_INSTR_BE;
	if(_m25p16_command_send(command, NULL, 0) != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();

	if(_m25p16_wait_flash_until_to(20*1000) != HAL_OK) {
		LOGE("%s, check flash busy, and timeout.", __func__);
		goto _error2;
	}

	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();
_error2:
	return (HAL_ERROR);
}

/* _m25p16_page_program ************************************************************
*   内部接口，通过页编程 PP 写 Flash，最大写入 256B，偏移地址不是页对齐，会出现卷绕。
*	in: address				欲写入的地址，地址长度为 3Byte
* 	in: buff				写入的内容指针
* 	in: len					数据长度
***********************************************************************************/
HAL_StatusTypeDef _m25p16_page_program(uint8_t* address, uint8_t* buff, uint32_t len)
{
	if(_m25p16_write_enable() != HAL_OK) {
		LOGE("%s, failed to write enable.");
		goto _error2;
	}

	_g_m26p16_flash.spi_bus.func.cs_low();

	uint8_t command = COMMAND_INSTR_PP;
	if(_m25p16_command_send(command, address, 3) != HAL_OK) {
		LOGE("%s, failed to send command", __func__);
		goto _error;
	}

	if(__m25p16_data_send(buff, len) != HAL_OK) {
		LOGE("%s, failed to send data.", __func__);
		goto _error;
	}

	_g_m26p16_flash.spi_bus.func.cs_high();

	if(_m25p16_wait_flash_until_to(400) != HAL_OK) {
		LOGE("%s, check flash busy, and timeout.", __func__);
		goto _error2;
	}

	return (HAL_OK);

_error:
	_g_m26p16_flash.spi_bus.func.cs_high();

_error2:
	return (HAL_ERROR);
}

/* External Interface *************************************************************/

/* m25p16_drv_init *****************************************************************
*   外部接口，初始化 SPI 总线接口，并注册 Flash 相关回调函数。
***********************************************************************************/
void m25p16_drv_init(void)
{
	_spi_bus *spi_bus = &_g_m26p16_flash.spi_bus;

	// reg func hook
	spi_bus->func.init       = HAL_SPI_Init;
	spi_bus->func.recv       = HAL_SPI_Receive;
	spi_bus->func.send       = HAL_SPI_Transmit;
	spi_bus->func.cs_high    = _spi_mio_cs_high;
	spi_bus->func.cs_low     = _spi_mio_cs_low;

	/* low level init */
	_spi_mio_cfg();

	/* spi reg init */
	_spi_hal_cfg();

	/* 默认拉高 SPI 片选 */
	spi_bus->func.cs_high();

	/* M25P16 内部回调注册，外部接口只能够调用已注册的回调 */
	_g_m26p16_flash.command_send   = _m25p16_command_send;
	_g_m26p16_flash.write_enable   = _m25p16_write_enable;
	_g_m26p16_flash.wait_until_to  = _m25p16_wait_flash_until_to;

	_g_m26p16_flash.read_id        = _m25p16_read_id;
	_g_m26p16_flash.read_stat_reg  = _m25p16_read_stat_reg;
	_g_m26p16_flash.read_data      = _m25p16_read_data;
	_g_m26p16_flash.erase_sector   = _m25p16_erase_sector;
	_g_m26p16_flash.erase_bulk     = _m25p16_erase_bulk;
	_g_m26p16_flash.page_program   = _m25p16_page_program;
}

/* m25p16_read_id ******************************************************************
*   外部接口，用于读取 Flash 的内部唯一 ID。
*   out: id					读取ID后保存到此地址所指定的位置
***********************************************************************************/
HAL_StatusTypeDef m25p16_read_id(uint32_t *id)
{
	uint8_t buff[3] = { 0 };
	if(_g_m26p16_flash.read_id == NULL)
	{
		// debug info
		return (HAL_ERROR);
	}

	if(_g_m26p16_flash.read_id(buff, 3) != HAL_OK) {
		// debug info
		return (HAL_ERROR);
	}

	*id = (buff[0] << 16) + (buff[1] << 8) + buff[2];
	return (HAL_OK);
}

/* m25p16_erase_sector *************************************************************
*   外部接口，用于擦除指定地址所在扇区（64KB）的内容。
*   in: address				欲擦出的 Flash 扇区地址，地址若非 64KB 对齐，会启动向前对齐。
***********************************************************************************/
HAL_StatusTypeDef m25p16_erase_sector(uint32_t address)
{
	// 将传入的地址向前进行扇区对齐
	uint32_t sector_start_addr = address & M25P16_SECTOR_MASK;
	uint8_t tmp_addr[3] = {
		((sector_start_addr & 0x00ff0000) >> 16) & 0xff,
		((sector_start_addr & 0x0000ff00) >>  8) & 0xff,
		((sector_start_addr & 0x000000ff))       & 0xff
	};

	if(_g_m26p16_flash.erase_sector != NULL)
		return _g_m26p16_flash.erase_sector(tmp_addr);

	return (HAL_ERROR);
}

/* m25p16_erase_bulk ***************************************************************
*   外部接口，擦除整个芯片，此操作大致会耗时 18s，谨慎操作。
***********************************************************************************/
HAL_StatusTypeDef m25p16_erase_bulk()
{
	if(_g_m26p16_flash.erase_bulk != NULL)
		return _g_m26p16_flash.erase_bulk();

	return (HAL_ERROR);
}

/* m25p16_read *********************************************************************
*   外部接口，从指定地址读取一定长度的数据。
*   in: address				Flash 偏移地址
* 	in: buff				数据缓冲指针
*   in: len					读取长度
***********************************************************************************/
HAL_StatusTypeDef m25p16_read(uint32_t address, uint8_t* buff, uint32_t len)
{
	uint8_t tmp_addr[3] = {
		((address & 0x00ff0000) >> 16) & 0xff,
		((address & 0x0000ff00) >>  8) & 0xff,
		((address & 0x000000ff))       & 0xff
	};

	if(_g_m26p16_flash.read_data != NULL) {
		return _g_m26p16_flash.read_data(tmp_addr, buff, len);
	}

	return (HAL_ERROR);
}

/* m25p16_write_page ***************************************************************
*   外部接口，写入数据到 Flash，写入之前需要先进性擦除操作
*	注意：写入长度最大不能超过 PAGE_SIZE，并且会自动将超出页边界之后的裁剪掉。
*   in: address				Flash 偏移地址
* 	in: buff				外部数据指针
*   in: len					写入长度，长度 <= PAGE_SIZE
***********************************************************************************/
HAL_StatusTypeDef m25p16_write_page(uint32_t address, uint8_t* buff, uint32_t len)
{
	uint8_t offset_addr[3] = {
		((address & 0x00ff0000) >> 16) & 0xff,
		((address & 0x0000ff00) >>  8) & 0xff,
		((address & 0x000000ff))       & 0xff
	};

	/* 校验确保写入的数据再一个页（256B）之内，而且不能回卷 */
	uint32_t page_end_addr = (address & M25P16_PAGE_MASK) + M25P16_PAGE_SIZE;
	uint32_t valid_len = (address + len < page_end_addr)
							? (len)
							: (page_end_addr - address);

	if(_g_m26p16_flash.page_program != NULL)
		return _g_m26p16_flash.page_program(offset_addr, buff, valid_len);

	return (HAL_ERROR);
}

/* m25p16_write ********************************************************************
*   外部接口，写入数据到 Flash，写入之前需要先进性擦除操作
*   in: address				Flash 偏移地址
* 	in: buff				外部数据指针
*   in: len					写入长度，数据长度不受页长度限制
***********************************************************************************/
HAL_StatusTypeDef m25p16_write(uint32_t address, uint8_t* buff, uint32_t len)
{
	if((address + len >= M25P16_MAX_SIZE)
		|| (buff == NULL)) {
		// debug info
		goto _error;
	}

	uint32_t remaining_len   = len;
	uint32_t current_addr    = address;
	uint8_t* buff_pos	     = buff;

	// debug info

	while(remaining_len > 0) {
		uint32_t tmp_page_end = (current_addr & M25P16_PAGE_MASK)
									+ M25P16_PAGE_SIZE;
		uint32_t tmp_len = (tmp_page_end < (current_addr + remaining_len))
							? (tmp_page_end - current_addr)
							: (remaining_len);

		if(m25p16_write_page(current_addr, buff_pos, tmp_len)
			!= HAL_OK) {
			LOGE("%s, failed to write page.", __func__);
			goto _error;
		}

		remaining_len -= tmp_len;
		current_addr  += tmp_len;
		buff_pos      += tmp_len;
	}

	return (HAL_OK);

_error:
	return (HAL_ERROR);
}
