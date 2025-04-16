#include <stdio.h>
#include "edf.h"
#include <string.h>

#define MAX_TASKS 20
// postoji configNUM_CORES, ali trenutno testiram samo za jedan core
#define NUM_CORES 1

TaskHandle_t scheduler_handle;

EDFtask *tasksReady[MAX_TASKS] = {NULL}; //promjenio u kazaljke
//EDFtask *tasksYield[MAX_TASKS]; //mislim da ovo polje nije potrebno radi funkcionalnosti, samo radi ispisa
int numTasks = 0; //broj taskova u tasksReady

// Testna funkcija koja printa ready i yield arrayeve
void edf_print_task_arrays() 
{
    printf("tasksReady: ");
    for (int i = 0; i < MAX_TASKS; i++) {
        printf("%d ", (int) tasksReady[i]->globalDeadline);
    }
    // printf("\ntasksYield: ");
    // for (int i = 0; i < MAX_TASKS; i++) {
    //     printf("%d ", (int) tasksYield[i]->globalDeadline);
    // }
    printf("\n");
}

// miče task iz yield arraya i stavlja ga u ready array (array sortiran prema globalnom deadlineu)
// ako je task predan funkciji novi ili ako je prvi u arrayu, probudi scheduler task
int edf_ready_task(EDFtask *task) 
{
    int i;
    // int newTaskFlag = 0;
    // for (i = 0; i < numTasks; i++) {
    //     if (tasksYield[i]->handle == task->handle) {
    //         tasksYield[i] = (EDFtask) {0};
    //         break;
    //     }
    // }
    // if (i == numTasks)
    //     newTaskFlag = 1;

    for (i = 0; i < numTasks; i++) {
        if (task->globalDeadline < tasksReady[i]->globalDeadline) {
            break;
        }
    }
    for (int j = numTasks-1; j >= i; j--) {
        tasksReady[j+1] = tasksReady[j];
    }
    tasksReady[i] = task;
    numTasks += 1;

    xTaskNotifyGive(scheduler_handle);

    return 0;
}

// task se stavlja u yield array i budi scheduler task
void edf_completed(EDFtask *task) 
{
    // for (int j = 0; j < numTasks; j++) {
    //     if (tasksYield[j]->handle == 0) {
    //         tasksYield[j] = task;
    //         break;
    //     }
    // }
    xTaskNotifyGive(scheduler_handle);
}

// task koji se konstantno vrti (task i parameteri predani kao parametri funkcije)
// edf_yield je macro za vTaskDelayUntil - zaustavlja task do kraja perioda
void edf_task(void *pvParameter) 
{
    EDFtask *task = *((EDFtask *) pvParameter);

    TickType_t current_time = xTaskGetTickCount();
    while (1) {
        task->globalDeadline = current_time + pdMS_TO_TICKS(task->deadline);
        edf_ready_task(task);
        // provjeriti return value
        uint32_t notif = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        task->task(task->params);
        edf_completed(task);
        vTaskDelayUntil(&current_time, pdMS_TO_TICKS(task->period));
        //current_time += pdMS_TO_TICKS(task->period) -> ovo radi prethodna funkcija!
    }
}

// postavlja task koji će se konstantno vrtiti
TaskHandle_t edf_set(TaskFunction_t task, void *params, size_t param_size, int period, int deadline) 
{
    if (numTasks == MAX_TASKS) {
        printf("Can't add any more tasks.");
        return -1;
    }

    EDFtask *e = malloc(sizeof(EDFtask));
    if (e == NULL) {
        printf("Memory allocation failed.");
        return -1;
    }
    e->period = period;
    e->deadline = deadline;
    e->task = task;
    e->params = params;
    xTaskCreate(edf_task, "EDF task", 2048, (void *) e, 5, &e->handle);
    return e->handle;
}

// miče task iz ready arraya
void edf_unready_task(EDFtask *task) 
{
    //miče se ali trebala bi biti prva?
    for (int j = 1; j < numTasks; j++) {
        tasksReady[j-1] = tasksReady[j];
    }
    numTasks -= 1;

}

// scheduler - čeka da ga netko pozove pa budi task koji je prvi u ready arrayu i miče ga iz nje
void edf_scheduler_task(void *pvParameter) 
{
    while (1) 
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (tasksReady[0]->deadline != 0) {
            edf_unready_task(tasksReady[0]);
            xTaskNotifyGive(tasksReady[0]->handle);
        }
    }
}

// pokreće scheduler task
void edf_start_scheduler(void)
{
    xTaskCreate(edf_scheduler_task, "EDF scheduler", 2048, NULL, configMAX_PRIORITIES-1, &scheduler_handle);
}
