/* eds.c */ 
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"



/* OSEK declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(EventDispatcher); 
DeclareTask(MotorControlTask);
DeclareTask(TaskLCD);

DeclareEvent(StartMotorEvent); /* Event declaration */
DeclareEvent(StopMotorEvent); /* Event declaration */ 

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
  static U8 TouchSensorStatus_old = 0;
  static int sonar_old = 0;
  U8 TouchSensorStatus; 
  int sonar;
	
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

/* MotorControlTask executed by OSEK Events */
TASK(MotorControlTask)
{
  while(1)
  {
	ClearEvent(StartMotorEvent);
    WaitEvent(StartMotorEvent); /* Task is in waiting status until the Event comes */ 
    ClearEvent(StartMotorEvent);
    nxt_motor_set_speed(NXT_PORT_A, 50, 1);
    nxt_motor_set_speed(NXT_PORT_B, 50, 1);

	ClearEvent(StopMotorEvent);
    WaitEvent(StopMotorEvent); /* Task is in waiting status until the Event comes */
    ClearEvent(StopMotorEvent);
    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
    nxt_motor_set_speed(NXT_PORT_B, 0, 1);
  }

  TerminateTask();
}

/* TaskLCD executed every 500ms */
TASK(TaskLCD)
{
  ecrobot_status_monitor("EDS");

	
  TerminateTask();
}

