/*
 * xbee_atcmd.c
 *
 *  Created on: Mar 30, 2019
 *      Author: Devin
 */

#include <stdio.h>
#include "xbee_interface.h"
#include "xbee_atcmd.h"

#define SILENCE_REQ_MS (1000)

#define MAX_REQ_LEN    5
#define MAX_RES_LEN   15

typedef enum
{
    INACTIVE = 0,
    SENT_WAITING_ENTER,
    WAITING_ENTER,
    ACTIVE
} Cmd_Mode_State_T;

typedef enum
{
    UNKNOWN = 0,
    NOT_RXED,
    RXED
} Resp_Status_T;

typedef enum
{
    READY,
    SENT
} Req_Status_T;

typedef struct
{
    uint8_t req[MAX_REQ_LEN];
    uint8_t resp[MAX_RES_LEN];
    uint8_t exp_res_len;
    Resp_Status_T resp_status;
    Req_Status_T req_status;
} Cmd_T;

static uint8_t Res_Buffer[MAX_RES_LEN];
static uint8_t Exp_Res_Len = 3;

static Cmd_Mode_State_T Cmd_Mode_State = INACTIVE;
static Cmd_T Current_Cmd;

static void Enter_Xbee_AT_Cmd_Mode(void);
static void Exit_Xbee_AT_Cmd_Mode(void);

static void Enter_Xbee_AT_Cmd_Mode(void)
{
    if (Get_Time_Since_Last_Tx() > SILENCE_REQ_MS)
    {
        /* Send request to enter command mode */
        Xbee_Serial_Write("+++", sizeof("+++")-1);
        Exp_Res_Len = 3;
        Cmd_Mode_State = SENT_WAITING_ENTER;
    }
    else
    {
        /* Need to wait until the bus has been silent for 1s */
        Cmd_Mode_State = WAITING_ENTER;
    }
}

static void Exit_Xbee_AT_Cmd_Mode(void)
{
    /* Send request to exit command mode */
    Xbee_Serial_Write("ATCN\r", sizeof("ATCN\r")-1);
    Cmd_Mode_State = INACTIVE;
    Current_Cmd.req_status = UNKNOWN;
    Current_Cmd.resp_status = UNKNOWN;
    printf("AT Command Mode Inactive");
}

void Process_Xbee_AT_Cmd(void)
{
    if (INACTIVE == Cmd_Mode_State)
    {
        return;
    }
    else if (WAITING_ENTER == Cmd_Mode_State)
    {
        Enter_Xbee_AT_Cmd_Mode();
    }
    else if (SENT_WAITING_ENTER == Cmd_Mode_State)
    {
        /* Look for a response */
        if (Exp_Res_Len == Xbee_Serial_Rx_Used())
        {
            Xbee_Serial_Read(Res_Buffer, Exp_Res_Len);
            if (0 == strcmp("OK\r", (const char *)Res_Buffer))
            {
                Cmd_Mode_State = ACTIVE;
                printf("AT Command Mode Active");
            }
            else
            {
                assert(FALSE);
            }
        }
    }
    else if (ACTIVE == Cmd_Mode_State)
    {
        if (READY == Current_Cmd.req_status)
        {
            Xbee_Serial_Write(Current_Cmd.req, sizeof(Current_Cmd.req));
            Current_Cmd.req_status = SENT;
        }
        else if (NOT_RXED == Current_Cmd.resp_status)
        {
            if (Current_Cmd.exp_res_len == Xbee_Serial_Rx_Used())
            {
                Xbee_Serial_Read(Current_Cmd.resp, Current_Cmd.exp_res_len);
                Current_Cmd.resp_status = RXED;
            }
        }
        else
        {
            Exit_Xbee_AT_Cmd_Mode();
        }
    }
    else
    {
        /* Unexpected mode state */
        assert(FALSE);
    }
}

void Queue_Xbee_AT_Cmd(AT_CMD_T cmd)
{
    if (INACTIVE == Cmd_Mode_State)
    {
        Enter_Xbee_AT_Cmd_Mode();
    }
    else
    {
        /* Invalid usage */
        assert(FALSE);
    }

    /* Set up the constant portion of the request */
    Current_Cmd.req[0] = 'A';
    Current_Cmd.req[1] = 'T';
    Current_Cmd.req[4] = '\r';

    /* Reset the data structure */
    memset(Current_Cmd.resp, 0, sizeof(Current_Cmd.resp));
    Current_Cmd.resp_status = NOT_RXED;
    Current_Cmd.req_status = READY;

    /* Build the command specific portion of the request */
    switch(cmd)
    {
        case GET_API_MODE:
            Current_Cmd.req[2] = 'A';
            Current_Cmd.req[3] = 'P';
            Current_Cmd.exp_res_len = 2;
            break;
        case GET_POWER_LEVEL:
            Current_Cmd.req[2] = 'P';
            Current_Cmd.req[3] = 'L';
            Current_Cmd.exp_res_len = 2;
            break;
        default:
            assert(FALSE);
    }
}

uint8_t Xbee_AT_Cmd_Response_Rxed(void)
{
    return RXED == Current_Cmd.resp_status ? TRUE : FALSE;
}

uint8_t * Get_Xbee_AT_Cmd_Response(void)
{
    return(Current_Cmd.resp);
}


