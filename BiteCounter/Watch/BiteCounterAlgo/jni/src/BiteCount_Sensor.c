/*******************************************************************************************************
*
*  FILE NAME	: BiteCount_Sensor.c
*
*  DESCRIPTION  : API's to capture Gyroscope sensor data.
*
*  DATE	               NAME					 REFERENCE			REASON
*  2nd July,2018       Shashi Shivaraju      Initial code		BiteCounter
*******************************************************************************************************/

/*Header inclusion*/
#include "BiteCount_Sensor.h"
//#include <syslog.h>

/*global declaration*/
FILE *g_logfp = NULL;
sensor_options *gyro_opt = NULL;
int totalBites = 0;


unsigned int gCapture = 0; 
static void* SensorThread(void* aoptions);

int log_init(const char* filepath)
{
	if(g_logfp)
		return 0;
		
#ifdef CONFIG_ALL_LOGS
	g_logfp = fopen(filepath,"w+");
	if(!g_logfp)
		return -1;
		
	fprintf(g_logfp,"*********START OF LOG********\n");
	fprintf(g_logfp,"BiteCounter Algorithm Version = %s\n",BC_ALGO_VERSION);
	fprintf(g_logfp,"\nAPI:log_init exit\n\n");
	
#endif /*CONFIG_ALL_LOGS*/
	return 0;
}

void log_deinit()
{
	if(g_logfp)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"API:log_deinit enter\n\n");
		fprintf(g_logfp,"*********END OF LOG***********\n");
		fclose(g_logfp);
#endif /*CONFIG_ALL_LOGS*/
	}
	g_logfp = NULL;
}

ASensorManager* get_sensormanager()
{
	ASensorManager* SensorManager = NULL;

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:get_sensormanager enter\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	SensorManager = ASensorManager_getInstance();

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"SensorManager = %p\n",SensorManager);
	fprintf(g_logfp,"\nAPI:get_sensormanager exit\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	return SensorManager;	
}

ASensor const* Get_Default_Sensor(ASensorManager* SensorManager,int type)
{
	ASensor const* Sensor = NULL;
	
#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:Get_Default_Sensor enter\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	Sensor = ASensorManager_getDefaultSensor(SensorManager,type);
    if (Sensor) 
    {
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"sensor:%s, vendor: %s , min-delay: %d, Type: %s Pointer = %p\n",\
		ASensor_getName(Sensor), ASensor_getVendor(Sensor),ASensor_getMinDelay(Sensor), ASensor_getStringType(Sensor),Sensor);
#endif /*CONFIG_ALL_LOGS*/
	}
	else
	{
#ifdef CONFIG_ALL_LOGS
			fprintf(g_logfp,"Sensor for type %d not supported\n",type);
#endif /*CONFIG_ALL_LOGS*/
	}

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"\nAPI:Get_Default_Sensor exit\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	return Sensor;
}

int Get_Sensor_List(ASensorManager* SensorManager, ASensorList* list)
{
	int ret = 0;
	ASensorList	   SensorList = NULL;
	
#ifdef CONFIG_ALL_LOGS
	int i=0;
	fprintf(g_logfp,"API:Get_Sensor_List enter\n\n");
	//fprintf(g_logfp,"ASensorManager_getSensorList called with SensorManager = %p , list = %p, *list = %p \n",SensorManager,list,*list);
#endif /*CONFIG_ALL_LOGS*/
	
	ret = ASensorManager_getSensorList(SensorManager,list);

#ifdef CONFIG_ALL_LOGS
	//fprintf(g_logfp,"ASensorManager_getSensorList returned ret = %d , list = %p, *list = %p,**list = %p\,list[0] = %p\n",ret,list,*list,**list,list[0]);
	fprintf(g_logfp,"Device supports %d Sensors\n",ret);
#endif /*CONFIG_ALL_LOGS*/
	SensorList = *list;
	
#ifdef CONFIG_ALL_LOGS
	for(i = 0;i<ret;i++)
	{
		fprintf(g_logfp,"\tsensor[%d]: %s, vendor: %s , min-delay: %d, Type: %s\n",\
				i+1,ASensor_getName(SensorList[i]), ASensor_getVendor(SensorList[i]),ASensor_getMinDelay(SensorList[i]), ASensor_getStringType(SensorList[i]));	
	}
	
	fprintf(g_logfp,"\nAPI:Get_Sensor_List exit\n\n");
#endif /*CONFIG_ALL_LOGS*/

	return ret;
}

int Stop_Gyro_Data()
{
	int ret = 0;
#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:Stop_Sensor_Data enter\n\n");
	//syslog(LOG_CRIT, "Stop_Sensor_Data enter %d\n\n",gCapture);
#endif /*CONFIG_ALL_LOGS*/

	/*disable capture flag*/
	gCapture = 0;
		
	if(gyro_opt)
	{
		/*wait for the threads to exit*/
//		syslog(LOG_CRIT, "pthread_join entered\n\n");
		
		ret = pthread_join(gyro_opt->Thread_id, NULL);
        if (ret != 0)
        {
#ifdef CONFIG_ALL_LOGS
			fprintf(g_logfp,"pthread_join failed\n");
#endif /*CONFIG_ALL_LOGS*/
		}
		
//		syslog(LOG_CRIT, "pthread_join exit\n\n");

	}
		
	if(gyro_opt)
		free(gyro_opt);
	gyro_opt = NULL;
	
#ifdef CONFIG_FILE_METHOD
	/*Call BiteCounter Algo to detect bites*/
	ret = BiteCount_Algo(g_logfp);
	if(totalBites != ret)
	{
#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"[BUG !!!]Total bites detected: Runtime %d Standalone %d\n",totalBites,ret);
#endif /*CONFIG_ALL_LOGS*/	
	}
#endif  /*CONFIG_FILE_METHOD*/	
	
	
#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"\nAPI:Stop_Sensor_Data exit\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	ret = totalBites;
	/* reset the global counter */
	totalBites = 0;
	
	return ret;
}

int Capture_Gyro_Data(ASensorManager* SensorManager,
						fBiteCountCallBack afAppCallBack,
						fEndSessionCallBack afEndCallBack)
{
	pthread_attr_t attr;
	int ret = 0;

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:Capture_Sensor_Data enter\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"pthread_attr_init failed %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
		return ret;
	}
	
	if(gyro_opt)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"Invalid state;gyro_opt not NULL\n");
#endif /*CONFIG_ALL_LOGS*/
		return -1;
	}
    
    gyro_opt = calloc(1, sizeof(sensor_options));
    if (!gyro_opt)
    {
#ifdef CONFIG_ALL_LOGS
        fprintf(g_logfp,"calloc failed\n");
#endif /*CONFIG_ALL_LOGS*/
        return -1;
    }
	
	/*enable capture flag*/
	gCapture = 1;
	
		
	ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"pthread_attr_init failed %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
		return ret;
	}

	/*create a thread to capture gyroscope data*/
	gyro_opt->SensorManager = SensorManager;
	gyro_opt->Thread_Count = 1;
	gyro_opt->SensorType =   ASENSOR_TYPE_GYROSCOPE ;
	gyro_opt->Rate = SENSOR_EVENT_RATE;
	gyro_opt->fCallback = afAppCallBack;
	gyro_opt->fEndMeal = afEndCallBack;
	gyro_opt->isEndSession = 0;
	
	ret = pthread_create(&gyro_opt->Thread_id, &attr,
                                  &SensorThread, gyro_opt);
    if (ret != 0)
    {
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"pthread_create failed\n");
#endif /*CONFIG_ALL_LOGS*/
		return ret;
	}
	
	ret = pthread_attr_destroy(&attr);
    if (ret != 0)
    {
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"pthread_attr_destroy failed\n");
#endif /*CONFIG_ALL_LOGS*/
		return ret;
	}

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"\nAPI:Capture_Sensor_Data exit\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	return ret;
}

static void* SensorThread(void* aoptions)
{
#ifdef CONFIG_FILE_METHOD
	FILE *fp  = NULL; 
#endif /*CONFIG_FILE_METHOD*/
	sensor_options *sensor_opt = NULL;
	ASensorEventQueue* SensorEventQueue = NULL;
	ALooper* looper = NULL;
	ASensor const* Sensor = NULL;
	ASensorEvent event;
	int64_t initial_TS = 0,current_TS = 0,TS_Diff = 0;
	double evt_time = 0;
	double running_sum = 0,roll_velocity = 0,roll =0;
	double time_zero = 0,time_one = 0,time_two = 0;
#ifdef CONFIG_SMOOTH_DATA
	double gyro_roll_data[SMOOTH_WINDOW_SIZE] = {0};
#endif /*CONFIG_SMOOTH_DATA*/
	int Bite_Event = 0;
	int ret = 0;
	int total_samples = 0;
	int identifier = 0;
	int index = 0;
	
	sensor_opt = (sensor_options *)aoptions;

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:SensorThread enter for Thread %d\n\n",sensor_opt->Thread_Count);
#endif /*CONFIG_ALL_LOGS*/
	
	/*Get default sensor for the sensor type*/
	Sensor = Get_Default_Sensor(sensor_opt->SensorManager,sensor_opt->SensorType);
	if(!Sensor)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"Get_Default_Sensor failed\n");
#endif /*CONFIG_ALL_LOGS*/
		return NULL;
	}
	
#ifdef CONFIG_FILE_METHOD
	/*Open file to capture sensor data*/
	fp = fopen(DATA_FILE,"w");
	if(!fp)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"fopen failed for %s\n",DATA_FILE);
#endif /*CONFIG_ALL_LOGS*/
		return NULL;
	}
	else
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"fopen success for %s\n",DATA_FILE);
#endif /*CONFIG_ALL_LOGS*/
	}
	
	/*print the header for the data file*/
	if(ASENSOR_TYPE_GYROSCOPE == sensor_opt->SensorType)
	{
			fprintf(fp,"time\t\tpitch\t\troll\t\tyaw\n");
	}
#endif /*CONFIG_FILE_METHOD*/
	
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
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"ASensorEventQueue_enableSensor failed  %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
		return NULL;
	}
	
	/*Set data event rate for the sensor*/
	ret = ASensorEventQueue_setEventRate(SensorEventQueue, Sensor, sensor_opt->Rate);
	if(0 > ret)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"ASensorEventQueue_setEventRate failed  %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
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
 					
 					if(MAX_MEAL_DURATION < evt_time && !sensor_opt->isEndSession)
 					{
#ifdef CONFIG_ALL_LOGS
						fprintf(g_logfp,"Max Meal Duration reached at %f\n",evt_time);
#endif /*CONFIG_ALL_LOGS*/
						sensor_opt->fEndMeal();
						sensor_opt->isEndSession = 1;
					}
 					
 					/*Rotation is positive in the counter-clockwise direction;
 					  This is the standard mathematical definition of positive rotation and 
 					  is not the same as the definition for roll that is used by the orientation sensor.*/
 					roll = 0.0 - event.vector.v[0]; /*negating to match standard directions*/

#ifdef CONFIG_FILE_METHOD
					fprintf(fp,"%lf %lf %lf %lf\n",
								evt_time,event.vector.v[2], roll, event.vector.v[1]);
#endif /*CONFIG_FILE_METHOD*/		
					
					
					roll_velocity = (roll * 180)/M_PI; /*convert to degree/s*/

#ifdef CONFIG_SMOOTH_DATA
					/*smooth the data*/
					if(SMOOTH_WINDOW_SIZE > total_samples)
					{
						gyro_roll_data[total_samples] = roll_velocity;
						running_sum = running_sum + roll_velocity;
					}
					else
					{
						index = (total_samples - SMOOTH_WINDOW_SIZE) % SMOOTH_WINDOW_SIZE;
						running_sum = running_sum - gyro_roll_data[index] + roll_velocity;
						gyro_roll_data[index] = roll_velocity; /*replace the data in circular queue*/
						roll_velocity =  running_sum/SMOOTH_WINDOW_SIZE;
					}
#endif /*CONFIG_SMOOTH_DATA*/
					
					
#ifdef CONFIG_ALL_LOGS
					fprintf(g_logfp," index %d,raw roll %lf,smooth roll %f,timestamp %lf,sum %lf,index %d\n",
								total_samples,(event.vector.roll*180)/M_PI,roll_velocity,evt_time,running_sum,index);
#endif /*CONFIG_ALL_LOGS*/
								
								
					/*Caluclate number of bites consumed using gyroscope data*/
					ret = isBite(roll_velocity,&Bite_Event,total_samples,
									evt_time,&time_zero,&time_one,&time_two,g_logfp);
					if(ret == total_samples) /*bite detected*/
					{
#ifdef CONFIG_ALL_LOGS
						fprintf(g_logfp,"Runtime Bite Detected at %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
						totalBites++; /*Bite Detected ;Output Sample index to file*/
						/*send callback to display the current bite count*/
						sensor_opt->fCallback(totalBites);
					}
						
					total_samples ++; /* increment counter for each sample */			
                }
			}
	}
	
	/*Disable the sensor*/
	ret =  ASensorEventQueue_disableSensor(SensorEventQueue, Sensor);
	if(0 > ret)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"ASensorEventQueue_enableSensor failed  %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
		return NULL;
	}
	
	/* Delete the event queue*/
	if (SensorEventQueue) 
	{
        ASensorManager_destroyEventQueue(sensor_opt->SensorManager, SensorEventQueue);
    }

#ifdef CONFIG_FILE_METHOD
    if(fp)
		fclose(fp);
	fp = NULL;
#endif /*CONFIG_FILE_METHOD*/
	
#ifdef CONFIG_ALL_LOGS
//	syslog(LOG_CRIT, "SensorThread exit for Thread %d\n\n",sensor_opt->Thread_Count);
    fprintf(g_logfp,"API:SensorThread exit for Thread %d\n\n",sensor_opt->Thread_Count);
#endif /*CONFIG_ALL_LOGS*/

	return NULL;
}

int Get_Current_BiteCount()
{
	int count = 0;
#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"API:Get_Current_BiteCount Enter\n");
#endif /*CONFIG_ALL_LOGS*/

	count = totalBites;

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"Current_BiteCount = %d\n",count);
	fprintf(g_logfp,"API:Get_Current_BiteCount Exit\n\n");
#endif /*CONFIG_ALL_LOGS*/
	
	return count;
}

int sendToSocket(int sock, const char* message)
{
	int ret = 0;
	ret = send(sock, message, strlen(message), 0);
	if(ret < 0)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"send failed for sock %d, error code = %d",sock,errno);
#endif /*CONFIG_ALL_LOGS*/
	}
	return ret;
}

int UploadDataToServer(const char* host, char* url, char *port, char* key, const char* data)
{
	int ret = 0;
	int sockfd = -1;
    char response[MAX_HTTP_RESPONSE_BYTES];
    int responseBytes; /*stores how many bytes we're getting back from our response*/
    int responseBufLen = MAX_HTTP_RESPONSE_BYTES; /*length of the response buffer */
    char contentHeader[100];/*for creating dynamic content header */
    char *token;/*token for our response parser*/   
    int httpSuccess = -1; /*flag for http success - defaults to failed*/
    int dbSuccess = -1;/*flag for db success - defaults to failed*/
    
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
	
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(host, port, &hints, &servinfo)) != 0) 
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(g_logfp,"getaddrinfo failed: %s for %s\n", gai_strerror(ret),host);
#endif /*CONFIG_ALL_LOGS*/
		return -1;
	}

	/*loop through all the results and connect to the first we can*/
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
#ifdef CONFIG_ALL_LOGS
			fprintf(g_logfp,"socket() failed: %d\n", sockfd);
#endif /*CONFIG_ALL_LOGS*/
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
#ifdef CONFIG_ALL_LOGS
			fprintf(g_logfp,"connect() failed\n");
#endif /*CONFIG_ALL_LOGS*/
			if(sockfd)
				close(sockfd);
			continue;
		}
		
		/*Set Socket to Non-Blocking*/
		//ret = SetSocketBlocking(sockfd,0);
		//if(0 != ret)
		//{
//#ifdef CONFIG_ALL_LOGS		
			//fprintf(g_logfp,"SetSocketBlocking failed for sock %d with %d\n",sockfd,ret);
//#endif /*CONFIG_ALL_LOGS*/				
		//}
		
		/*Set time out for recv call for the socket to make it non blocking*/
		SetSocketRECVtimeout(sockfd,MAX_HTTP_RESPONSE_TIME);
			
		break;
	}
		
	if (p == NULL) 
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"client: failed to connect\n");
#endif /*CONFIG_ALL_LOGS*/
		if(sockfd)
			close(sockfd);
			return -1;
	}

	freeaddrinfo(servinfo); // all done with this structure
		
		
	sendToSocket(sockfd, "POST "); // POST method
	sendToSocket(sockfd, url);// send URL we're POSTing to
	sendToSocket(sockfd, " HTTP/1.0\r\n");// using 1.0 method so we don't have to specify the host
	//sendToSocket(sockfd,"Host: bc.ces.clemson.edu\r\n");
	sprintf(contentHeader, "Content-Length: %d\r\n", strlen(key)+strlen(data)+1); // header is dynamic based on the length of the content we're sending
																					 // key length + data length + 1 character for = sign

	sendToSocket(sockfd, contentHeader);
	sendToSocket(sockfd, "Content-Type: application/x-www-form-urlencoded\r\n");
	sendToSocket(sockfd, "\r\n");
	sendToSocket(sockfd, key);	 // send the key
	sendToSocket(sockfd, "=");	// send equals
	sendToSocket(sockfd, data);		// send the data 

    
	responseBytes = recv(sockfd, response, responseBufLen, 0);// get our response
	// bytes recieved must be greater than zero and less that MAX_HTTP_RESPONSE_BYTES
	if (responseBytes == 0 || responseBytes > MAX_HTTP_RESPONSE_BYTES)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"recv() call returned with invalid response size %d\n",responseBytes);
#endif /*CONFIG_ALL_LOGS*/
		close(sockfd);
		sockfd = -1;
		return -1; // error with recv call
	}
	else
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"recv() call returned with valid response size %d and response %s\n",responseBytes,response);
#endif /*CONFIG_ALL_LOGS*/
	}

	// close the socket
	close(sockfd);
	sockfd = -1;

    // parse our response
    token = strtok(response,"\r\n");
    // iterate through our tokens
    while (token != NULL)
    {
#ifdef CONFIG_ALL_LOGS		
		//fprintf(g_logfp,"Token = %s\n",token);
#endif /*CONFIG_ALL_LOGS*/

		 // check for response code 200
        if (strcmp(token, "HTTP/1.0 200 OK") == 0)
        {
            httpSuccess = 0;
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"200 OK found %d\n",httpSuccess);
#endif /*CONFIG_ALL_LOGS*/
        }
        // check for DB success code
        else if (strcmp(token, "DB-Post-Success: true") == 0)
        {
            dbSuccess = 0;
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"DB-Post-Success found %d\n",dbSuccess);
#endif /*CONFIG_ALL_LOGS*/
        }

        token = strtok(NULL, "\r\n");
    }

    // if http response code or db write failed, return -1
    if (httpSuccess != 0 || dbSuccess != 0)
    {
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"failure due to  httpSuccess %d and dbSuccess %d\n",httpSuccess,dbSuccess);
#endif /*CONFIG_ALL_LOGS*/

        return -1;
    }

    return 0;
}

/** Function to set Socket to Non-Blocking */
int SetSocketBlocking(int sock,int Isblocking)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)
    {
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"fcntl error %d for sock %d\n",flags,sock);
#endif /*CONFIG_ALL_LOGS*/
		return -1;
    }

	if(!Isblocking)
	{
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
		{   
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"fcntl F_SETFL error for sock %d\n",sock);
#endif /*CONFIG_ALL_LOGS*/
			return -1;
		}
		
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"SetSocketBlocking success for sock %d\n",sock);
#endif /*CONFIG_ALL_LOGS*/
	}

	return 0;
}

int storeDatatoFile(const char* Filename,const char* Data)
{
	FILE *fp = NULL;
	char *fileData = NULL,*dataString = NULL; 
	unsigned int file_size = 0;
	int ret = 0,index = 0,i = 0,count = 0;
	char data = 0;
	
#ifdef CONFIG_ALL_LOGS		
	fprintf(g_logfp,"[storeDatatoFile] Enter\n");
#endif /*CONFIG_ALL_LOGS*/

	/*Open the file in append mode*/
	fp = fopen(Filename,"a+");
	if(!fp)
	{
#ifdef CONFIG_ALL_LOGS	
		fprintf(g_logfp,"fopen failed for %s\n",Filename);
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;

	}
	
	/*In append mode,file pointer is already at end*/
	file_size = ftell(fp); /*get file size*/
	
	if(MAX_DATAFILE_SIZE > file_size+strlen(Data))/*check thresold to append new entry*/
	{
		ret = fprintf(fp,"%s",Data);
		if(ret != strlen(Data))
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"fprintf returned %d instead of %d\n",ret,strlen(Data));
#endif /*CONFIG_ALL_LOGS*/
			ret = -1;
			goto CLEANUP;
		}	
		ret = 0;
	}
	else
	{
#ifdef CONFIG_ALL_LOGS	
		fprintf(g_logfp," [storeDatatoFile] Deleting oldest entry and adding newest entry\n");
#endif /*CONFIG_ALL_LOGS*/
		
		/*move the file pointer to beginning of the file*/
		fseek(fp, 0L, SEEK_SET);
		
		/*allocate memory to store the data in back to file*/
		fileData = (char*)calloc(file_size+strlen(Data),sizeof(char));
		if(!fileData)
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"calloc failed for fileData\n");
			ret = -1;
			goto CLEANUP;
#endif /*CONFIG_ALL_LOGS*/
		}
		
		/*allocate memory to read the data from the file*/
		dataString = (char*)calloc(file_size,sizeof(char));
		if(!dataString)
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"calloc failed for dataString\n");
			ret = -1;
			goto CLEANUP;
#endif /*CONFIG_ALL_LOGS*/
		}
		
		i = 0;
		index  = 0;
		while(0<fscanf(fp,"%c",&data))/*will exit at EOF or fscanf error*/
		{
			dataString[i] = data;
			if('\n' == data)
			{
				count ++;
			}
			i++;
		
			/*The first string to be removed from the file consist of three substrings
			* separated with '\n'
			* refer GenerateDataString() in application layer for format*/
			if(3 == count & data == '\n')
			{
				index = i;
				
				/*read the rest of the data from backup file*/
				if(file_size-index)
				{
					ret = fread(fileData,1,file_size-index,fp);
					if(ret != (file_size-index))
					{
#ifdef CONFIG_ALL_LOGS
						fprintf(g_logfp,"fread failed with %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
						ret = -1;
						goto CLEANUP;
					}
				}
			    break;
			 }
	     }
		
		/*close the file*/
		if(fp)
			fclose(fp);
		fp = NULL;
		
		/*Rewrite the backup data file*/
		fp = fopen(Filename,"w");
		if(!fp)
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"fopen failed\n");
#endif /*CONFIG_ALL_LOGS*/
			ret = -1;
			goto CLEANUP;
		}

		ret = fprintf(fp,"%s",fileData);
		if(ret != strlen(fileData))
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"fprintf returned %d instead of %d\n",ret,strlen(fileData));
#endif /*CONFIG_ALL_LOGS*/
			ret = -1;
			goto CLEANUP;
		}
			
		ret = fprintf(fp,"%s",Data);
		if(ret != strlen(Data))
		{
#ifdef CONFIG_ALL_LOGS		
			fprintf(g_logfp,"fprintf returned %d instead of %d\n",ret,strlen(Data));
#endif /*CONFIG_ALL_LOGS*/
			ret = -1;
			goto CLEANUP;
		}
		
		ret = 0;
	}
		
CLEANUP:
	if(fileData)
		free(fileData);
	fileData = NULL;
		
	if(dataString)
		free(dataString);
	dataString = NULL;
			
	if(fp)
		fclose(fp);
	fp = NULL;
	
#ifdef CONFIG_ALL_LOGS		
	fprintf(g_logfp,"[storeDatatoFile] Exit\n\n");
#endif /*CONFIG_ALL_LOGS*/

	return ret;
}

int UploadBackUpDatatoServer(const char* HostURL,const char* BackupFile)
{
	FILE *fp = NULL;
	char *fileData = NULL,*dataString = NULL; 
	unsigned int file_size = 0;
	int ret = 0,i = 0,count = 0,index = 0;
	char data = 0;
	
	
#ifdef CONFIG_ALL_LOGS		
	fprintf(g_logfp,"[UploadBackUpDatatoServer] Enter\n");
#endif /*CONFIG_ALL_LOGS*/

	fp = fopen(BackupFile,"r");
	if(!fp)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"fopen failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"Backup data file size =%d\n",file_size);
#endif /*CONFIG_ALL_LOGS*/
	
	/*allocate memory to store the data in case of upload failure*/
	fileData = (char*)calloc(file_size,sizeof(char));
	if(!fileData)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"calloc failed for fileData\n");
		ret = -1;
		goto CLEANUP;
#endif /*CONFIG_ALL_LOGS*/
	}
	
	/*allocate memory to store the data to be uploaded to server*/
	dataString = (char*)calloc(file_size,sizeof(char));
	if(!dataString)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"calloc failed for dataString\n");
		ret = -1;
		goto CLEANUP;
#endif /*CONFIG_ALL_LOGS*/
	}
	
	i = 0;
	index  = 0;
	while(0<fscanf(fp,"%c",&data))/*will exit at EOF or fscanf error*/
	{
		dataString[i] = data;
		if('\n' == data)
		{
			count ++;
		}
		
		i++;
		
		/*the string to be uploaded to the server consist of three substrings
		 * separated with '\n'
		 * refer GenerateDataString() in application layer for format*/
		if(3 == count & data == '\n')
		{
#ifdef CONFIG_ALL_LOGS
			fprintf(g_logfp,"%s",dataString);
#endif /*CONFIG_ALL_LOGS*/
            
            /*update the amount of data read*/
			index = index + i;
			
			ret = UploadDataToServer(HostURL,SERVER_PATH,HTTP_PORT,SERVER_KEY,dataString);
			if(0 != ret)
			{	
#ifdef CONFIG_ALL_LOGS
				fprintf(g_logfp,"UploadDataToServer failed %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
				strncpy(fileData,dataString,strlen(dataString));
				
				/*upload to data failed*/
				/*read the rest of the data from backup file*/
				ret = 0;
				if(file_size-index)
				{
					ret = fread(fileData+strlen(dataString),1,file_size-index,fp);
					if(ret != file_size-index)
					{
#ifdef CONFIG_ALL_LOGS
						fprintf(g_logfp,"fread failed with %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
					}
					else
						index = index+ret;
				}
			   
			    break;
			}
			
			/*reset the variables*/
			memset(dataString,0,file_size);
			i = 0;
			count = 0; 
		}
	}
	
	if(fp)
		fclose(fp);
		fp = NULL;
	
	/*Rewrite the backup data file*/
	fp = fopen(BackupFile,"w");
	if(!fp)
	{
#ifdef CONFIG_ALL_LOGS		
		fprintf(g_logfp,"fopen failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}

#ifdef CONFIG_ALL_LOGS
	fprintf(g_logfp,"Backup data : %s\n len = %d index = %d\n",fileData,strlen(fileData),index);
#endif /*CONFIG_ALL_LOGS*/

	if(strlen(fileData))
	{
		fprintf(fp,"%s",fileData);
	}
	
	if(fp)
		fclose(fp);
		fp = NULL;

CLEANUP:
		if(fileData)
			free(fileData);
		fileData = NULL;
		
		if(dataString)
			free(dataString);
		dataString = NULL;
			
		if(fp)
			fclose(fp);
		fp = NULL;
		
#ifdef CONFIG_ALL_LOGS		
	fprintf(g_logfp,"[UploadBackUpDatatoServer] Exit\n\n");
#endif /*CONFIG_ALL_LOGS*/

	return ret;
}

/* Function to set sendto timeout for Blocking socket */
int SetSocketSNDtimeout(int sock,unsigned int aTime)
{
	struct timeval timeout = {0,0};      
    timeout.tv_sec = aTime;
    timeout.tv_usec = 0;

#ifdef CONFIG_ALL_LOGS	
	fprintf(g_logfp,"SetSocketSNDtimeout with time %d second\n",aTime);
#endif /*CONFIG_ALL_LOGS*/

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (void *)&timeout,
                sizeof(timeout)) < 0)
	{
#ifdef CONFIG_ALL_LOGS	
		fprintf(g_logfp,"SetSocketSNDtimeout setsockopt failed error %d\n",errno);
#endif /*CONFIG_ALL_LOGS*/
		return -1;
	}

	return 0;
}

/* Function to set recv timeout for Blocking socket */
int SetSocketRECVtimeout(int sock,unsigned int aTime)
{
	struct timeval timeout = {0,0};      
    timeout.tv_sec = aTime;
    timeout.tv_usec = 0;

#ifdef CONFIG_ALL_LOGS	
	fprintf(g_logfp,"SetSocketRECVtimeout with time %d second\n",aTime);
#endif /*CONFIG_ALL_LOGS*/

    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout,
                sizeof(timeout)) < 0)
	{
#ifdef CONFIG_ALL_LOGS	
		fprintf(g_logfp,"SetSocketRECVtimeout setsockopt failed error %d\n",errno);
#endif /*CONFIG_ALL_LOGS*/
		return -1;
	}

	return 0;
}
