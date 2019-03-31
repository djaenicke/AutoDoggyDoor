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

static uint8_t Req_Buffer[5];
static uint8_t Res_Buffer[RESP_BUF_SIZE];
static uint8_t Exp_Res_Len;

static void Enter_Xbee_AT_Cmd_Mode(void);
static void Exit_Xbee_AT_Cmd_Mode(void);

static void Enter_Xbee_AT_Cmd_Mode(void)
{
    /* Wait for the bus to be silent for 1 s */
    while (Get_Time_Since_Last_Tx() < SILENCE_REQ_MS);

    /* Send request to enter command mode */
    Xbee_Serial_Write("+++", sizeof("+++")-1);
    Exp_Res_Len = 3;

    /* Wait for the response */
    while (Exp_Res_Len != Xbee_Serial_Rx_Used());

    Xbee_Serial_Read(Res_Buffer, Exp_Res_Len);

    /* Check the response */
    if (0 != strcmp("OK\r", (const char *)Res_Buffer))
    {
        /* Unknown response */
        assert(FALSE);
    }
}

static void Exit_Xbee_AT_Cmd_Mode(void)
{
    /* Send request to exit command mode */
    Xbee_Serial_Write("ATCN\r", sizeof("ATCN\r")-1);
    Exp_Res_Len = 3;

    /* Wait for the response */
    while (Exp_Res_Len != Xbee_Serial_Rx_Used());

    Xbee_Serial_Read(Res_Buffer, Exp_Res_Len);

    /* Check the response */
    if (0 != strcmp("OK\r", (const char *)Res_Buffer))
    {
        /* Unknown response */
        assert(FALSE);
    }
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

    Xbee_Serial_Write(Req_Buffer, sizeof(Req_Buffer));

    while (Exp_Res_Len != Xbee_Serial_Rx_Used());

    Xbee_Serial_Read(resp_buf, Exp_Res_Len);

    Exit_Xbee_AT_Cmd_Mode();

    return(Exp_Res_Len);
}

