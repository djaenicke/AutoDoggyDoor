/*
 * app_supervisor.c
 *
 *  Created on: Apr 20, 2019
 *      Author: Devin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Application */
#include "tcpip_app_iface.h"
#include "http_server_app.h"
#include "http_client_app.h"
#include "proximity_estimation.h"
#include "lock.h"
#include "ultrasonic.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define XBEE

typedef enum {
    NOT_CONNECTED,
    CONNECTED
} Network_Status_T;

typedef enum {
    CONFIG,
    NORMAL
} App_Mode_T;

typedef struct Task_Cfg_Tag
{
    TaskFunction_t func;
    const char name[configMAX_TASK_NAME_LEN];
    const configSTACK_DEPTH_TYPE stack_size;
    UBaseType_t priority;
    TaskHandle_t handle;
} Task_Cfg_T;

/*******************************************************************************
* Prototypes
******************************************************************************/
/* Task function declarations */
static void Supervisor_Task(void *pvParameters);

static void Prox_Estimation_Task(void *pvParameters);
static void HTTP_Client_Task(void *pvParameters);
static void Lock_Control_Task(void *pvParameters);
static void Detect_Pet_Task(void *pvParameters);
static void IoT_Task(void *pvParameters);

/* Task Configurations */
#define NUM_TASKS (4)
#define SUPERVISOR_PRIORITY configMAX_PRIORITIES-5

Task_Cfg_T Periodic_Tasks_Table[NUM_TASKS] =
{
    /* Function,           Name,          Stack Size,  Priority */
	{IoT_Task,			   "IoT",         100,         configMAX_PRIORITIES - 5, NULL},
	{Detect_Pet_Task,	   "Detect_Pet",  100,         configMAX_PRIORITIES - 4, NULL},
    {Lock_Control_Task,    "Lock Ctrl",   100,         configMAX_PRIORITIES - 3, NULL},
    {HTTP_Client_Task,     "HTTP_Client", 1000,        configMAX_PRIORITIES - 2, NULL},
};


/* Local functions */
static void Create_Periodic_OS_Tasks(void);

/*******************************************************************************
* Variables
******************************************************************************/
static struct netif FSL_NetIf;
static Network_Status_T Network_Status = NOT_CONNECTED;
static App_Mode_T App_Mode = CONFIG;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
void Start_App_Supervisor(void)
{
    /* Run application initialization */
#ifdef XBEE
    Init_Xbee_Interface();
#endif
    Init_Network_If(&FSL_NetIf);

    Create_Periodic_OS_Tasks();
    xTaskCreate(Supervisor_Task, "Supervisor", 500, NULL, SUPERVISOR_PRIORITY, NULL);
    vTaskStartScheduler();
}

static void Supervisor_Task(void *pvParameters)
{
    struct dhcp *dhcp;
    uint8_t lock_method;

    while(1)
    {
        /* Check the network connection status */
        Print_DHCP_State(&FSL_NetIf);
        dhcp = netif_dhcp_data(&FSL_NetIf);

        if (netif_is_up(&FSL_NetIf) && DHCP_STATE_BOUND == dhcp->state)
        {
            if (NOT_CONNECTED == Network_Status && CONFIG == App_Mode)
            {
                /* Start the HTTP Server */
                HTTP_Server_Socket_Init();
            }

            Network_Status = CONNECTED;
        }
        else
        {
            Network_Status = NOT_CONNECTED;
        }

        /* Check if the user has changed the app mode */
        lock_method = Get_Lock_Method();

        if ((AUTO == lock_method) && (CONFIG == App_Mode))
        {
            /* Switching from config to normal */
            App_Mode = NORMAL;
            printf("The lock is now being controlled automatically.\n\r");

            /* Start the proximity estimation task now */
            xTaskCreate(Prox_Estimation_Task, "Prox Est", 1000, NULL, configMAX_PRIORITIES - 1, NULL);
        }
        else if ((MANUAL == lock_method) && (NORMAL == App_Mode))
        {
            App_Mode = CONFIG;
            printf("The lock is now being controlled manually.\n\r");
        }
        else
        {
            /* Already in the right mode */
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void Prox_Estimation_Task(void *pvParameters)
{
    while(1)
    {
#ifdef XBEE
        Run_Proximity_Estimation();
#endif

        if (NORMAL == App_Mode)
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        else
        {
            /* Kill the proximity estimation task so that the HTTP server isn't starved */
            vTaskDelete(NULL);
        }
    }
}

static void Lock_Control_Task(void *pvParameters)
{
    while(1)
    {
        Run_Lock_Control();
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

static void HTTP_Client_Task(void *pvParameters)
{
    struct dhcp *dhcp;

    while(1)
    {
        dhcp = netif_dhcp_data(&FSL_NetIf);

        if (netif_is_up(&FSL_NetIf) && DHCP_STATE_BOUND == dhcp->state)
        {
            Run_HTTP_Client();
        }

        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}


void Create_Periodic_OS_Tasks(void)
{
    uint8_t i;
    for (i=0; i<NUM_TASKS; i++)
    {
        if (xTaskCreate(Periodic_Tasks_Table[i].func, Periodic_Tasks_Table[i].name, \
                        Periodic_Tasks_Table[i].stack_size, NULL, Periodic_Tasks_Table[i].priority, \
                        &Periodic_Tasks_Table[i].handle) != pdPASS)
        {
            printf("Task number %d creation failed!.\r\n", i);
            assert(false);
        }
    }
}

static void Detect_Pet_Task(void *pvParameters)
{
	Detect_Pet();
	vTaskDelay(pdMS_TO_TICKS(200));
}

static void IoT_Task(void *pvParameters)
{
    Run_IoT_Logging();
	vTaskDelay(pdMS_TO_TICKS(3000));
}
