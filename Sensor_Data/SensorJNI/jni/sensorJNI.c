/*******************************************************************************************************
*
*  FILE NAME	: sensorsJNI.c
*
*  DESCRIPTION  : Java native interface for java SensorApp.
*
*  DATE	               NAME					 REFERENCE			REASON
*  27-May-2018         Shashi Shivaraju      Initial code		AOSP
*******************************************************************************************************/
#include <jni.h>
#include <syslog.h>
#include "sensors.h"

/* Global declaration */
ASensorManager* gSensorManger = NULL;

jint Java_com_example_shashishivaraju_sensorapp_MainActivity_init_1logging (
        JNIEnv *env,
        jobject thisObj) {
			
	int ret = 0;
	ret = log_init();
	if(-1 == ret )
		syslog(LOG_CRIT, "log_init failed");
		
	return ret;

    //return (*env)->NewStringUTF(env, "Hello from native code!");
}

jint Java_com_example_shashishivaraju_sensorapp_MainActivity_check_1sensor_1capabilities(
		JNIEnv *env,
        jobject thisObj) {

	
	ASensorList	SensorList = NULL;  
	ASensorRef Accl_Sensor = NULL;
	ASensorRef Gyro_Sensor = NULL;
	 		
	int ret = 0;
	
	/*sensor manager handle*/
	 gSensorManger = get_sensormanager();
	 if(!gSensorManger)
	 {
		 syslog(LOG_CRIT, "SensorManger Not Supported\n");
		 return -1;
	 }
	 
	 /*Get list of available sensors*/
	 ret = Get_Sensor_List(gSensorManger,&SensorList);
	 if(ret)
	 {
	   syslog(LOG_CRIT, "Device supports %d Sensors",ret);
     }
     else
     {
		syslog(LOG_CRIT, "No sensors supported");
		return -1;
	 }
	 	
	 /*Get the Default Accelorometer sensor available*/
	 Accl_Sensor = Get_Default_Sensor(gSensorManger,ASENSOR_TYPE_ACCELEROMETER);
	 if(!Accl_Sensor)
	 {
		  syslog(LOG_CRIT, "Accelorometer NOT supported");
		  return -1;
	 }
	 
	 /*Get the Default Gyroscope sensor available*/
	 Gyro_Sensor = Get_Default_Sensor(gSensorManger,ASENSOR_TYPE_GYROSCOPE);
	 if(!Gyro_Sensor)
	 {
		  syslog(LOG_CRIT, "Gyroscope NOT supported");
		  return -1;
	 }
	 
	 return 0;
}

jint Java_com_example_shashishivaraju_sensorapp_MainActivity_capture_1sensor_1data (
		JNIEnv *env,
        jobject thisObj) {
		
	int ret = 0;
	
	ret = Capture_Sensor_Data(gSensorManger);
	if(0 != ret)
	{
		 syslog(LOG_CRIT, "Capture_Sensor_Data failed %d\n",ret);
		 return -1;
	}
	
	return ret;			
}

jint Java_com_example_shashishivaraju_sensorapp_MainActivity_stop_1sensors (
		JNIEnv *env,
        jobject thisObj) {
			
	int ret = 0;
	
	ret = Stop_Sensor_Data(gSensorManger);
	if(0 != ret)
	{
		 syslog(LOG_CRIT, "Stop_Sensor_Data failed %d\n",ret);
		 return -1;
	}
	
	return ret;
		
}
		
		

void Java_com_example_shashishivaraju_sensorapp_MainActivity_deinit_1logging (
		JNIEnv *env,
        jobject thisObj) {
	
	 syslog(LOG_INFO, "log_deinit called");
	 log_deinit();
}
