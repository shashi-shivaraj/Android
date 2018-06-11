/**************************************************************************************
*
*  FILE NAME	: sensors.h
*
*  DESCRIPTION  : header file
*
*  DATE	              NAME					 REFERENCE			REASON
*  
*  27-May-2018        Shashi Shivaraju       Initial code		AOSP 
*
**************************************************************************************/
/*Header Inclusions*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include <android/sensor.h>


/*Macro Definations*/
#define CHECK_VOID(X)	if(!X){ return -1;}
#define LOGFILE			"/sdcard/sensor_log.txt"
#define SENSOR_EVENT_RATE	50000


typedef struct __sensor_options
{
	ASensorManager* SensorManager;
	pthread_t		Thread_id;
	int				SensorType;
    int 			Thread_Count;
	unsigned int 	Rate; 		/*in microseconds*/
}sensor_options;


/*Function Prototypes*/
int  log_init();
void log_deinit();
ASensorManager* get_sensormanager();
int Get_Sensor_List(ASensorManager* SensorManager, ASensorList* list);
ASensor const* Get_Default_Sensor(ASensorManager* SensorManager,int type);
int Capture_Sensor_Data(ASensorManager* SensorManager);
int Stop_Sensor_Data();
