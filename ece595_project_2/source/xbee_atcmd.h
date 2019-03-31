/*
 * xbee_atcmd.h
 *
 *  Created on: Mar 30, 2019
 *      Author: Devin
 */

#ifndef XBEE_ATCMD_H_
#define XBEE_ATCMD_H_

typedef enum {
    GET_API_MODE = 0,
    GET_POWER_LEVEL = 1
} AT_CMD_T;

extern void Process_Xbee_AT_Cmd(void);
extern void Queue_Xbee_AT_Cmd(AT_CMD_T cmd);
uint8_t Xbee_AT_Cmd_Response_Rxed(void);
uint8_t * Get_Xbee_AT_Cmd_Response(void);

#endif /* XBEE_ATCMD_H_ */
