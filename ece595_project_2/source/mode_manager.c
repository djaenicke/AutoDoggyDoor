#include "lock.h"
#include "mode_manager.h"
#include "task.h"

#define AUTO (0)
#define MANUAL (1)

void Run_Mode_Manager(void)
{
static uint8_t Lock_Method;
Lock_Method Get_Lock_Method(void);
if(Lock_Method == MANUAL)
{
	vTaskSuspend(&HTTPServer_Handle);

}
}
uint8_t Mode_Manager_Handle;
uint8_t Lock_Control_Handle;
uint8_t HTTPServer_Handle;
uint8_t Prox_Estimation_Handle;
