#ifndef LOCK_H_
#define LOCK_H_

extern void Run_Lock_Control(void);
extern uint8_t PORTC_IRQHandler(Lock_Method);

#endif /* LOCK_H_ */
