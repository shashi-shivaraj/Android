/**************************************************************************************
*
*  FILE NAME	: BiteCount_Sensor.h
*
*  DESCRIPTION  : header file
*
*  DATE	              NAME					 REFERENCE			REASON
*  2nd July,2018      Shashi Shivaraju       Initial code		BiteCounter 
*
**************************************************************************************/
/*Header Inclusions*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <android/sensor.h>


/*Macro Definations*/
#define BC_ALGO_VERSION				"v_1_0_0"
#define CHECK_VOID(X)	if(!X){ return -1;}
#define DATA_FILE					"/sdcard/gyro_data.txt"
#define OUTPUT_FILE					"/sdcard/bite_index.txt"		
#define SENSOR_EVENT_RATE			66666		/*set to 15Hz (15 samples per second)*/
#define MAX_MEAL_DURATION			59*60 		/*set to 59 mins (almost 1 hour)*/
#define MAX_HTTP_RESPONSE_TIME 		2			/*set to 2 seconds*/
#define MAX_STORAGE_RECORDS			1000			/*maximum number of records supported in data backup file */  
#define MAX_DATAFILE_SIZE			300*MAX_STORAGE_RECORDS /*size threshold for data backup file*/ 

/*window size used to smooth the data*/
#define SMOOTH_WINDOW_SIZE	15
/*max http response*/
#define MAX_HTTP_RESPONSE_BYTES 1024
/*port number to send data*/
#define HTTP_PORT "80"
/*file location at server*/
#define SERVER_PATH "/upload/eventData"
/*server key*/
#define SERVER_KEY "data"

/*Event Callback to Application*/
typedef void (* fBiteCountCallBack)(int count);

/*Max meal duration callback to application*/
typedef void (* fEndSessionCallBack)();

typedef struct __sensor_options
{
	ASensorManager* SensorManager;
	fBiteCountCallBack  fCallback;
	fEndSessionCallBack fEndMeal;
	pthread_t		Thread_id;
	int				SensorType;
    int 			Thread_Count;
	unsigned int 	Rate; 		/*in microseconds*/
	unsigned int 	isEndSession;
}sensor_options;


/*Function Prototypes*/
int log_init(const char* filepath);
void log_deinit();
ASensorManager* get_sensormanager();
int Get_Sensor_List(ASensorManager* SensorManager, ASensorList* list);
ASensor const* Get_Default_Sensor(ASensorManager* SensorManager,int type);
int Capture_Gyro_Data(ASensorManager* SensorManager,
					fBiteCountCallBack  afCallback,
					fEndSessionCallBack afEndCallBack);
int Stop_Gyro_Data();
int BiteCount_Algo(FILE *fp);
int Get_Current_BiteCount();

int isBite(double velocity,int* event,int index,
			double timestamp,double *time_zero,double *time_one,double *time_two,
			FILE *fp_log);
			
int sendToSocket(int sock, const char* message);
int UploadDataToServer(const char* host, char* url, char* port, char* key, const char* data);
int UploadBackUpDatatoServer(const char* HostURL,const char* BackupFile);
int SetSocketBlocking(int sock,int Isblocking);
int storeDatatoFile(const char* Filename,const char* Data);
/* Function to set recv timeout for Blocking socket */
int SetSocketRECVtimeout(int sock,unsigned int aTime);
/* Function to set sendto timeout for Blocking socket */
int SetSocketSNDtimeout(int sock,unsigned int aTime);
