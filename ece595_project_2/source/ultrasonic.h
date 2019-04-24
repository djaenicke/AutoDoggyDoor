/*
 * ultrasonic.h
 *
 *  Created on: Apr 19, 2019
 *      Author: Lacy
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

typedef enum
{
	INSIDE = 0,
	OUTSIDE
} Dog_Status_T;

extern void Detect_Pet(void);
extern Dog_Status_T Get_Dog_Status(void);

#endif /* ULTRASONIC_H_ */
