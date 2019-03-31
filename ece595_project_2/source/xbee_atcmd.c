/*
 * xbee_atcmd.c
 *
 *  Created on: Mar 30, 2019
 *      Author: Devin
 */

#include "stdio.h"
#include "xbee_interface.h"

typedef enum
{
    INACTIVE = 0,
    SENT_WAITING_ENTER,
    WAITING_ENTER,
    ACTIVE,
    WAITING_RESPONSE
} Cmd_Mode_State_T;

static Cmd_Mode_State_T Cmd_Mode_State = INACTIVE;
static uint8_t Exp_Res_Len = 0;

static uint8_t Res_Buffer[15];

void Enter_Xbee_AT_Cmd_Mode(void)
{
    if (Get_Time_Since_Last_Tx() > 1000)
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

void Exit_Xbee_AT_Cmd_Mode(void)
{
    /* Send request to exit command mode */

    Cmd_Mode_State = INACTIVE;
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

    }
    else if (WAITING_RESPONSE == Cmd_Mode_State)
    {

    }
    else
    {
        /* Unexpected mode state */
        assert(FALSE);
    }
}
