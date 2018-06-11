/*******************************************************************************************************
*
*  FILE NAME	: sensors.c
*
*  DESCRIPTION  : API's to capture sensor data.
*
*  DATE	               NAME					 REFERENCE			REASON
*  27-May-2018         Shashi Shivaraju      Initial code		AOSP
*******************************************************************************************************/

/*Header inclusion*/
#include "sensors.h"
//#include <syslog.h>

/*global declarations*/
FILE *g_logfp = NULL;
sensor_options *accl_opt = NULL;
sensor_options *gyro_opt = NULL;

unsigned int gCapture = 0; 
static void* SensorThread(void* aoptions);

int log_init()
{
	if(g_logfp)
		return 0;

	g_logfp = fopen("/sdcard/sensor_log.txt","w");
	if(!g_logfp)
		return -1;
		
	fprintf(g_logfp,"*********START OF LOG********\n");
	
	fprintf(g_logfp,"\nAPI:log_init exit\n\n");
	return 0;
}

void log_deinit()
{
	if(g_logfp)
	{
		fprintf(g_logfp,"API:log_deinit enter\n\n");
		fprintf(g_logfp,"*********END OF LOG***********\n");
		fclose(g_logfp);
	}
	g_logfp = NULL;
}

ASensorManager* get_sensormanager()
{
	ASensorManager* SensorManager = NULL;
	
	fprintf(g_logfp,"API:get_sensormanager enter\n\n");
	
	SensorManager = ASensorManager_getInstance();
	
	fprintf(g_logfp,"SensorManager = %p\n",SensorManager);
	
	fprintf(g_logfp,"\nAPI:get_sensormanager exit\n\n");
	
	return SensorManager;	
}

ASensor const* Get_Default_Sensor(ASensorManager* SensorManager,int type)
{
	ASensor const* Sensor = NULL;
	
	fprintf(g_logfp,"API:Get_Default_Sensor enter\n\n");
	
	Sensor = ASensorManager_getDefaultSensor(SensorManager,type);
    if (Sensor) 
    {
		fprintf(g_logfp,"sensor:%s, vendor: %s , min-delay: %d, Type: %s Pointer = %p\n",\
		ASensor_getName(Sensor), ASensor_getVendor(Sensor),ASensor_getMinDelay(Sensor), ASensor_getStringType(Sensor),Sensor);
	}
	else
	{
			fprintf(g_logfp,"Sensor for type %d not supported\n",type);
	}
	
	fprintf(g_logfp,"\nAPI:Get_Default_Sensor exit\n\n");
	
	return Sensor;
}

int Get_Sensor_List(ASensorManager* SensorManager, ASensorList* list)
{
	int ret = 0, i=0;
	ASensorList	   SensorList = NULL;
	
	fprintf(g_logfp,"API:Get_Sensor_List enter\n\n");
	
	//fprintf(g_logfp,"ASensorManager_getSensorList called with SensorManager = %p , list = %p, *list = %p \n",SensorManager,list,*list);
	
	ret = ASensorManager_getSensorList(SensorManager,list);
	
	//fprintf(g_logfp,"ASensorManager_getSensorList returned ret = %d , list = %p, *list = %p,**list = %p\,list[0] = %p\n",ret,list,*list,**list,list[0]);
	fprintf(g_logfp,"Device supports %d Sensors\n",ret);
	SensorList = *list;
	for(i = 0;i<ret;i++)
	{
		fprintf(g_logfp,"\tsensor[%d]: %s, vendor: %s , min-delay: %d, Type: %s\n",\
				i+1,ASensor_getName(SensorList[i]), ASensor_getVendor(SensorList[i]),ASensor_getMinDelay(SensorList[i]), ASensor_getStringType(SensorList[i]));	
	}
	
	fprintf(g_logfp,"\nAPI:Get_Sensor_List exit\n\n");
	
	return ret;
}

int Stop_Sensor_Data()
{
	int ret = 0;
	//void *res = NULL;

	fprintf(g_logfp,"API:Stop_Sensor_Data enter\n\n");
	
	//syslog(LOG_CRIT, "Stop_Sensor_Data enter %d\n\n",gCapture);

	/*disable capture flag*/
	gCapture = 0;
	
	//syslog(LOG_CRIT, "Stop_Sensor_Data gCapture %d\n\n",gCapture);
	
	if(accl_opt && gyro_opt)
	{
		/*wait for the threads to exit*/
//		syslog(LOG_CRIT, "pthread_join entered\n\n");
		ret = pthread_join(accl_opt->Thread_id, NULL);
        if (ret != 0)
        {
			fprintf(g_logfp,"pthread_join failed\n");
		}
		
		ret = pthread_join(gyro_opt->Thread_id, NULL);
        if (ret != 0)
        {
			fprintf(g_logfp,"pthread_join failed\n");
		}
		
//		syslog(LOG_CRIT, "pthread_join exit\n\n");

	}
	
	if(accl_opt)
		free(accl_opt);
	accl_opt = NULL;
		
	if(gyro_opt)
		free(gyro_opt);
	gyro_opt = NULL;
	
	fprintf(g_logfp,"\nAPI:Stop_Sensor_Data exit\n\n");
	
	
	return ret;
}

int Capture_Sensor_Data(ASensorManager* SensorManager)
{
	pthread_attr_t attr;
	int ret = 0;
	
	fprintf(g_logfp,"API:Capture_Sensor_Data enter\n\n");
	
	ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_attr_init failed %d\n",ret);
		return ret;
	}
	
	if(accl_opt || gyro_opt)
	{
		fprintf(g_logfp,"Invalid state; accl_opt || gyro_opt not NULL\n");
		return -1;
	}
		
	accl_opt = calloc(1, sizeof(sensor_options));
    if (!accl_opt)
    {
        fprintf(g_logfp,"calloc failed\n");
        return -1;
    }
    
    gyro_opt = calloc(1, sizeof(sensor_options));
    if (!gyro_opt)
    {
        fprintf(g_logfp,"calloc failed\n");
        return -1;
    }
	
	/*enable capture flag*/
	gCapture = 1;
	
	/*create a thread to capture accelerometer data*/
	accl_opt->SensorManager = SensorManager;
	accl_opt->Thread_Count = 1;
	accl_opt->SensorType =  ASENSOR_TYPE_ACCELEROMETER ;
	accl_opt->Rate = SENSOR_EVENT_RATE;
	
	ret = pthread_create(&accl_opt->Thread_id, &attr,
                                (void *)&SensorThread, accl_opt);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_create failed\n");
		return ret;
	}
	
	ret = pthread_attr_destroy(&attr);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_attr_destroy failed\n");
		return ret;
	}
		
	ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_attr_init failed %d\n",ret);
		return ret;
	}

	/*create a thread to capture gyroscope data*/
	gyro_opt->SensorManager = SensorManager;
	gyro_opt->Thread_Count = 2;
	gyro_opt->SensorType =   ASENSOR_TYPE_GYROSCOPE ;
	gyro_opt->Rate = SENSOR_EVENT_RATE;
	
	ret = pthread_create(&gyro_opt->Thread_id, &attr,
                                  &SensorThread, gyro_opt);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_create failed\n");
		return ret;
	}
	
	ret = pthread_attr_destroy(&attr);
    if (ret != 0)
    {
		fprintf(g_logfp,"pthread_attr_destroy failed\n");
		return ret;
	}
		
	fprintf(g_logfp,"\nAPI:Capture_Sensor_Data exit\n\n");
	
	return ret;
}

static void* SensorThread(void* aoptions)
{
	FILE *fp  = NULL; 
	sensor_options *sensor_opt = NULL;
	ASensorEventQueue* SensorEventQueue = NULL;
	ALooper* looper = NULL;
	ASensor const* Sensor = NULL;
	char filename[50] = "/sdcard/";
	ASensorEvent event;
	int ret = 0;
	int identifier = 0;
	int64_t initial_TS = 0,current_TS = 0,TS_Diff = 0;
	double evt_time = 0;
	
	
	sensor_opt = (sensor_options *)aoptions;
	
	fprintf(g_logfp,"API:SensorThread enter for Thread %d\n\n",sensor_opt->Thread_Count);
	
	/*Get default sensor for the sensor type*/
	Sensor = Get_Default_Sensor(sensor_opt->SensorManager,sensor_opt->SensorType);
	if(!Sensor)
	{
		fprintf(g_logfp,"Get_Default_Sensor failed\n");
		return NULL;
	}
	
	strncat(filename,ASensor_getStringType(Sensor),strlen(ASensor_getStringType(Sensor)));
	strncat(filename,".txt",4);
	
	fprintf(g_logfp,"filename = %s\n",filename);
	
	/*Open file to capture sensor data*/
	fp = fopen(filename,"w");
	if(!fp)
	{
		fprintf(g_logfp,"fopen failed\n");
		return NULL;
	}
	
	if(ASENSOR_TYPE_ACCELEROMETER  == sensor_opt->SensorType)
	{
			fprintf(fp,"time\t\taccX\t\taccY\t\taccZ\n");
	}
	else if(ASENSOR_TYPE_GYROSCOPE == sensor_opt->SensorType)
	{
			fprintf(fp,"time\t\tpitch\t\troll\t\tyaw\n");
	}
	
	looper = ALooper_forThread();
	if(!looper)
	{
		looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}
	
	/*Create event queue for sensor data*/
	SensorEventQueue = ASensorManager_createEventQueue(
											sensor_opt->SensorManager,
											looper, sensor_opt->Thread_Count,
											 NULL, NULL);
	
	/*Enable the sensor*/
	ret = ASensorEventQueue_enableSensor(SensorEventQueue, Sensor);
	if(0 > ret)
	{
		fprintf(g_logfp,"ASensorEventQueue_enableSensor failed  %d\n",ret);
		return NULL;
	}
	
	/*Set data event rate for the sensor*/
	ret = ASensorEventQueue_setEventRate(SensorEventQueue, Sensor, sensor_opt->Rate);
	if(0 > ret)
	{
		fprintf(g_logfp,"ASensorEventQueue_setEventRate failed  %d\n",ret);
		return NULL;
	}
	
	/*Capture the data*/
	while(gCapture)
	{
			identifier = ALooper_pollAll(-1, NULL, NULL, NULL);
			if(identifier == sensor_opt->Thread_Count)
			{
				ret = ASensorEventQueue_getEvents(SensorEventQueue, &event, 1);
				if (ret && (event.type == sensor_opt->SensorType)) 
                {
					if(!initial_TS)
					{
						initial_TS = current_TS = event.timestamp;
					}
					else
					{
						current_TS = event.timestamp;
					}
					
 					TS_Diff = current_TS - initial_TS;
 					
 					evt_time = (double)TS_Diff / (double)1000000000;
					
					if(ASENSOR_TYPE_ACCELEROMETER  == sensor_opt->SensorType)
					{
						fprintf(fp,"%f\t\t%f\t\t%f\t%f\t\t\n",
									evt_time,event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
					else if(ASENSOR_TYPE_GYROSCOPE  == sensor_opt->SensorType)
					{
						fprintf(fp,"%f\t\t%f\t\t%f\t%f\t\t\n",
									evt_time,event.vector.pitch, event.vector.roll, event.vector.azimuth);
					}
					
                }
			}
	}
	
	/*Disable the sensor*/
	ret =  ASensorEventQueue_disableSensor(SensorEventQueue, Sensor);
	if(0 > ret)
	{
		fprintf(g_logfp,"ASensorEventQueue_enableSensor failed  %d\n",ret);
		return NULL;
	}
	
	/* Delete the event queue*/
	if (SensorEventQueue) 
	{
        ASensorManager_destroyEventQueue(sensor_opt->SensorManager, SensorEventQueue);
    }
    
    if(fp)
		fclose(fp);
	fp = NULL;
	
//	syslog(LOG_CRIT, "SensorThread exit for Thread %d\n\n",sensor_opt->Thread_Count);
    
    fprintf(g_logfp,"API:SensorThread exit for Thread %d\n\n",sensor_opt->Thread_Count);

	return NULL;
}
