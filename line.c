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

/* Global Constant Settings */
// Motor speed 0 -> 100, negative due to orientation
const int FORWARD_SPEED = 100; 
// Turning speed 0 -> 100, negative due to orientation
const int TURNING_SPEED = 100; 
// Minimum motor distance in degrees before firing the distance traveled event
const int STEP_DISTANCE = 222;
// Distance to object before obstacle detection event fires 
const int OBSTACLE_DIST =  5;

/* LEJOS OSEK hooks */
void ecrobot_device_initialize()
{
  nxt_motor_set_speed(NXT_PORT_A, 0, 1); 
  nxt_motor_set_speed(NXT_PORT_B, 0, 1); 
  ecrobot_init_sonar_sensor(NXT_PORT_S2);
  ecrobot_init_nxtcolorsensor(NXT_PORT_S1, NXT_COLORSENSOR);
}

void ecrobot_device_terminate()
{
  nxt_motor_set_speed(NXT_PORT_A, 0, 1); 
  nxt_motor_set_speed(NXT_PORT_B, 0, 1); 
  ecrobot_term_sonar_sensor(NXT_PORT_S2);
  ecrobot_term_nxtcolorsensor(NXT_PORT_S1);
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

/* EventDispatcher executed every 5ms */
TASK(EventDispatcher)
{
  // Static variables
  static int leftMotorCount_total  = 0;
  static int rightMotorCount_total = 0;
  static int leftMotorCount_old    = 0;
  static int rightMotorCount_old   = 0;
  static U16 color_old 			   = NXT_COLOR_UNKNOWN;

  // Local variables
  int sonar;
  int leftMotorCount;
  int rightMotorCount;
  int distance;
  U16 color;

  
  ecrobot_process_bg_nxtcolorsensor();
	
  // Pull values from sensors
  sonar = ecrobot_get_sonar_sensor(NXT_PORT_S2);
  leftMotorCount = nxt_motor_get_count(NXT_PORT_A);
  rightMotorCount = nxt_motor_get_count(NXT_PORT_B);
  color = ecrobot_get_nxtcolorsensor_id(NXT_PORT_S1);

  // Update the motor distances
  distance = leftMotorCount - leftMotorCount_old;
  leftMotorCount_total += distance * ((distance > 0) - (distance < 0));
  leftMotorCount_old = leftMotorCount;
  distance = rightMotorCount - rightMotorCount_old;
  rightMotorCount_total += distance * ((distance > 0) - (distance < 0));
  rightMotorCount_old = rightMotorCount;
	
  // Fire events as needed
  // Color Events
  if (color == NXT_COLOR_RED)
  {
    SetEvent(MainControlTask, FinishFoundEvent);
  }
  else if ((color_old == NXT_COLOR_BLACK || color_old == NXT_COLOR_BLUE) && color != NXT_COLOR_BLACK && color != NXT_COLOR_BLUE)
  {
    SetEvent(MainControlTask, LineLostEvent);
  }
  else if ((color_old != NXT_COLOR_BLACK && color_old != NXT_COLOR_BLUE) && (color == NXT_COLOR_BLACK || color == NXT_COLOR_BLUE))
  {
    SetEvent(MainControlTask, LineFoundEvent);
  }
  color_old = color;
  
  // Sonar Events
	if(sonar != -1 && sonar <= OBSTACLE_DIST)
	{
		SetEvent(MainControlTask, ObstacleDetectedEvent);
	}
	
  // Motor Events
  if (rightMotorCount_total >= STEP_DISTANCE)
  {
    SetEvent(MainControlTask, DistanceTraveledRightEvent);
    rightMotorCount_total = 0;
  }
  if (leftMotorCount_total >= STEP_DISTANCE)
  {
    SetEvent(MainControlTask, DistanceTraveledLeftEvent);
    leftMotorCount_total = 0;
  }
	
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
	nxt_motor_set_speed(NXT_PORT_B, -FORWARD_SPEED, 1);
	
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
	//turnLeft();
	//turnLeft();
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
	//turnRight();
	//turnRight();
}

void forwardUntilLine()
{
	ClearEvent(LineFoundEvent);
	moveForward();
	WaitEvent(LineFoundEvent);
}

int checkLeft()
{
	EventMaskType eventmask = 0;
	
	/*
	GetEvent(MainControlTask, &eventmask);
	if(eventmask & LineFoundEvent)
	{
		ClearEvent(LineFoundEvent);
		return 1;
	}
	*/
	
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
		return 1;
	}
	return 0;
}

int checkRight()
{
	EventMaskType eventmask = 0;
	
	/*)
	GetEvent(MainControlTask, &eventmask);
	if(eventmask & LineFoundEvent)
	{
		ClearEvent(LineFoundEvent);
		return 1;
	}
	*/
	
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
		return 1;
	}
	return 0;
}

void findLine()
{
	int i = 1;
	int j = 0;
	while(i < 10)
	{
		for(j = 0; j < i; j++)
		{
			if(checkLeft())
			{
				return;
			}
		}
		for(j = 0; j < i * 2; j++)
		{
			if(checkRight())
			{
				return;
			}
		}
		for(j = 0; j < i; j++)
		{
			if(checkLeft())
			{
				return;
			}
		}
		if(i < 3)
		{
			stepForward();
		}
		else
		{
			i++;
		}
		i++;
	}
	//If here is reached, no line was found. So do bad things like fly backwards maybe. 
	//stepBackward();
	//findLine();
}

void navigateBox()
{
	int i = 0;
	for(i = 0; i < 3; i++)
	{
		turnRight();
	}
	for(i = 0; i < 5; i++)
	{
		stepForward();
	}
	for(i = 0; i < 4; i++)
	{
		turnLeft();
	}
}


/* MainControlTask executed by OSEK Events */
TASK(MainControlTask)
{
	EventMaskType eventmask = 0;
	
	/*
	stepForward();
	stepBackward();
	turnLeft();
	stepForward();
	turnRight();
	stepBackward();
	turnLeft90();
	stepBackward();
	turnRight90();
	stepBackward();
	*/
	
	
	
	while(1)
	{
		moveForward();
		ClearEvent(LineLostEvent);
		WaitEvent(LineLostEvent /* | ObstacleDetectedEvent */);
		//GetEvent(MainControlTask, &eventmask);
		//if(eventmask & LineLostEvent)
		//{
			ClearEvent(LineLostEvent);
			stepForward();
			findLine();
		//}
		//else if(eventmask & ObstacleDetectedEvent)
		//{
		//	ClearEvent(ObstacleDetectedEvent);
		//	navigateBox();
		//}
	}
	
	
	
	//First must cross starting line
	//moveForward()
	//WaitEvent(FinishFoundEvent);
	moveForward();
	while(1)
	{
		//ClearEvent(FinishFoundEvent);
		//ClearEvent(LineLostEvent);
		WaitEvent(FinishFoundEvent | LineLostEvent /*| ObstacleDetectedEvent*/);
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
				//stepBackward();
				findLine();
				GetEvent(MainControlTask, &eventmask);
				if(eventmask & LineFoundEvent)
				{
					ClearEvent(LineFoundEvent);
					//Then we're at the dashed portion, and we're good to keep going forward. 
				}
				else
				{
					//stepBackward();
					findLine();
				}
			}
		}
		/*
		if(eventmask & ObstacleDetectedEvent)
		{
			//Frikkin turn 90 degrees!
			turnLeft90();
			stepForward();
			stepForward();
			stepForward();
			stepForward();
			stepForward();
			turnRight90();
			stepForward();
			stepForward();
			stepForward();
			stepForward();
			stepForward();
			turnRight90();
			forwardUntilLine();
			turnLeft90();
			findLine();
		}
		*/
		moveForward();
	}
	TerminateTask();
}

/* TaskLCD executed every 500ms */
TASK(TaskLCD)
{
	ecrobot_status_monitor("Line Follower");
	
	/*
	display_goto_xy(0,1);
	display_string("Color: ");
	switch(ecrobot_get_nxtcolorsensor_id(NXT_PORT_S1)) // get color number data
	{
		case NXT_COLOR_BLACK:
			display_string("BLACK");
			break;
		case NXT_COLOR_BLUE:
			display_string("BLUE");
			break;
		case NXT_COLOR_GREEN:
			display_string("GREEN");
			break;
		case NXT_COLOR_YELLOW:
			display_string("YELLOW");
			break;
		case NXT_COLOR_ORANGE:
			display_string("ORANGE");
			break;
		case NXT_COLOR_RED:
			display_string("RED");
			break;
		case NXT_COLOR_WHITE:
			display_string("WHITE");
			break;
		default:
			display_string("UNKNOWN COLOR");
			break;
	}
	*/
	
	
  TerminateTask();
}

