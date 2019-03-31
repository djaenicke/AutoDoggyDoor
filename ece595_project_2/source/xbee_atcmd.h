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

uint8_t Send_Xbee_AT_Cmd_Blocking(AT_CMD_T cmd, uint8_t* resp_buf, size_t resp_buf_size);

#endif /* XBEE_ATCMD_H_ */
