/*******************************************************************************************************
*
*  FILE NAME	: BiteCounterJNI.h
*
*  DESCRIPTION  : Java native interface header file.
* 
*  PLATFORM		: Android
*
*  DATE	               NAME					 REFERENCE			REASON
*  2-July-2018         Shashi Shivaraju      Initial code		BiteCounter 
*******************************************************************************************************/
#include <jni.h>
#include <syslog.h>
#include "BiteCount_Sensor.h"

typedef struct jni_context {
    JavaVM  *javaVM;
    jclass   mainActivityClz;
    jobject  mainActivityObj;
    jclass   displayActivityClz;
    jobject  displayActivityObj;
} JNIContext;

