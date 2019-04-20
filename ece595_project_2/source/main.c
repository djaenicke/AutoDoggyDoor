/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "board.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "tcpip_app_iface.h"
#include "fsl_device_registers.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "io_abstraction.h"
#include "proximity_estimation.h"
#include "http_client_app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Stack size of the thread which prints DHCP info. */
#define PRINT_THREAD_STACKSIZE 512

/*! @brief Priority of the thread which prints DHCP info. */
#define PRINT_THREAD_PRIO configMAX_PRIORITIES - 1

#define XBEE

typedef struct Task_Cfg_Tag
{
    TaskFunction_t func;
    const char name[configMAX_TASK_NAME_LEN];
    const configSTACK_DEPTH_TYPE stack_size;
    UBaseType_t priority;
} Task_Cfg_T;


/*******************************************************************************
* Prototypes
******************************************************************************/
/* Task function declarations */
static void Prox_Estimation_Task(void *pvParameters);
static void HTTP_Client_Task(void *pvParameters);

/* Local functions */
static void Init_OS_Tasks(void);

/*******************************************************************************
* Variables
******************************************************************************/
/* Task Configurations */
#define NUM_TASKS (2)
const Task_Cfg_T Task_Cfg_Table[NUM_TASKS] =
{
    /* Function,           Name,          Stack Size,  Priority */
    {Prox_Estimation_Task, "Prox Est",    1000,        configMAX_PRIORITIES - 2},
    {HTTP_Client_Task,     "HTTP_Client", 1000,        configMAX_PRIORITIES - 3}
};

static struct netif fsl_netif;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
int main(void)
{
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_InitBootPeripherals();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

#ifdef XBEE
    Init_Xbee_Interface();
#endif

    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    Init_Network_If(&fsl_netif);
    if (sys_thread_new("print_dhcp", Print_DHCP_State, &fsl_netif, PRINT_THREAD_STACKSIZE, PRINT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }

    Init_OS_Tasks();

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}

void Init_OS_Tasks(void)
{
    uint8_t i;

    printf("Initializing OS Tasks...\r\n");

    for (i=0; i<NUM_TASKS; i++)
    {
        if (xTaskCreate(Task_Cfg_Table[i].func, Task_Cfg_Table[i].name,
                        Task_Cfg_Table[i].stack_size, NULL, Task_Cfg_Table[i].priority, NULL) != pdPASS)
        {
            printf("Task number %d creation failed!.\r\n", i);
            assert(false);
        }
    }
}

static void Prox_Estimation_Task(void *pvParameters)
{
    while(1)
    {
#ifdef XBEE
        Run_Proximity_Estimation();
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void HTTP_Client_Task(void *pvParameters)
{
    struct dhcp *dhcp;

    while(1)
    {
        dhcp = netif_dhcp_data(&fsl_netif);

        if (netif_is_up(&fsl_netif) && DHCP_STATE_BOUND == dhcp->state)
        {
            Run_HTTP_Client();
        }

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
