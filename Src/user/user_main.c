
/* Include ************************************************************************/
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"

#include "stm32f4xx.h"
#include "kernel/core/_log.h"
#include "kernel/core/_malloc.h"
#include "driver/gpio/led.h"
#include "driver/spi/m25p16.h"

/* Global Defines *****************************************************************/
size_t thread_test_exit = 0;
struct led_s {
    int pos;
    int delay;
};

/* blink ***************************************************************************
*   Led 周期闪烁控制方法
*   in: arg         struct led_s 指针
*                   内部含有 led pos 和 闪烁周期等参数
***********************************************************************************/
void* blink(void *arg) {
    struct led_s *led = (struct led_s *)arg;

    while( thread_test_exit == 0) {

        if(led_stat(led->pos)) {
            led_act(led->pos, 1);
        } else {
            led_act(led->pos, 0);
        }

        usleep(led->delay);
    }

    return (NULL);
}

/* user_main ***********************************************************************
*   用户函数入口
*   in: arg         NULL
***********************************************************************************/
void* user_main(void *arg) {

    pthread_t blink_a;
    pthread_t blink_b;
    pthread_t blink_c;
    pthread_t blink_d;

    uint32_t flash_id = 0;
    if(m25p16_read_id(&flash_id) != HAL_OK) {
        LOGE("read flash error.");
    } else {
        LOGI("flash id: %x", flash_id);
    }

    uint8_t size = 35;
    uint8_t* send = Kernel_MALLOC(size);
    uint8_t* recv = Kernel_MALLOC(size);

    if(send == NULL || recv == NULL) return NULL;

    for(int i = 0; i < size; i++) {
        send[i] = i;
    }

    uint32_t address = 0x0;
    if(m25p16_erase_sector(address) != HAL_OK) {
        LOGE("erase filure.");
    }

    uint32_t start = HAL_GetTick();
    if(m25p16_write_page(address, send, size) != HAL_OK) {
        LOGE("write filure.");
    }
    HAL_Delay(100);
    uint32_t stop = HAL_GetTick();

    if(m25p16_read(address, recv, size) != HAL_OK) {
        LOGE("read filure.");
    }

    for(int idx = 0; idx < size; idx++) {
        LOGI("%d", recv[idx]);
    }

    LOGW("write time: %d.", stop - start);

    Kernel_FREE(send);
    Kernel_FREE(recv);

    struct led_s led_a = { 0 };
    struct led_s led_b = { 0 };
    struct led_s led_c = { 0 };
    struct led_s led_d = { 0 };

    led_a.pos = 0;
    led_a.delay = 1000 * 200;
    pthread_create(&blink_a, NULL, blink, (void*)&led_a);

    led_b.pos = 1;
    led_b.delay = 1000 * 400;
    pthread_create(&blink_b, NULL, blink, (void*)&led_b);

    led_c.pos = 2;
    led_c.delay = 1000 * 600;
    pthread_create(&blink_c, NULL, blink, (void*)&led_c);

    led_d.pos = 3;
    led_d.delay = 1000 * 800;
    pthread_create(&blink_d, NULL, blink, (void*)&led_d);



    pthread_join(blink_a, NULL);
    pthread_join(blink_b, NULL);
    pthread_join(blink_c, NULL);
    pthread_join(blink_d, NULL);

    return (NULL);
}

/* end of this file. */
