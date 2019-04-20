/*
 * app_supervisor.c
 *
 *  Created on: Apr 20, 2019
 *      Author: Devin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/* FreeRTOS kernel includes. */
#include <http_server_app.h>
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
} Task_Cfg_T;

/*******************************************************************************
* Prototypes
******************************************************************************/
/* Task function declarations */
static void Supervisor_Task(void *pvParameters);

static void Prox_Estimation_Task(void *pvParameters);
static void HTTP_Client_Task(void *pvParameters);
static void Lock_Control_Task(void *pvParameters);

/* Task Configurations */
#define NUM_TASKS (3)
#define SUPERVISOR_PRIORITY configMAX_PRIORITIES-4

const Task_Cfg_T Task_Cfg_Table[NUM_TASKS] =
{
    /* Function,           Name,          Stack Size,  Priority */
    {Prox_Estimation_Task, "Prox Est",    1000,        configMAX_PRIORITIES - 1},
    {Lock_Control_Task,    "Lock Ctrl",   100,         configMAX_PRIORITIES - 2},
    {HTTP_Client_Task,     "HTTP_Client", 1000,        configMAX_PRIORITIES - 3}
};


/* Local functions */
//static void Init_OS_Tasks(void);

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
            // TODO - Stop HTTP server
            // TODO - Start other tasks
            App_Mode = NORMAL;
            printf("The lock is now being controlled automatically.\n\r");
        }
        else if ((MANUAL == lock_method) && (NORMAL == App_Mode))
        {
            /* Switching from normal to config */
            // TODO - Stop other tasks
            // TODO - Start HTTP server
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
        vTaskDelay(pdMS_TO_TICKS(1000));
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

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

/*
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
*/
