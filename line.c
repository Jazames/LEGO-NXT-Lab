#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* Task declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(EventDispatcher); 
DeclareTask(MainControlTask);
DeclareTask(TaskLCD);

/* Event Declarations */
DeclareEvent(LineLostEvent);
DeclareEvent(LineFoundEvent);
DeclareEvent(FinishFoundEvent);
DeclareEvent(ObstacleDetectedEvent);
DeclareEvent(DistanceTraveledLeftEvent);
DeclareEvent(DistanceTraveledRightEvent);


/* Global Constants */
const int FORWARD_SPEED = -50;
const int TURNING_SPEED = -30;


/* LEJOS OSEK hooks */
void ecrobot_device_initialize()
{
  nxt_motor_set_speed(NXT_PORT_A, 0, 1); 
  nxt_motor_set_speed(NXT_PORT_B, 0, 1); 
  ecrobot_init_sonar_sensor(NXT_PORT_S4);
}

void ecrobot_device_terminate()
{
  nxt_motor_set_speed(NXT_PORT_A, 0, 1); 
  nxt_motor_set_speed(NXT_PORT_B, 0, 1); 
  ecrobot_term_sonar_sensor(NXT_PORT_S4);
}

/* LEJOS OSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
  StatusType ercd;

  ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
  if(ercd != E_OK)
  {
    ShutdownOS(ercd);
  }
}

/* EventDispatcher executed every 50ms */
TASK(EventDispatcher)
{
  static int sonar_old = 0;
  int sonar;
	
	//countA = nxt_motor_get_count(NXT_PORT_A);
	//countB = nxt_motor_get_count(NXT_PORT_B);
	
	
	
	sonar = ecrobot_get_sonar_sensor(NXT_PORT_S4);
	TouchSensorStatus = ecrobot_get_touch_sensor(NXT_PORT_S1);
	
	if(sonar != -1)
	{
		sonar_old = sonar;
	}
	
	if (TouchSensorStatus == 1 && TouchSensorStatus_old == 0 && sonar_old <= 10)
	{
		/* Send a Touch Sensor ON Event to the Handler */ 
		SetEvent(MotorControlTask, StartMotorEvent);
	}
	else if ((TouchSensorStatus == 0 && TouchSensorStatus_old == 1) || sonar_old > 10)
	{
		/* Send a Touch Sensor OFF Event to the Handler */ 
		SetEvent(MotorControlTask, StopMotorEvent);
	}
	/*
	if (sonar_old > 10)
	{
		SetEvent(MotorControlTask, StopMotorEvent); 
	}
	*/
	
	
	//sonar_old = sonar;
	TouchSensorStatus_old = TouchSensorStatus;
	
	TerminateTask();
}

/*Functions for different motor control schemes*/
void moveForward()
{
	nxt_motor_set_speed(NXT_PORT_A, FORWARD_SPEED, 1);
    nxt_motor_set_speed(NXT_PORT_B, FORWARD_SPEED, 1);
}

void halt()
{
	nxt_motor_set_speed(NXT_PORT_A, 0, 1);
	nxt_motor_set_speed(NXT_PORT_B, 0, 1);
}

void stepForward()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	moveForward();
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent);
	GetEvent(MainControlTask, &eventmask);
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		WaitEvent(DistanceTraveledRightEvent);
	}
	else
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		WaitEvent(DistanceTraveledLeftEvent);
	}
	halt();
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
}

void stepBackward()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	//Send the motors backward. 
	nxt_motor_set_speed(NXT_PORT_A, -FORWARD_SPEED, 1);
	nxt_motor_set_speed(NXT_PORT_b, -FORWARD_SPEED, 1);
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent);
	GetEvent(MainControlTask, &eventmask);
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		WaitEvent(DistanceTraveledRightEvent);
	}
	else
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		WaitEvent(DistanceTraveledLeftEvent);
	}
	halt();
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
}

/*
  SetEvent(MainControlTask, LineLostEvent);
  SetEvent(MainControlTask, LineFoundEvent);
  SetEvent(MainControlTask, FinishFoundEvent);
  SetEvent(MainControlTask, ObstacleDetectedEvent);
  SetEvent(MainControlTask, DistanceTraveledLeftEvent);
  SetEvent(MainControlTask, DistanceTraveledRightEvent);
*/

void turnLeft()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	nxt_motor_set_speed(NXT_PORT_A, -TURNING_SPEED, 1);
	nxt_motor_set_speed(NXT_PORT_B,  TURNING_SPEED, 1);
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent);
	GetEvent(MainControlTask, &eventmask);
	
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
		WaitEvent(DistanceTraveledRightEvent);
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
	}
	else if(eventmask & DistanceTraveledRightEvent)
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
		WaitEvent(DistanceTraveledLeftEvent);
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
	}
	halt();
}

void turnLeft90()
{
	turnLeft();
	turnLeft();
	turnLeft();
	turnLeft();
}

void turnRight()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	nxt_motor_set_speed(NXT_PORT_A,  TURNING_SPEED, 1);
	nxt_motor_set_speed(NXT_PORT_B, -TURNING_SPEED, 1);
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent);
	GetEvent(MainControlTask, &eventmask);
	
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
		WaitEvent(DistanceTraveledRightEvent);
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
	}
	else if(eventmask & DistanceTraveledRightEvent)
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
		WaitEvent(DistanceTraveledLeftEvent);
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
	}
	halt();
}

void turnRight90()
{
	turnRight();
	turnRight();
	turnRight();
	turnRight();
}

bool checkLeft()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(LineFoundEvent);
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	nxt_motor_set_speed(NXT_PORT_A, -TURNING_SPEED, 1);
	nxt_motor_set_speed(NXT_PORT_B,  TURNING_SPEED, 1);
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent | LineFoundEvent);
	GetEvent(MainControlTask, &eventmask);
	
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
		WaitEvent(DistanceTraveledRightEvent);
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
	}
	else if(eventmask & DistanceTraveledRightEvent)
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
		WaitEvent(DistanceTraveledLeftEvent);
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
	}
	halt();
	if(eventmask & LineFoundEvent)//Line was found
	{
		ClearEvent(LineFoundEvent);
		return true;
	}
	return false;
}

bool checkRight()
{
	EventMaskType eventmask = 0;
	
	ClearEvent(LineFoundEvent);
	ClearEvent(DistanceTraveledLeftEvent);
	ClearEvent(DistanceTraveledRightEvent);
	
	nxt_motor_set_speed(NXT_PORT_A,  TURNING_SPEED, 1);
	nxt_motor_set_speed(NXT_PORT_B, -TURNING_SPEED, 1);
	
	WaitEvent(DistanceTraveledLeftEvent | DistanceTraveledRightEvent | LineFoundEvent);
	GetEvent(MainControlTask, &eventmask);
	
	if(eventmask & DistanceTraveledLeftEvent)
	{
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
		WaitEvent(DistanceTraveledRightEvent);
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
	}
	else if(eventmask & DistanceTraveledRightEvent)
	{
		nxt_motor_set_speed(NXT_PORT_B, 0, 1);
		ClearEvent(DistanceTraveledRightEvent);
		WaitEvent(DistanceTraveledLeftEvent);
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
		ClearEvent(DistanceTraveledLeftEvent);
	}
	halt();
	if(eventmask & LineFoundEvent)//Line was found
	{
		ClearEvent(LineFoundEvent);
		return true;
	}
	return false;
}

void findLine()
{
	if(checkLeft())
	{
		return;
	}
	if(checkRight())
	{
		return;
	}
	if(checkRight())
	{
		return;
	}
	//If here is reached, no line was found. So do bad things like fly backwards maybe. 
}


/* MainControlTask executed by OSEK Events */
TASK(MainControlTask)
{
	EventMaskType eventmask = 0;
	//First must cross starting line
	//moveForward()
	//WaitEvent(FinishFoundEvent);
	moveForward();
	while(1)
	{
		WaitEvent(FinishFoundEvent | LineLostEvent | ObstacleDetectedEvent);
		GetEvent(MainControlTask, &eventmask);
		if(eventmask & FinishFoundEvent)
		{
			ClearEvent(FinishFoundEvent);
			stepForward();
			stepForward();
			halt();
			TerminateTask();
		}
		if(eventmask & LineLostEvent)
		{
			ClearEvent(LineLostEvent);
			stepForward();
			GetEvent(MainControlTask, &eventmask);
			if(eventmask & LineFoundEvent)
			{
				ClearEvent(LineFoundEvent);
				//Then we're at the dashed portion, and we're good to keep going forward. 
			}
			else
			{
				//May perhaps want to try to find the line before backing up?
				
				//Back up and reorient.  
				stepBackward();
				findLine();
			}
		}
		if(eventmask & ObstacleDetectedEvent)
		{
			//Frikkin turn 90 degrees!
			
		}
		
	}
	
	TerminateTask();
}

/* TaskLCD executed every 500ms */
TASK(TaskLCD)
{
  ecrobot_status_monitor("EDS");

	
  TerminateTask();
}

