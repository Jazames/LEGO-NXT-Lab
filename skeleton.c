#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* You can define the ports used here */
/* #define COLOR_PORT_ID NXT_PORT_S1 */

DeclareTask(TestColorSensor);

/* nxtOSEK hooks */
void ecrobot_device_initialize() 
{
	//Light sensor initilization?
	//ecrobot_init_nxtcolorsensor(NXT_PORT_S1, NXT_LIGHTSENSOR_RED);
}
void ecrobot_device_terminate() {}
void user_1ms_isr_type2() {}

TASK(TestColorSensor)
{
	/* Your code */
	unsigned int i = 0;
	unsigned short data[5];
	int j = 0;
	unsigned int sum;
	char dataBuffer[500];
	char countBuffer[50];
	
	
	//Light sensor initilization?
	ecrobot_init_nxtcolorsensor(NXT_PORT_S1, NXT_LIGHTSENSOR_RED);
	
	while(true)
	{
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
			
			sprintf(dataBuffer, "Light Sensor: %d", sum);
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

