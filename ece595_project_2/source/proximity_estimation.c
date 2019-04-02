/*
 * proximity_estimation.c
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */
#include <stdio.h>
#include "proximity_estimation.h"
#include "xbee_interface.h"
#include "xbee_atcmd.h"
#include "xbee_api.h"
#include "assert.h"

Xbee_Serial_T Xbee_Serial_Port = {UART2, 9600};
static uint8_t Xbee_AT_Resp[RESP_BUF_SIZE];
static uint8_t RSSI;

void Run_Proximity_Estimation(void)
{
    static Tx_Status_T last_tx_status = NO_ACK_RXED;

    if (TX_SUCCESS == last_tx_status)
    {
        Xbee_Receive_Msg_Blocking((char *)Xbee_AT_Resp, sizeof(Xbee_AT_Resp), sizeof("ECHO"));
        printf("\r\n%s", Xbee_AT_Resp);
        RSSI = Get_Last_RSSI();
        printf("\r\nRSSI = -%d", RSSI);
    }

    last_tx_status = Xbee_Send_Msg_Blocking("PING!", sizeof("PING!")-1);
}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();
    Xbee_Serial_Open(&Xbee_Serial_Port);

    printf("Checking if Xbee API mode = 2...");
    Send_Xbee_AT_Cmd_Blocking(GET_API_MODE, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
    if ('2' != Xbee_AT_Resp[0])
    {
        printf("\r\nSetting Xbee API mode = 2...");
        /* Default to API mode */
        Enter_Xbee_API_Mode();
    }
    printf("\r\nDone!");

    printf("\r\nChecking if Xbee power level = lowest...");
    Send_Xbee_AT_Cmd_Blocking(GET_POWER_LEVEL, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
    if (LOWEST != Xbee_AT_Resp[0])
    {
        printf("\r\nSetting Xbee power level = lowest...");
        /* Default to lowest power mode */
        Change_Xbee_Power_Level_Blocking(LOWEST);
    }
    printf("\r\nDone!");
}

