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

#define DETECT_PET_PRIO    2
#define SUPERVISOR_PRIO    12
#define LOCK_CONTROL_PRIO  13
#define PROX_EST_PRIO      14
#define HTTP_CLIENT_PRIO   15
#define IOT_PRIO           16

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

    xTaskCreate(HTTP_Client_Task,  "HTTP_Client", 1000, NULL, HTTP_CLIENT_PRIO, NULL);
    xTaskCreate(Supervisor_Task,   "Supervisor",  500,  NULL, SUPERVISOR_PRIO,  NULL);
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
            xTaskCreate(Prox_Estimation_Task,  "Prox Est",    1000, NULL, PROX_EST_PRIO,     NULL);
            xTaskCreate(IoT_Task,              "IoT",          100, NULL, IOT_PRIO,          NULL);
            xTaskCreate(Detect_Pet_Task,       "Detect_Pet",  2000, NULL, DETECT_PET_PRIO,   NULL);
            xTaskCreate(Lock_Control_Task,     "Lock_Ctrl",   2000, NULL, LOCK_CONTROL_PRIO, NULL);
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

        if (NORMAL == App_Mode)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            /* Kill the lock control task so that the HTTP server isn't starved */
            vTaskDelete(NULL);
        }
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

static void Detect_Pet_Task(void *pvParameters)
{
	while(1)
	{
        Detect_Pet();

        if (NORMAL == App_Mode)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else
        {
            /* Kill the detect pet task so that the HTTP server isn't starved */
            vTaskDelete(NULL);
        }
	}
}

static void IoT_Task(void *pvParameters)
{
    struct dhcp *dhcp;

    while(1)
    {
        if (NORMAL == App_Mode)
        {
            dhcp = netif_dhcp_data(&FSL_NetIf);

            if (netif_is_up(&FSL_NetIf) && DHCP_STATE_BOUND == dhcp->state)
            {
                Run_IoT_Logging();
            }
        }
        else
        {
            /* Kill the IoT task so that the HTTP server isn't starved */
            vTaskDelete(NULL);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
