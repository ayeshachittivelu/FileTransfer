#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

volatile uint64_t ulTickCounter = 0;

/* Forward declaration */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    while (1);
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    StackType_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* Simple test task */
void test_task1(void *pvParameters)
{
    (void) pvParameters;

    while (1)
    {
        ulTickCounter++;   // should increment every 500ms
        printf("TickCounter = %u\n", (unsigned int)ulTickCounter);
        vTaskDelay(pdMS_TO_TICKS(500));
        printf("Timer delay is completed 1.\n");

    }
}

/* Simple test task */
void test_task2(void *pvParameters)
{
    (void) pvParameters;

    while (1)
    {
        ulTickCounter++;   // should increment every 500ms
        printf("TickCounter = %u\n", (unsigned int)ulTickCounter);
        vTaskDelay(pdMS_TO_TICKS(500));
        printf("Timer delay is completed 2.\n");

    }
}


int main(void)
{
    printf("Starting FreeRTOS on ARMv8-A...\n");
    xTaskCreate(test_task1, "T1", 1024, NULL, 1, NULL);
    xTaskCreate(test_task2, "T2", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1);
}
