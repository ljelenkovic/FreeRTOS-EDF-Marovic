#include <stdio.h>
#include "edf.h"
#include <string.h>

#define MAX_TASKS 20
// postoji configNUM_CORES, ali trenutno testiram samo za jedan core
#define NUM_CORES 1

TaskHandle_t scheduler_handle;

EDFtask tasksReady[MAX_TASKS];
EDFtask tasksYield[MAX_TASKS];
int numTasks = 0;

// Testna funkcija koja printa ready i yield arrayeve
void edf_print_task_arrays() 
{
    printf("tasksReady: ");
    for (int i = 0; i < MAX_TASKS; i++) {
        printf("%d ", (int) tasksReady[i].globalDeadline);
    }
    printf("\ntasksYield: ");
    for (int i = 0; i < MAX_TASKS; i++) {
        printf("%d ", (int) tasksYield[i].globalDeadline);
    }
    printf("\n");
}

// miče task iz yield arraya i stavlja ga u ready array (array sortiran prema globalnom deadlineu)
// ako je task predan funkciji novi ili ako je prvi u arrayu, probudi scheduler task
int edf_ready_task(EDFtask task) 
{
    // if (tasksReady[MAX_TASKS-1].globalDeadline != 0) {
    //     printf("Ready array is full.\n");
    //     return 1;
    // }
    int i;
    int newTaskFlag = 0;
    for (i = 0; i < numTasks; i++) {
        if (tasksYield[i].handle == task.handle) {
            tasksYield[i] = (EDFtask) {0};
            break;
        }
    }
    if (i == numTasks)
        newTaskFlag = 1;

    for (i = 0; i < numTasks; i++) {
        if (tasksReady[i].globalDeadline == 0 || task.globalDeadline < tasksReady[i].globalDeadline) {
            break;
        }
    }
    for (int j = numTasks-1; j >= i; j--) {
        if (tasksReady[j].globalDeadline == 0)
            continue;
        tasksReady[j+1] = tasksReady[j];
    }
    tasksReady[i] = task;

    if ((newTaskFlag || i == 0) && scheduler_handle != 0)
        xTaskNotifyGive(scheduler_handle);
    return 0;
}

// task se stavlja u yield array i budi scheduler task
void edf_completed(EDFtask task) 
{
    // int i;
    // for (i = 0; i < NUM_CORES; i++) {
    //     if (tasksReady[i].handle == task.handle) {
    //         tasksReady[i] = (EDFtask) {0};
    //         break;
    //     }
    // }
    // for (int j = i+1; j < numTasks; j++) {
    //     if (tasksReady[j].globalDeadline == 0)
    //         break;
    //     tasksReady[j-1] = tasksReady[j];
    // }
    for (int j = 0; j < numTasks; j++) {
        if (tasksYield[j].handle == 0) {
            tasksYield[j] = task;
            break;
        }
    }
    xTaskNotifyGive(scheduler_handle);
}

// task koji se konstantno vrti (task i parameteri predani kao parametri funkcije)
// edf_yield je macro za vTaskDelayUntil - zaustavlja task do kraja perioda
void edf_task(void *pvParameter) 
{
    EDFtask task = *((EDFtask *) pvParameter);

    while (1) {
        TickType_t current_time = xTaskGetTickCount();
        task.globalDeadline = current_time + pdMS_TO_TICKS(task.deadline);
        edf_ready_task(task);
        // provjeriti return value
        uint32_t notif = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        task.task(task.params);
        edf_completed(task);
        edf_yield(&current_time, pdMS_TO_TICKS(task.period));
    }
}

// postavlja task koji će se konstantno vrtiti
TaskHandle_t edf_set(TaskFunction_t task, void *params, size_t param_size, int period, int deadline) 
{
    if (numTasks == MAX_TASKS) {
        printf("Can't add any more tasks.");
        return 0;
    }
    void *param_copy = malloc(param_size);
    if (param_copy == NULL) {
        printf("Memory allocation failed.");
        return 0;
    }

    // mozda bolje napraviti malloc prije samog poziva edf_set umjesto kopiranja? 
    memcpy(param_copy, params, param_size);
    EDFtask e;
    e.period = period;
    e.deadline = deadline;
    e.task = task;
    e.params = param_copy;
    xTaskCreate(edf_task, "EDF task", 2048, (void *) &e, 5, &e.handle);
    numTasks += 1;
    return e.handle;
}

// miče task iz ready arraya
void edf_unready_task(EDFtask task) 
{
    int i;
    for (i = 0; i < NUM_CORES; i++) {
        if (tasksReady[i].handle == task.handle) {
            tasksReady[i] = (EDFtask) {0};
            break;
        }
    }
    for (int j = i+1; j < numTasks+1; j++) {
        tasksReady[j-1] = tasksReady[j];
        if (tasksReady[j].globalDeadline == 0)
            break;
    }
}

// scheduler - čeka da ga netko pozove pa budi task koji je prvi u ready arrayu i miče ga iz nje
void edf_scheduler_task(void *pvParameter) 
{
    
    // samo za jedan core trenutno
    if (tasksReady[0].deadline != 0) {
        xTaskNotifyGive(tasksReady[0].handle);
        edf_unready_task(tasksReady[0]);
    }

    while (1) 
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (tasksReady[0].deadline != 0) {
            xTaskNotifyGive(tasksReady[0].handle);
            edf_unready_task(tasksReady[0]);
        }
    }
}

// pokreće scheduler task
void edf_start_scheduler(void)
{
    xTaskCreate(edf_scheduler_task, "EDF scheduler", 2048, NULL, configMAX_PRIORITIES-1, &scheduler_handle);
}
