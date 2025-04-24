#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define edf_yield( current_time, period ) vTaskDelayUntil( (current_time), (period) )

// edf_set              ✓
// edf_set_deadline     ✓
// edf_completed
// edf_yield            ✓
// edf_start_scheduler  ✓

#define PRIO_MAX        (configMAX_PRIORITIES-1)
#define PRIO_SCHEDULER  PRIO_MAX
#define PRIO_SLEEP      (PRIO_MAX-2)
#define PRIO_ACTIVE     (PRIO_MAX-3)
#define PRIO_READY      (PRIO_MAX-4)


typedef struct {
    int period;
    int deadline;
    int globalDeadline;
    int status; //EDF_READY, EDF_STARTED, EDF_SLEEP
    TaskHandle_t handle;
    TaskFunction_t task;
    int id;
    void *params;
} EDFtask;

void edf_print_task_arrays();
TaskHandle_t edf_set(TaskFunction_t task, void *params, size_t param_size, int period, int deadline);
void edf_start_scheduler();
