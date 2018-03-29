#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* You can define the ports used here */
/* #define COLOR_PORT_ID NXT_PORT_S1 */


/* TOPPERS/ATK declarations */
DeclareCounter(SysTimerCnt);
DeclareAlarm(AlarmTask2);
DeclareEvent(EventTask2);
//DeclareTask(TestColorSensor);

/* nxtOSEK hooks */
void ecrobot_device_initialize() 
{
	//Light sensor initilization?
	ecrobot_init_nxtcolorsensor(NXT_PORT_S1, NXT_LIGHTSENSOR_NONE);
}

void ecrobot_device_terminate() 
{
	ecrobot_term_nxtcolorsensor(NXT_PORT_S1);
}

void user_1ms_isr_type2() 
{
	(void)SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
}

TASK(TestColorSensor)
{
	/* Your code */
	unsigned int i = 0;
	unsigned short data[5];
	int j = 0;
	unsigned int sum;
	char dataBuffer[500];
	char countBuffer[500];
	
	
	//Light sensor initilization?
	//ecrobot_init_nxtcolorsensor(NXT_PORT_S1, NXT_LIGHTSENSOR_RED);
	
	while(true)
	{
		//WaitEvent(EventTask2);
        //ClearEvent(EventTask2);
		
		//Whatever this line does, it needs to be called before getting the color sensor data. Supposedly it can run in the background, but I haven't seen that work. 
		ecrobot_process_bg_nxtcolorsensor();
		
		//Get sensor data and store;
		data[i%5] = ecrobot_get_nxtcolorsensor_light(NXT_PORT_S1);

	
		if((i % 5) == 0)
		{
			//Reset sum. 
			sum = 0;
			//Figure out average value.
			for(j = 0; j < 5; j++)
			{
				sum += data[j];
			}
			sum = sum/5;
			
			//sum = ecrobot_get_nxtcolorsensor_light(NXT_PORT_S1);
			
			sprintf(dataBuffer, "LightSensor:%d", sum);
			sprintf(countBuffer, "Count: %d", i);
			
			
			//It is the fifth iteration, time to update the display
			display_clear(0);
			display_goto_xy(0, 2);
			display_string("Welcome to My World!");
			display_goto_xy(0, 3);
			display_string("Name: Jay/James");
			display_goto_xy(0, 4);
			display_string(dataBuffer);
			display_goto_xy(0, 5);
			display_string(countBuffer);
			display_update();
			
			//Display should be:
			/*
			Welcome to My World!
			Name: <your name>
			Light Sensor: <average value since last update>
			Count: <sensor read count since start>
			*/
			
			
			//i = 0;//reset counter
		}
		i++;
		systick_wait_ms(100);
	}

	TerminateTask();
}


TASK(OSEK_Task_Background)
{
	//SetRelAlarm(AlarmTask2, 1, 100); // set event for Task2 by Alarm
	while(1)
	{
  		//ecrobot_status_monitor("Okily Dokily");
		//Need to somehow get sensor information
		//Then need to display it. 
		ecrobot_process_bg_nxtcolorsensor(); // communicates with NXT Color Sensor (this must be executed repeatedly in a background Task)
		//systick_wait_ms(500); /* 500msec wait */
	}
}


