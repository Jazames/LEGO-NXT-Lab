/* helloworld.c for TOPPERS/ATK(OSEK) */
#include <stdlib.h> 
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void){ /* do nothing */ }

TASK(OSEK_Task_Background)
{
	while(1)
	{
  		//ecrobot_status_monitor("Okily Dokily");
		//Need to somehow get sensor information
		//Then need to display it. 
		ecrobot_process_bg_nxtcolorsensor(); // communicates with NXT Color Sensor (this must be executed repeatedly in a background Task)
		//systick_wait_ms(500); /* 500msec wait */
	}
}
