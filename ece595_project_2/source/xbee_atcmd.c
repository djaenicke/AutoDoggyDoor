/*
 * xbee_atcmd.c
 *
 *  Created on: Mar 30, 2019
 *      Author: Devin
 */

#include <stdio.h>
#include "xbee_interface.h"
#include "xbee_atcmd.h"

static uint8_t Req_Buffer[6];
static uint8_t Res_Buffer[RESP_BUF_SIZE];
static uint8_t Exp_Res_Len;

static void Enter_Xbee_AT_Cmd_Mode(void);
static void Exit_Xbee_AT_Cmd_Mode(void);
static void Verify_OK_Response(void);
static void Write_Parameter(void);
static void Apply_Changes(void);

static void Enter_Xbee_AT_Cmd_Mode(void)
{
    /* Wait for the bus to be silent for 1 s */
    while (Get_Time_Since_Last_Tx() < SILENCE_REQ_MS);

    /* Make sure the Xbee has had tome to reboot */
    while (Get_Time_Since_Xbee_Reboot() < XBEE_REBOOT_TIME);

    /* Send request to enter command mode */
    Xbee_Serial_Write("+++", sizeof("+++")-1);
    Exp_Res_Len = 3;
    Verify_OK_Response();
}

static void Exit_Xbee_AT_Cmd_Mode(void)
{
    /* Send request to exit command mode */
    Xbee_Serial_Write("ATCN\r", sizeof("ATCN\r")-1);
    Exp_Res_Len = 3;
    Verify_OK_Response();

}

static void Verify_OK_Response(void)
{
    while (Exp_Res_Len != Xbee_Serial_Rx_Used());

    Xbee_Serial_Read(Res_Buffer, Exp_Res_Len);

    /* Check the response */
    if (0 != strcmp("OK\r", (const char *)Res_Buffer))
    {
        /* Unknown response */
        assert(FALSE);
    }
}

static void Write_Parameter(void)
{
    /* Send request to write the parameter to Xbee memory */
    Xbee_Serial_Write("ATWR\r", sizeof("ATWR\r")-1);
    Exp_Res_Len = 3;
    Verify_OK_Response();
}

static void Apply_Changes(void)
{
    /* Send request to write the parameter to Xbee memory */
    Xbee_Serial_Write("ATAC\r", sizeof("ATAC\r")-1);
    Exp_Res_Len = 3;
    Verify_OK_Response();
}

uint8_t Send_Xbee_AT_Cmd_Blocking(AT_CMD_T cmd, uint8_t* resp_buf, size_t resp_buf_size)
{
    Enter_Xbee_AT_Cmd_Mode();

    /* Set up the constant portion of the request */
    Req_Buffer[0] = 'A';
    Req_Buffer[1] = 'T';
    Req_Buffer[4] = '\r';

    /* Build the command specific portion of the request */
    switch(cmd)
    {
        case GET_API_MODE:
            Req_Buffer[2] = 'A';
            Req_Buffer[3] = 'P';
            Exp_Res_Len = 2;
            break;
        case GET_POWER_LEVEL:
            Req_Buffer[2] = 'P';
            Req_Buffer[3] = 'L';
            Exp_Res_Len = 2;
            break;
        default:
            assert(FALSE);
    }

    if (Exp_Res_Len > resp_buf_size)
    {
        /* Provided buffer is not large enough */
        assert(FALSE);
    }

    Xbee_Serial_Write(Req_Buffer, 5);

    while (Exp_Res_Len != Xbee_Serial_Rx_Used());

    Xbee_Serial_Read(resp_buf, Exp_Res_Len);

    Exit_Xbee_AT_Cmd_Mode();

    return(Exp_Res_Len);
}

void Change_Xbee_Power_Level_Blocking(Power_Level_T level)
{
    Enter_Xbee_AT_Cmd_Mode();

    /* Set up the constant portion of the request */
    Req_Buffer[0] = 'A';
    Req_Buffer[1] = 'T';
    Req_Buffer[2] = 'P';
    Req_Buffer[3] = 'L';
    Req_Buffer[4] = level;
    Req_Buffer[5] = '\r';
    Exp_Res_Len = 3;

    Xbee_Serial_Write(Req_Buffer, 6);
    Verify_OK_Response();

    Write_Parameter();
    Apply_Changes();
    Start_Xbee_Reboot_Wait();
}

void Enter_Xbee_API_Mode(void)
{
    Enter_Xbee_AT_Cmd_Mode();

    /* Set up the constant portion of the request */
    Req_Buffer[0] = 'A';
    Req_Buffer[1] = 'T';
    Req_Buffer[2] = 'A';
    Req_Buffer[3] = 'P';
    Req_Buffer[4] = '2';
    Req_Buffer[5] = '\r';
    Exp_Res_Len = 3;

    Xbee_Serial_Write(Req_Buffer, 6);
    Verify_OK_Response();

    Write_Parameter();
    Apply_Changes();
    Start_Xbee_Reboot_Wait();
}

