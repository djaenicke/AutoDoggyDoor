#ifndef LOCK_H_
#define LOCK_H_

#define AUTO (0)
#define MANUAL (1)

extern void Run_Lock_Control(void);
extern uint8_t Get_Lock_Method(void);

#endif /* LOCK_H_ */
