/*
 * xbee_api.h
 *
 *  Created on: Mar 31, 2019
 *      Author: Devin
 */

#ifndef XBEE_API_H_
#define XBEE_API_H_

#include <stdint.h>

void Xbee_Send_Msg(char *rf_data, uint8_t rf_data_len);

#endif /* XBEE_API_H_ */
