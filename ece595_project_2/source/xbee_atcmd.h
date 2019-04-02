/*
 * xbee_atcmd.h
 *
 *  Created on: Mar 30, 2019
 *      Author: Devin
 */

#ifndef XBEE_ATCMD_H_
#define XBEE_ATCMD_H_

#define RESP_BUF_SIZE 15

typedef enum {
    GET_API_MODE = 0,
    GET_POWER_LEVEL = 1
} AT_CMD_T;

/* Note: These are ASCII 0-4 */
typedef enum {
    LOWEST  = 0x30,
    LOW     = 0x31,
    MEDIUM  = 0x32,
    HIGH    = 0x33,
    HIGHEST = 0x34
} Power_Level_T;

extern uint8_t Send_Xbee_AT_Cmd_Blocking(AT_CMD_T cmd, uint8_t* resp_buf, size_t resp_buf_size);
extern void Change_Xbee_Power_Level_Blocking(Power_Level_T level);
extern void Enter_Xbee_API_Mode(void);

#endif /* XBEE_ATCMD_H_ */
