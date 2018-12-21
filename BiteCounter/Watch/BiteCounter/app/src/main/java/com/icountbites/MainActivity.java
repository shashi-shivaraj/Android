/*******************************************************************************************************
 *
 *  FILE NAME	: MainActivity.java
 *
 *  DESCRIPTION : Implements the main activity of the Bite Counter WearOS app
 *                which provide UI for user to start a new session or view settings
 *                page.
 *
 *  PLATFORM    : Android
 *
 *  DATE	               NAME					 REFERENCE			REASON
 *  2nd July,2018          Shashi Shivaraju      Initial code		BiteCounter
 *******************************************************************************************************/

package com.icountbites;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.wearable.activity.WearableActivity;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.widget.ImageButton;
import android.widget.Toast;
import android.os.Build;
import android.os.Environment;
import android.provider.Settings.Secure;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.UUID;

enum device_state{
    idle,
    active,
    timedout
}


public class MainActivity extends WearableActivity {

    /*Version of the release for play store*/
    private static final String Version = "Bite_Counter_v_1_0_0";
    private static final int REQUEST_PERMISSIONS = 0x01;
    public static boolean Storage_Permission = false;
    public static boolean Internet_Permission = false;
    public static boolean isAudioSupported = false;
    public static device_state   status =  device_state.idle;
    public static int BiteCount = 0;
    /*Timestamp in seconds since Jan 1,2000 12:00 AM when session was started*/
    public static int StartEvent_TS = 0;
    /*Timestamp in seconds since Jan 1,2000 12:00 AM when midnight occured*/
    public static int MidnightStart_TS = 0;
    public static int MidnightDateTime[];
    public static int StopEvent_TS  = 0;
    public static int SessionDuration = 0;
    public static Boolean alarmState = false;
    public static Boolean displayState = true;
    public static int alarmThreshold = 30; /* disabled by default */
    public static String SerialNumber = "000000000";
    public static String TS_String;
    public static String Stop_TS_String;
    public static int SerialLen = 9;
    public static boolean Serial_Permission = false;
    String[] PERMISSIONS = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_PHONE_STATE,Manifest.permission.INTERNET};
    public static final String HostURL = "http://bc.ces.clemson.edu/upload/eventData";
    public static String DataString = " ";
    public int steps = 0,bite_goal = 100,step_goal = 10000,displayModesActive = 0;
    public final UUID APP_UUID = UUID.fromString("7881177e-284f-4f1c-a1f4-81d55f5055fe");

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("BiteCounterAlgo");
        System.loadLibrary("BiteCounterJNI");
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Enables Always-on
        setAmbientEnabled();

        final ImageButton start_bt = findViewById(R.id.start_button);
        final ImageButton info_bt  = findViewById(R.id.info_btn);


        /*check for storage permission*/
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED
                ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE)
                        != PackageManager.PERMISSION_GRANTED ) {

            Storage_Permission=false;
            Serial_Permission = false;
            Internet_Permission = false;
            // Permission is not granted
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                Toast.makeText(getApplicationContext(), "Provide Permissions for App ",
                        Toast.LENGTH_LONG).show();
            }
            else if(ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.READ_PHONE_STATE)) {
                Toast.makeText(getApplicationContext(), "Provide Permissions for App ",
                        Toast.LENGTH_LONG).show();
            }
            else if(ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.INTERNET)) {
                Toast.makeText(getApplicationContext(), "Provide Permissions for App ",
                        Toast.LENGTH_LONG).show();
            }
            else {

                ActivityCompat.requestPermissions(this,
                        PERMISSIONS,
                        REQUEST_PERMISSIONS);
            }
        }
        else
        {
            Storage_Permission = true;
            Serial_Permission = true;
            Internet_Permission = true;
        }

      /*
        //Utility code to get screen dimensions in pixels
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int width = size.x;
        int height = size.y;

        Toast.makeText(getApplicationContext(), width+"x"+height,
                Toast.LENGTH_LONG).show();*/


        info_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                if(!Storage_Permission)
                {
                    Toast.makeText(getApplicationContext(), "STORAGE PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                else if(!Serial_Permission)
                {
                    Toast.makeText(getApplicationContext(), "Device Info PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                else if(!Internet_Permission)
                {
                    Toast.makeText(getApplicationContext(), "Internet PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                else
                {
                    GetSerialNumber();
                    Intent intent = new Intent(getBaseContext(),settings.class);
                    startActivity(intent);
                    finish();
                }
            }
        });

        start_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                if(!Storage_Permission)
                {
                    Toast.makeText(getApplicationContext(), "STORAGE PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                else if(!Serial_Permission)
                {
                    Toast.makeText(getApplicationContext(), "Device Info PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                if(!Internet_Permission)
                {
                    Toast.makeText(getApplicationContext(), "Internet PERMISSION Essential for usage",
                            Toast.LENGTH_LONG).show();
                }
                else
                {
                    /*get the device serial number */
                    GetSerialNumber();

                    /*read config file if stored*/
                    readConfigFile();

                    /*check if the device supports audio*/
                    isAudioSupported = CheckAudioSupport();


                    File directory = getApplicationContext().getExternalFilesDir(null);
                    File LogFile = new File(directory, "BC_log.txt");

                    int ret = init_logging(LogFile.getAbsolutePath());
                    if(ret != 0)
                    {
                        Toast.makeText(getApplicationContext(), "Logging NOT supported",
                                Toast.LENGTH_LONG).show();
                    }

                    /*Check if device supports gyroscope */
                    ret = check_sensor_capabilities();
                    if(ret != 0)
                    {
                        Toast.makeText(getApplicationContext(), "Sensors NOT supported",
                                Toast.LENGTH_LONG).show();
                    }

                    /*move to display activity*/
                    Intent intent = new Intent(getBaseContext(),DisplayCountActivity.class);
                    startActivity(intent);


                    /*Get current system date/time*/
                     TS_String = new SimpleDateFormat("yyyy:MM:dd:HH:mm:ss").format(new java.util.Date());


                    /*Get timestamp of session start*/
                    StartEvent_TS = Get_EventTimeStamp(
                            (Integer.parseInt(TS_String.substring(0,4))-2000),
                            Integer.parseInt(TS_String.substring(5,7)),
                            Integer.parseInt(TS_String.substring(8,10)),
                            Integer.parseInt(TS_String.substring(11,13)),
                            Integer.parseInt(TS_String.substring(14,16)),
                            Integer.parseInt(TS_String.substring(17,19))
                    );



                        /*Get timestamp of midnight i.e. next day start*/
                        MidnightStart_TS = 24*60*60 + Get_EventTimeStamp(
                                (Integer.parseInt(TS_String.substring(0, 4)) - 2000),
                                Integer.parseInt(TS_String.substring(5, 7)),
                                Integer.parseInt(TS_String.substring(8, 10)),
                                0,
                                0,
                                0
                        ) ;

                    MidnightDateTime = DateFromTimestamp(MidnightStart_TS);
                    /*generate year in 4 digit form*/
                    MidnightDateTime[0] = MidnightDateTime[0]+2000;

                    /*Capture data from gyroscope and detect bites consumed*/
                    ret = start_bite_detection();
                    if(ret != 0)
                    {
                        Toast.makeText(getApplicationContext(), "Failure to capture sensor data",
                                Toast.LENGTH_LONG).show();
                    }

                    status = device_state.active;

                    finish();
                }
            }
        });
    }

    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case REQUEST_PERMISSIONS: {
                // If request is cancelled, the result arrays are empty.
                if(grantResults.length == 3)
                {
                    if(grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                        Storage_Permission = true;

                        Log.i("Main Activity", "App Version = "+Version);

                        File directory = getApplicationContext().getExternalFilesDir(null);
                        File LogFile = new File(directory, "BC_log.txt");

                        /*initialize logging*/
                        int ret = init_logging(LogFile.getAbsolutePath());
                        if(ret != 0)
                        {
                            Toast.makeText(getApplicationContext(), "Logging NOT supported:\nPROVIDE STORAGE PERMISSION FOR APP ",
                                Toast.LENGTH_LONG).show();
                        }

                        /*store default config in the config file*/
                        writeConfigtoFile();
                    }
                    else{
                        Storage_Permission = false;
                    }

                    if(grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                        Serial_Permission = true;
                    }
                    else{
                        Serial_Permission = false;
                    }

                    if(grantResults[2] == PackageManager.PERMISSION_GRANTED) {
                        Internet_Permission = true;
                    }
                    else{
                        Internet_Permission = false;
                    }

                } else {
                    Storage_Permission = false;
                    Serial_Permission = false;
                    Internet_Permission = false;
                }
            }
            break;
        }
    }


    public void onTaskRemoved(Intent rootIntent){
        /*deinitialize logging*/
        deinit_logging();

        status = device_state.idle;
    }

    public void readConfigFile(){
        boolean Storage_State = isExternalStorageWritable();
        if(Storage_State == true)
        {
            try {
                File directory = getApplicationContext().getExternalFilesDir(null);
                File Config = new File(directory, "BC_config.txt");

                FileInputStream ConfigFile = new FileInputStream(Config);
                BufferedReader  Reader = new BufferedReader(new InputStreamReader(ConfigFile));

                /*1st line is serial number*/
                String line = Reader.readLine();
                for (String val: line.split(":", 2)) {
                }

                /*2nd line is Bite Display*/
                line = Reader.readLine();
                for (String val: line.split(":", 2)) {
                    if (val.equals("On"))
                        displayState = true;
                    else
                        displayState = false;
                }

                /*3rd line is Alarm*/
                line = Reader.readLine();
                for (String val: line.split(":", 2)) {
                    if (val.equals("On"))
                        alarmState = true;
                    else
                        alarmState = false;
                }

                if(alarmState == true)
                {
                    line = Reader.readLine();
                    for (String val: line.split(":", 2)) {
                        if (!val.equals("Alarm Threshold")) {
                            alarmThreshold = Integer.parseInt(val);
                        }
                    }
                }

                ConfigFile.close();

            }catch ( Exception e )
            {
                e.printStackTrace();
            };
        }
    }

    public void writeConfigtoFile(){

        String SN_header  = "Serial Number:";
        String Display_header = "Display:";
        String Alarm_header = "Alarm:";
        String Alarm_Thres = "Alarm Threshold:";
        String Nextline = "\n";

        boolean Storage_State = isExternalStorageWritable();
        if(Storage_State == true)
        {
            try {
                File directory = getApplicationContext().getExternalFilesDir(null);
                File Config = new File(directory, "BC_config.txt");

                FileOutputStream ConfigFile = new FileOutputStream(Config);

                ConfigFile.write(SN_header.getBytes());
                ConfigFile.write(SerialNumber.getBytes());
                ConfigFile.write(Nextline.getBytes());

                ConfigFile.write(Display_header.getBytes());
                if(true == displayState)
                {
                    ConfigFile.write("On".getBytes());
                }
                else
                {
                    ConfigFile.write("Off".getBytes());
                }
                ConfigFile.write(Nextline.getBytes());

                ConfigFile.write(Alarm_header.getBytes());
                if(true == alarmState)
                {
                    ConfigFile.write("On".getBytes());
                    ConfigFile.write(Nextline.getBytes());
                }
                else
                {
                    ConfigFile.write("Off".getBytes());
                    ConfigFile.write(Nextline.getBytes());
                }
                ConfigFile.write(Alarm_Thres.getBytes());
                ConfigFile.write(Integer.toString(alarmThreshold).getBytes());
                ConfigFile.write(Nextline.getBytes());

                ConfigFile.close();

            }catch ( Exception e )
            {
                e.printStackTrace();
            };
        }
    }

    /* Checks if external storage is available for read and write */
    public boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }

    /*Check if the device supports audio*/
    public boolean CheckAudioSupport()
    {
        PackageManager packageManager = getApplicationContext().getPackageManager();
        AudioManager audioManager = (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);

        // Check whether the device has a speaker.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M &&
                // Check FEATURE_AUDIO_OUTPUT to guard against false positives.
                packageManager.hasSystemFeature(PackageManager.FEATURE_AUDIO_OUTPUT)) {
            AudioDeviceInfo[] devices = audioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS);
            for (AudioDeviceInfo device : devices) {
                if (device.getType() == AudioDeviceInfo.TYPE_BUILTIN_SPEAKER) {
                    return true;
                }
            }
        }
        return false;
    }

    public void GetSerialNumber(){

        String Serial;
        int index = 0;

        /*Gets the hardware serial number, if available else returns "unknown"*/
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Serial = Build.getSerial();
        } else {
            Serial = Build.SERIAL;
        }

        /*serial number unavailable;get android id*/
        if(Serial.equals("unknown"))
        {
            Serial = Secure.getString(getContentResolver(), Secure.ANDROID_ID);
        }

        /*Server requires device ID to be 9 digits*/
        if(SerialLen < Serial.length())
        {
            index = Serial.length() - SerialLen;
            SerialNumber = Serial.substring(index);
        }
        else if(SerialLen > Serial.length())
        {
            index = SerialLen - Serial.length();
            SerialNumber = Serial;
            while(SerialLen != SerialNumber.length()) {
                SerialNumber = SerialNumber + Integer.toString(index);
                index = index - 1;
            }
        }
    }

    /* compute the time of the device in seconds, since Jan 1 2000 12:00am */
    /* the month and day are 1-indexed to match how TI's processor stores them in its registers, so device code and this code match exactly */
    public int Get_EventTimeStamp( int RTCYEARL,        /* 0...256 */
                   int    RTCMON,            /* 1...12 */
                   int    RTCDAY,            /* 1...31 */
                   int    RTCHOUR,        /* 0...23 */
                   int    RTCMIN,            /* 0...59 */
                   int    RTCSEC)            /* 0...59 */
    {
        int   days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        int   j;
        int   elapsed;

        elapsed=0;
        for (j=0; j<RTCYEARL; j++)
            elapsed+=31536000;
        j=(RTCYEARL+3)/4;
        elapsed+=(j*86400);
        for (j=1; j<RTCMON; j++)
            elapsed+=(days[j-1]*86400);
        if (RTCYEARL%4 == 0  &&  RTCMON > 2)
            elapsed+=86400;
        elapsed+=((RTCDAY-1)*86400);
        elapsed+=(RTCHOUR*3600);
        elapsed+=(RTCMIN*60);
        elapsed+=RTCSEC;
        return(elapsed);
    }

    /* convert a seconds-since-Jan1,2000 timestamp into date components */
   public int[] DateFromTimestamp(int timestamp)


    {
         int    total_seconds;
         int     Year;         /* 0...256 (can be added to 2000 to get 4-digit year) */
         int     Month;          /* 0...11 */
         int     Day;         /* 0...30 */
         int     Hour;          /* 0...23 */
         int     Min;          /* 0...59 */
         int     Sec;          /* 0...59 */
         int    days_in_month[]={31,28,31,30,31,30,31,31,30,31,30,31};
         int    YYYYMMDDhhmmss[] = {0,0,0,0,0,0};

        total_seconds=timestamp;
        Year=0;
        while (total_seconds >= 31536000)
        {
            if ((Year)%4 == 0)
            {
                if (total_seconds < 31536000+86400)
                    break;
                total_seconds-=(31536000+86400);
            }
            else
                total_seconds-=(31536000);

            (Year)++;
        }

        if ((Year)%4 == 0)
        days_in_month[1]=29;    /* leap year */

        Month=1; /*Month starts from 1*/
        while (total_seconds >= days_in_month[Month-1]*86400)
        {
            total_seconds-=(days_in_month[Month-1]*86400);
            (Month)++;
        }

        Day=1; /*Days starts from 1*/
        while (total_seconds >= 86400)
        {
            total_seconds-=(86400);
            (Day)++;
        }
        Hour=(total_seconds/3600);
        total_seconds-=((Hour)*3600);
        Min=(total_seconds/60);
        Sec=(total_seconds%60);

        YYYYMMDDhhmmss[0] = Year;
        YYYYMMDDhhmmss[1] = Month;
        YYYYMMDDhhmmss[2] = Day;
        YYYYMMDDhhmmss[3] = Hour;
        YYYYMMDDhhmmss[4] = Min;
        YYYYMMDDhhmmss[5] = Sec;

        return YYYYMMDDhhmmss;
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int  init_logging(String logfile);
    public native void deinit_logging();
    public native int  check_sensor_capabilities();
    public native int  start_bite_detection();
    public native int  stop_bite_detection();
}
