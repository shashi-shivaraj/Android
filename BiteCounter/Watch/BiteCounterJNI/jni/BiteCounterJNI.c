/*******************************************************************************************************
*
*  FILE NAME	: BiteCounterJNI.c
*
*  DESCRIPTION  : Java native interface for java BiteCounter Application.
* 
*  PLATFORM		: Android
*
*  DATE	               NAME					 REFERENCE			REASON
*  2-July-2018         Shashi Shivaraju      Initial code		BiteCounter 
*******************************************************************************************************/
/*Header file inclusion*/
#include "BiteCounterJNI.h"

/* Global declaration */
JNIContext g_JNIctx;
ASensorManager* gSensorManger = NULL;

/*function prototype declaration*/
void JNIAppCallBack(int Count);
void JNIEndMealCallBack();

/*will be invoked on loading native library*/
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	g_JNIctx.javaVM = vm;
	 
	syslog(LOG_INFO,"JNI_OnLoad Entered \n");
	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }
	
	if(env == NULL)
	{
		syslog(LOG_CRIT,"env is NULL \n");
		return -1;
	}
	
	/* success -- return valid version number */
	return JNI_VERSION_1_6;
}
 
jint Java_com_icountbites_MainActivity_init_1logging (
        JNIEnv *env,
        jobject thisObj,
        jstring JNILogFile) {
			
	int ret = 0;
	const char *LogFile;
	
	
	LogFile = (*env)->GetStringUTFChars(env, JNILogFile, NULL);
	if (!LogFile) 
		return -1;
	
	syslog(LOG_INFO, "log file = %s\n",LogFile);
	syslog(LOG_INFO, "Bite Counter Algo version = %s\n",BC_ALGO_VERSION);		
		
	ret = log_init(LogFile);
	if(-1 == ret )
		syslog(LOG_CRIT, "log_init failed");
		
	(*env)->ReleaseStringUTFChars(env, JNILogFile, LogFile);	
		
	return ret;
}

jint Java_com_icountbites_MainActivity_check_1sensor_1capabilities (
		JNIEnv *env,
        jobject thisObj) {

	
	ASensorList	SensorList = NULL;  
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
	   syslog(LOG_INFO, "Device supports %d Sensors",ret);
     }
     else
     {
		syslog(LOG_CRIT, "No sensors supported");
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

jint Java_com_icountbites_MainActivity_start_1bite_1detection (
		JNIEnv *env,
        jobject thisObj) {
		
	int ret = 0;
	
	jclass clz = (*env)->GetObjectClass(env, thisObj);
    g_JNIctx.mainActivityClz = (*env)->NewGlobalRef(env, clz);
    g_JNIctx.mainActivityObj = (*env)->NewGlobalRef(env, thisObj);
	
	ret = Capture_Gyro_Data(gSensorManger,JNIAppCallBack,JNIEndMealCallBack);
	if(0 != ret)
	{
		 syslog(LOG_CRIT, "Capture_Sensor_Data failed %d\n",ret);
		 return -1;
	}
	
	return ret;			
}

void Java_com_icountbites_DisplayCountActivity_set_1ActivityContext(
		JNIEnv *env,
        jobject thisObj) {
 
	jclass clz = (*env)->GetObjectClass(env, thisObj);
    g_JNIctx.displayActivityClz = (*env)->NewGlobalRef(env, clz);
    g_JNIctx.displayActivityObj = (*env)->NewGlobalRef(env, thisObj);
    
 /* jclass cls = (*env)->GetObjectClass(env, obj);*/
  jmethodID mid = (*env)->GetMethodID(env, g_JNIctx.displayActivityClz, "UpdateBiteCount", "(I)V");
  if (mid == 0)
    return;
  (*env)->CallVoidMethod(env, g_JNIctx.displayActivityObj, mid, 0);
    		
}

jint Java_com_icountbites_DisplayCountActivity_getCurrentBiteCount (
		JNIEnv *env,
        jobject thisObj) {
			
	return Get_Current_BiteCount();
			
}

jint Java_com_icountbites_MainActivity_stop_1bite_1detection (
		JNIEnv *env,
        jobject thisObj) {
			
	int ret = 0;
	
	ret = Stop_Gyro_Data(gSensorManger);
	if(0 > ret)
	{
		 syslog(LOG_CRIT, "Stop_Sensor_Data failed %d\n",ret);
		 return -1;
	}
	
	 /*release object we allocated from start bite detection function */
    (*env)->DeleteGlobalRef(env, g_JNIctx.mainActivityClz);
    (*env)->DeleteGlobalRef(env, g_JNIctx.mainActivityObj);
    (*env)->DeleteGlobalRef(env, g_JNIctx.displayActivityClz);
    (*env)->DeleteGlobalRef(env, g_JNIctx.displayActivityObj);
    g_JNIctx.mainActivityObj = NULL;
    g_JNIctx.mainActivityClz = NULL;
    g_JNIctx.displayActivityClz = NULL;
    g_JNIctx.displayActivityObj = NULL;
    
    
	
	return ret;
		
}

void Java_com_icountbites_MainActivity_deinit_1logging(
		JNIEnv *env,
        jobject thisObj) {
	
	 syslog(LOG_INFO, "log_deinit called");
	 log_deinit();
}

void JNIEndMealCallBack()
{
	syslog(LOG_INFO, "JNIEndMealCallBack involked");
	
	JavaVM *javaVM = g_JNIctx.javaVM;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6); /*should fail since callback is from native thread*/
    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
           syslog(LOG_CRIT,"Failed to AttachCurrentThread, ErrorCode = %d", res);
            return;
        }
    }
    
   /* jclass cls = (*env)->GetObjectClass(env, obj);*/
  jmethodID mid = (*env)->GetMethodID(env, g_JNIctx.displayActivityClz, "ForceEndSession", "()V");
  if (mid == 0)
  {
	syslog(LOG_INFO, "GetMethodID returned with 0");
    return;
  }
  
  (*env)->CallVoidMethod(env, g_JNIctx.displayActivityObj, mid);

}


void JNIAppCallBack(int Count)
{
	syslog(LOG_INFO, "JNIAppCallBack involked with %d",Count);
	 
	JavaVM *javaVM = g_JNIctx.javaVM;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6); /*should fail since callback is from native thread*/
    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
           syslog(LOG_CRIT,"Failed to AttachCurrentThread, ErrorCode = %d", res);
            return;
        }
    }
    
   /* jclass cls = (*env)->GetObjectClass(env, obj);*/
  jmethodID mid = (*env)->GetMethodID(env, g_JNIctx.displayActivityClz, "UpdateBiteCount", "(I)V");
  if (mid == 0)
  {
	syslog(LOG_INFO, "GetMethodID returned with 0");
    return;
  }
  (*env)->CallVoidMethod(env, g_JNIctx.displayActivityObj, mid, Count);
  
}

int Java_com_icountbites_EndActivity_UploadToServer(JNIEnv *env,
        jobject thisObj,
        jstring JNIHostURL,jstring JNIData,jstring JNIFile,jint JNIisStoretoFile)
{
	
	const char *inHostURL,*inData,*inFile;
	int ret = 0;
	int isStoretoFile = (int)JNIisStoretoFile;
	 
	inHostURL = (*env)->GetStringUTFChars(env, JNIHostURL, NULL);
	if (!inHostURL) 
		return -1;
		
	inData = (*env)->GetStringUTFChars(env, JNIData, NULL);
	if (!inData) 
		return -1;
		
	inFile = (*env)->GetStringUTFChars(env, JNIFile, NULL);
	if (!inFile) 
		return -1;
	
	syslog(LOG_INFO, "UploadToServer involked with url = %s\n Data = %s\n File = %s\n",inHostURL,inData,inFile);
	
	if(isStoretoFile)
	{
		syslog(LOG_INFO, "Data to be stored in backup file\n");
		ret = storeDatatoFile(inFile,inData);
		if(0 != ret)
		{
			syslog(LOG_CRIT, "storeDatatoFile failed %d\n",ret);	
		}	
	}
	else
	{
		/*Device connected to network;*/
		
		/*Try to upload the previous data from the backup file to server*/
		ret = UploadBackUpDatatoServer(inHostURL,inFile);
		if(0 != ret)
		{
			syslog(LOG_CRIT, "UploadFileDatatoServer failed %d\n",ret);
		}
		  
		/*Try to upload the current data to server;
		 *If upload to server to fails,then store the data to the backup file
		 */
		ret = UploadDataToServer(inHostURL,SERVER_PATH,HTTP_PORT,SERVER_KEY,inData);
		if(0 == ret)
		{
			syslog(LOG_INFO, "UploadDataToServer success %d\n",ret);	
		}
		else
		{
			syslog(LOG_CRIT, "sendDataToServer failed %d;Data to be stored in backup file\n",ret);
			ret = storeDatatoFile(inFile,inData);
			if(0 != ret)
			{
				syslog(LOG_CRIT, "storeDatatoFile failed %d\n",ret);	
			}	
		}
	}
	
	/* release resources */
	(*env)->ReleaseStringUTFChars(env, JNIHostURL, inHostURL);
	(*env)->ReleaseStringUTFChars(env, JNIData, inData);  
	(*env)->ReleaseStringUTFChars(env, JNIFile, inFile);
	
	return ret;    
}
