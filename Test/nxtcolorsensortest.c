/* nxtcolorsensortest.c */
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"


#define PORT_ID NXT_PORT_S1

/* TOPPERS/ATK declarations */
DeclareCounter(SysTimerCnt);
DeclareAlarm(AlarmTask2);
DeclareEvent(EventTask2);

/* nxtOSEK hooks */
void ecrobot_device_initialize(void)
{
	ecrobot_init_nxtcolorsensor(PORT_ID, NXT_LIGHTSENSOR_NONE); // initialize a sensor
}

void ecrobot_device_terminate(void)
{
	ecrobot_term_nxtcolorsensor(PORT_ID); // terminate a sensor
}

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
	(void)SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
}

/* Alarm executed Task2 */
TASK(Task2)
{
	int count = 0;

	while(1)
	{
		if (count % 5 == 0)
		{
			WaitEvent(EventTask2);
	        ClearEvent(EventTask2);

			display_clear(0);
			display_goto_xy(0, 0);
			display_string("Welcome to my world!");
			display_goto_xy(0, 1);
			display_string("Name: Jay & James");
			display_goto_xy(0, 2);
			display_string("Light Sensor: ");
			display_int(ecrobot_get_nxtcolorsensor_light(PORT_ID), 0); // get light sensor data
			display_goto_xy(0, 3);
			display_string("Count:");
			display_int(count, 0);
			display_update();	
		}	

		count++;
		systick_wait_ms(100);
	}
}

/* Background Task */
TASK(Task1)
{
	SetRelAlarm(AlarmTask2, 1, 100); // set event for Task2 by Alarm
	while(1)
	{
		ecrobot_process_bg_nxtcolorsensor(); // communicates with NXT Color Sensor (this must be executed repeatedly in a background Task)
	}
}
