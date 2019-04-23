/*
 * proximity_estimation.h
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */

#ifndef PROXIMITY_ESTIMATION_H_
#define PROXIMITY_ESTIMATION_H_

typedef enum {
	FAR=0,
	CLOSE
} Proximity_Status_T;

extern Proximity_Status_T Get_Proximity_Status(void);
extern void Init_Xbee_Interface(void);
extern void Run_Proximity_Estimation(void);


#endif /* PROXIMITY_ESTIMATION_H_ */
