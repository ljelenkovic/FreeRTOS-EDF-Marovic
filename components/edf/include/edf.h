#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define edf_yield( current_time, period ) vTaskDelayUntil( (current_time), (period) )

// edf_set              ✓
// edf_set_deadline     ✓
// edf_completed
// edf_yield            ✓
// edf_start_scheduler  ✓

typedef struct {
    int period;
    int deadline;
    int globalDeadline;
    TaskHandle_t handle;
    TaskFunction_t task;
    void *params;
} EDFtask;

void edf_print_task_arrays();
TaskHandle_t edf_set(TaskFunction_t task, void *params, size_t param_size, int period, int deadline);
void edf_start_scheduler();
