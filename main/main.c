#include <stdio.h>
#include "edf.h"
#include "driver/gpio.h"
#include "esp_system.h"

#define BUTTON_PIN GPIO_NUM_26

static uint32_t last_interrupt_time = 0;

void IRAM_ATTR isr_handler(void *arg) {
    uint32_t current_time = xTaskGetTickCountFromISR();
    if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(200)) {
    }
    last_interrupt_time = current_time;
}

void test(void *pvParameter) {
    int n = *((int *) pvParameter);
    // printf("START WORK\n");
    // edf_print_task_arrays();
    vTaskDelay(pdMS_TO_TICKS(n));
    // printf("END WORK\n");
}

void button_task(void *pvParameter)
{
    while (1) 
    {
        if (gpio_get_level(BUTTON_PIN)) {
            edf_start_scheduler();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void print_task(void *pvParameter)
{
    while (1)
    {
        edf_print_task_arrays();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    // gpio_set_direction(BUTTON_PIN, GPIO_MODE_DEF_INPUT);
    // xTaskCreate(button_task, "button", 2048, NULL, 10, NULL);
    // xTaskCreate(print_task, "print", 2048, NULL, 10, NULL);
    int n = 3000;
    edf_set(test, &n, sizeof(n), 10000, 7000);
    n = 1000;
    edf_set(test, &n, sizeof(n), 8000, 6000);
    vTaskDelay(1000);
    edf_start_scheduler();
    // while (1) {
    //     edf_print_task_arrays();
    //     vTaskDelay(1000);
    // }
}