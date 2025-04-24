#include <stdio.h>
#include "edf.h"
#include "driver/gpio.h"
#include "esp_system.h"

void test(void *pvParameter) {
    int n = *((int *) pvParameter);
    // printf("START WORK\n");
    // edf_print_task_arrays();
    vTaskDelay(pdMS_TO_TICKS(n));
    // printf("END WORK\n");
}

static int n = 3000;
static int m = 1000;

void app_main(void)
{
    //postavi ovoj dretvi (sam sebi) najveći prioritet PRIO_MAX

    edf_start_scheduler();

    edf_set(test, &n, sizeof(n), 10000, 7000);
    edf_set(test, &m, sizeof(m), 8000, 6000);
    vTaskDelay(1000);
}
