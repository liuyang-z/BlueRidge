
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"

#include "kernel/core/_log.h"
#include "driver/gpio/led.h"
#include "driver/fsmc/sram.h"

#include "FreeRTOS.h"
#include <memory.h>


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
      /* loop infinity */
  }
}

void Hw_Init() {
    /* init led (gpio) */
    led_cfg();

    // init sram
    Hw_Sram_Init();
}

size_t thread_test_exit = 0;
struct led_s {
    int pos;
    int delay;
};

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

void* user_main(void *arg) {

    pthread_t blink_a;
    pthread_t blink_b;
    pthread_t blink_c;
    pthread_t blink_d;

    float a=2.1,b;

	a=2.3*3.1;
	b=a*12;
	a=b/3.3;

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
