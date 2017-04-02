#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

extern void* TestFunc_PWM(void* arg);
extern void* Test_IRQ(void* arg);
extern void* TestFunc_64bitenable(void* arg);
extern void* TestFunc_timeout(void* arg);
extern void* TestFunc_count(void* arg);
extern void* TestFunc_interrupt(void* arg);
extern void* TestFunc_count_interrupt(void* arg);

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_create(&task, &attr, TestFunc_count, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
