
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"

#include "kernel/core/_log.h"
#include "driver/gpio/led.h"


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
}

size_t thread_test_exit = 0;

void* print_test(void *arg) {
    int cnt = 0;
    while( thread_test_exit == 0) {
        LOGI("print test_%d", cnt++);
        sleep(1);
    }

    return (NULL);
}

void* user_main(void *arg) {

    pthread_t test_a;
    pthread_t test_b;

    float a=2.1,b;

	a=2.3*3.1;
	b=a*12;
	a=b/3.3;

    pthread_create(&test_a, NULL, print_test, NULL);
    pthread_create(&test_b, NULL, print_test, NULL);

    int cnt = 0;

    for ( ;; ) {
        sleep(1);

        if(led_stat(0)) {
            led_act(0, 1);
        } else {
            led_act(0, 0);
        }

        if(cnt++ == 10) {
            thread_test_exit = 1;
            break;
        }
    }

    pthread_join(test_b, NULL);
    // pthread_join(test_a, NULL);

    return (NULL);
}

/* end of this file. */
