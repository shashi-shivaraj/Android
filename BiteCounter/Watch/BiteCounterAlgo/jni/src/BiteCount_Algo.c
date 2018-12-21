/*******************************************************************************************************
*
*  FILE NAME	: BiteCount_Algo.c
*
*  DESCRIPTION  : Program to detect meal intake in humans by
*                 tracking wrist motion using gyroscope. 
*
*  DATE	               NAME					 REFERENCE			REASON
*  25-June-2018        Shashi Shivaraju      Initial code		BiteCounter
*******************************************************************************************************/

/*Header inclusion*/
#include "BiteCount_Sensor.h"

/*Macro Declarations*/

/*Algorithm thresholds*/
#define POSITIVE_VELOCITY_THRESHOLD	10	/* in degree */
#define NEGATIVE_VELOCITY_THRESHOLD -10  /* in degree */
#define INTRABITE_TIME_THRESHOLD 2			/* in second */
#define INTERBITE_TIME_THRESHOLD 8			/* in second */

#define SAMPLE_FREQUENCY 15

/*window size used to smooth the data*/
#define SMOOTH_WINDOW_SIZE	15

/*Function Prototypes*/

/*Function to detect bites consumed using gyroscope roll velocity*/	
static int DetectBites(FILE* Outfp,double* gyro_roll_data,double* raw_roll_data,
					double* timestamp,int total_samples,
					int start_index,int stop_index,FILE* fp_log);

#ifdef CONFIG_FILE_METHOD
#ifdef CONFIG_SMOOTH_DATA
/*Function to smooth the data using flat mean filter method*/
static void smooth_data(double* origional_data,double* smooth_data,double* raw_data,int samplesize,int win_size,FILE *fp);
#endif /*CONFIG_SMOOTH_DATA*/
#endif /*CONFIG_FILE_METHOD*/


int BiteCount_Algo(FILE *fp_log)
{
	
	FILE* fp = NULL;                /*File pointer for file operations*/
	double *gyro_roll_data = NULL,*smooth_gyro_roll_data = NULL,*raw_data =NULL; /*pointes  to store gyro data*/
	double *timestamps = NULL;
	double pitch = 0,yaw = 0,roll = 0;	/*variables for gyroscope data*/
	double time = 0;
	double running_sum = 0;
	int index = 0,start_index = 0 ,stop_index = 0;
	int ret = 0,total_samples = 0 /*,i = 0*/;
	char ch = 0;
	
		
	/*open input data file*/
	fp = fopen(DATA_FILE,"r");        /*open input image file provided as cmd line arg*/
	if(!fp)                         /*error handling*/
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]fopen failed for %s\n",DATA_FILE);/*failure to open the input file*/
#endif /*CONFIG_ALL_LOGS*/
		return -1;              /*return error code*/	
	}
	
	/*find number of samples in input data file*/
	while(!feof(fp))
	{
		ch = fgetc(fp);
		if(ch == '\n')
		{
			total_samples++;
		}
	}
	
	if(!total_samples)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]No data in file;total_samples = %d\n",total_samples);
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	else /*reset file pointer to start of file*/
	{
		/*first line is header*/
		total_samples = total_samples-1;
		
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Total Samples in file = %d\n",total_samples);
#endif /*CONFIG_ALL_LOGS*/
		ret = fseek(fp, 0, SEEK_SET);
		if(ret != 0)
		{
#ifdef CONFIG_ALL_LOGS
			fprintf(fp_log,"[BiteCountAlgo]fseek failed with %d\n",ret);
#endif /*CONFIG_ALL_LOGS*/
			goto CLEANUP;
		}
	}
	
	/*Allocate memory to store gyroscope data*/	
	gyro_roll_data = (double*)calloc(total_samples,sizeof(double));
	if(!gyro_roll_data)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Memory allocation failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	
	/*Allocate memory to store smooth gyroscope data*/	
	smooth_gyro_roll_data = (double*)calloc(total_samples,sizeof(double));
	if(!smooth_gyro_roll_data)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Memory allocation failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	
	raw_data = (double*)calloc(total_samples,sizeof(double));
	if(!raw_data)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Memory allocation failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	
	timestamps = (double*)calloc(total_samples,sizeof(double));
	if(!timestamps)
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Memory allocation failed\n");
#endif /*CONFIG_ALL_LOGS*/
		ret = -1;
		goto CLEANUP;
	}
	
	/*skip the header*/
	while(!feof(fp))
	{
		ch = fgetc(fp);
		if(ch == '\n')
		{
			break;
		}
	}
	
	/*read the gyroscope data from the file*/
	total_samples = 0;
	while(0<fscanf(fp,"%lf %lf %lf %lf",\
		&time,&pitch,&roll,&yaw))/*will exit at EOF or fscanf error*/
	{
		raw_data[total_samples] = roll;
		gyro_roll_data[total_samples] = (roll * 180)/M_PI; /*convert from radian/s to Degree/s*/
		timestamps[total_samples] = time;

#ifdef CONFIG_SMOOTH_DATA
		/*smooth the data*/
		if(SMOOTH_WINDOW_SIZE > total_samples)
		{
			smooth_gyro_roll_data[total_samples] =  gyro_roll_data[total_samples];
			running_sum = running_sum + gyro_roll_data[total_samples]; 
		}
		else
		{
			index =  total_samples - SMOOTH_WINDOW_SIZE;
			running_sum = running_sum - gyro_roll_data[index] + gyro_roll_data[total_samples];
			smooth_gyro_roll_data[total_samples] =  running_sum/SMOOTH_WINDOW_SIZE;
		}
#else 
		smooth_gyro_roll_data[total_samples] =  gyro_roll_data[total_samples];
		
#endif /*CONFIG_SMOOTH_DATA*/

#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log," index %d,raw roll %lf,roll %lf,smooth roll %f,timestamp %lf,sum %lf,index %d mod %d\n",
				total_samples,raw_data[total_samples],gyro_roll_data[total_samples],smooth_gyro_roll_data[total_samples],
				time,running_sum,index,(index)%SMOOTH_WINDOW_SIZE);
#endif /*CONFIG_ALL_LOGS*/
		
		total_samples ++;
	}
	
	start_index = 0;
	stop_index = total_samples;

#ifdef CONFIG_ALL_LOGS
	fprintf(fp_log,"[BiteCountAlgo]start index = %d\nstop index = %d\n",start_index,stop_index);
#endif /*CONFIG_ALL_LOGS*/

	/*close the file*/
	if(fp)
		fclose(fp);
	fp = NULL;
	
	
	
	/*smooth the gyro data*/
	//smooth_data(gyro_roll_data,smooth_gyro_roll_data,raw_data,total_samples,SMOOTH_WINDOW_SIZE,fp_log);
	
	/*open output result file*/
	fp = fopen(OUTPUT_FILE,"w");        /*open output file provided as cmd line arg*/
	if(!fp)                         /*error handling*/
	{
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]fopen failed for %s\n",OUTPUT_FILE);/*failure to open the ouput file*/
#endif /*CONFIG_ALL_LOGS*/
		return -1;              /*return error code*/	
	}
	
	/*Caluclate number of bites consumed using gyroscope data*/
	ret = DetectBites(fp,smooth_gyro_roll_data,gyro_roll_data,timestamps,total_samples,start_index,stop_index,fp_log);
	
CLEANUP:
	
	/*close the file*/
	if(fp)
		fclose(fp);
	fp = NULL;
	
	/*deallocate heap memory*/
	if(gyro_roll_data)
		free(gyro_roll_data);
	gyro_roll_data = NULL;
	
	if(raw_data)
		free(raw_data);
	raw_data = NULL;
	
	if(smooth_gyro_roll_data)
		free(smooth_gyro_roll_data);
	smooth_gyro_roll_data = NULL;
	
	if(timestamps)
		free(timestamps);
	timestamps = NULL;
	
	return ret;
}

/*Function with bitecounter algorithm*/	
static int DetectBites(FILE* Outfp,double* gyro_roll_data,double* raw_roll_data,
					double* timestamp,int total_samples,
					int start_index,int stop_index,FILE* fp_log)
{
	int total_bites = 0;
	int i = 0;		/*loop variable*/
	int event = 0;	/*flag variable to track events in algorithm*/
	double velocity = 0;  	/*gyro roll velocity*/
	double time_zero = 0,time_one = 0,time_two = 0; /*timestamps at whichs events occur*/
	
	
	for(i = start_index;i <= stop_index;i++) /*loop through the gryo roll velocity data*/
	{
		velocity = gyro_roll_data[i];	/*current sample roll velocity*/
		
#ifdef CONFIG_ALL_LOGS
		fprintf(fp_log,"[BiteCountAlgo]Zero event 0 at i = %d with velocity = %lf and time_zero = %lf raw_gyro = %lf\n",
					i,velocity,timestamp[i],raw_roll_data[i]);
#endif /*CONFIG_ALL_LOGS*/

		/*check if current roll velocity exceeds T1 (10 deg/s) and event is 0*/
		if(0 == event && velocity > POSITIVE_VELOCITY_THRESHOLD) 
		{
			event = 1;
			time_zero = timestamp[i];	/*Timestamp of Event 1*/
			
#ifdef CONFIG_ALL_LOGS
			fprintf(fp_log,"[BiteCountAlgo]First event 1 at i = %d with velocity = %lf and time_zero = %lf raw_gyro = %lf\n",
					i,velocity,time_zero,raw_roll_data[i]);
#endif /*CONFIG_ALL_LOGS*/

			i++;	/*increament sample index*/
			
			while(1 == event || 2  == event)	/*loop until event 3 is reached*/
			{
				velocity = gyro_roll_data[i];	/*current sample roll velocity*/
				time_one = timestamp[i];	/*Timestamp of current sample*/
				
#ifdef CONFIG_ALL_LOGS
				fprintf(fp_log,"[BiteCountAlgo]Event %d at i = %d with velocity = %lf and time_one = %lf raw_gyro = %lf\n",
						event,i,velocity,time_one,raw_roll_data[i]);
#endif /*CONFIG_ALL_LOGS*/						
				
				/*check if current roll velocity is lesser than T2 (-10 deg/s) and event is 1
				 * check if the time elapsed after event 1 is more than or equal to 2s*/
				if(1 == event  && velocity < NEGATIVE_VELOCITY_THRESHOLD && (time_one-time_zero) >= INTRABITE_TIME_THRESHOLD)
				{
					fprintf(Outfp,"%d\n",i); /*Bite Detected ;Output Sample index to file*/
					total_bites ++;
					event = 2;
					time_two = time_one;	/*Timestamp of Event 2*/
#ifdef CONFIG_ALL_LOGS
					fprintf(fp_log,"[BiteCountAlgo]BiteDetected event 2 at i = %d with velocity = %lf and time_two = %lf raw_gyro = %lf\n",
							i,velocity,time_two,raw_roll_data[i]);
#endif /*CONFIG_ALL_LOGS*/
	
				}	
				
				/*check if event is 2 and if the time elapsed after event 2 is more than or equal to 8s*/
				if(event == 2 && (time_one - time_two) >= INTERBITE_TIME_THRESHOLD)
				{
					
#ifdef CONFIG_ALL_LOGS
					fprintf(fp_log,"[BiteCountAlgo]RESET event 3 at i = %d with velocity = %lf and time = %lf raw_gyro = %lf\n",
							i,velocity,time_one,raw_roll_data[i]);
#endif /*CONFIG_ALL_LOGS*/

					event = 0;	/*Reset event counter*/
					break;		/*exit form while loop*/
				}
				
				i++;			/*increament sample index*/
						
				if(stop_index < i)	/*required samples processed*/
				{
					break;		/*exit from for loop*/
				}
			}
		}	
	}
	
	return total_bites;
}

#ifdef CONFIG_FILE_METHOD
#ifdef CONFIG_SMOOTH_DATA
/*Function to smooth the data using flat mean filter method*/
void smooth_data(double* origional_data,double* smooth_data,double* raw_data,int sample_size,int win_size,FILE *fp)
{
	int i = 0,j=0;	/*loop variables*/
	double sum = 0;
	
	for(i = 0; i < sample_size; i++)
	{
		if(i < win_size)
		{
			smooth_data[i] = origional_data[i];
		}
		else 
		{
			/* value at a current index is taken as the average of the
			 * previous values and the current value present in the window*/
			for(j = i;j > i-win_size;j--)
			{
				sum = sum + origional_data [j];
			}	
			smooth_data[i] = sum/win_size;
		}
		
#ifdef CONFIG_ALL_LOGS
		fprintf(fp,"raw data %lf, roll %lf,smooth roll %lf,sum %lf\n",raw_data[i],origional_data[i],smooth_data[i],sum);
#endif /*CONFIG_ALL_LOGS*/
		sum = 0;
	}
}
#endif /*CONFIG_SMOOTH_DATA*/
#endif /*CONFIG_FILE_METHOD*/

int isBite(double velocity,int* event,int index,
			double timestamp,double *time_zero,double *time_one,double *time_two,
			FILE *fp_log)
{
		if(0 == *event)
		{
#ifdef CONFIG_ALL_LOGS
			fprintf(fp_log,"Zero event 0 at i = %d with velocity = %lf and time_zero = %lf\n",
						index,velocity,timestamp);
#endif /*CONFIG_ALL_LOGS*/
			
			/*check if current roll velocity exceeds T1 (10 deg/s) and event is 0*/
			if(0 == *event && velocity > POSITIVE_VELOCITY_THRESHOLD) 
			{
				*event = 1;
				*time_zero = timestamp;	/*Timestamp of Event 1*/
			
#ifdef CONFIG_ALL_LOGS
			fprintf(fp_log,"First event 1 at i = %d with velocity = %lf and time_zero = %lf\n",
					index,velocity,*time_zero);
#endif /*CONFIG_ALL_LOGS*/

			}
		}
		else if(1 == *event || 2  == *event)
		{
			*time_one = timestamp;	/*Timestamp of current sample*/

#ifdef CONFIG_ALL_LOGS
			fprintf(fp_log,"Event %d at i = %d with velocity = %lf and time_one = %lf\n",
						*event,index,velocity,*time_one);
#endif /*CONFIG_ALL_LOGS*/

			/*check if current roll velocity is lesser than T2 (-10 deg/s) and event is 1
				 * check if the time elapsed after event 1 is more than or equal to 2s*/
				if(1 == *event  && velocity < NEGATIVE_VELOCITY_THRESHOLD && (*time_one-*time_zero) >= INTRABITE_TIME_THRESHOLD)
				{
					*event = 2;
					*time_two = *time_one;	/*Timestamp of Event 2*/
#ifdef CONFIG_ALL_LOGS
					fprintf(fp_log,"BiteDetected event 2 at i = %d with velocity = %lf and time_two = %lf\n",
							index,velocity,*time_two);
#endif /*CONFIG_ALL_LOGS*/
					return index;
	
				}	
				
				/*check if event is 2 and if the time elapsed after event 2 is more than or equal to 8s*/
				if(2 == *event && (*time_one - *time_two) >= INTERBITE_TIME_THRESHOLD)
				{
					
#ifdef CONFIG_ALL_LOGS
					fprintf(fp_log,"RESET event 3 at i = %d with velocity = %lf and time = %lf\n",
							index,velocity,*time_one);
#endif /*CONFIG_ALL_LOGS*/

					*event = 0;	/*Reset event counter*/
				}	
			}	
	
	return index+1;
}
