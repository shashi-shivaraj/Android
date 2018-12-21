/*******************************************************************************************************
 *
 *  FILE NAME	: DisplayCountActivity.java
 *
 *  DESCRIPTION : Implements the  activity of the Bite Counter WearOS app which
 *                displays the current bite count of the session.
 *
 *  PLATFORM    : Android
 *
 *  DATE	               NAME					 REFERENCE			REASON
 *  2nd July,2018          Shashi Shivaraju      Initial code		BiteCounter
 *******************************************************************************************************/

package com.icountbites;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.view.WindowManager;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.content.Context;
import android.os.Build;


import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import android.net.Uri;
import android.media.RingtoneManager;


public class DisplayCountActivity extends MainActivity {

    public static final String EXTRA_MESSAGE = "com.icountbites.MESSAGE";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display_count);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        /*pass the java handles to native layer*/
        set_ActivityContext();

        final Button stop_bt = findViewById(R.id.stop_button);
        final TextView BCount = findViewById(R.id.count_txt);
        final TextView BMesg = findViewById(R.id.textView_Bites);
        if(false == displayState)
        {
            BCount.setVisibility(View.INVISIBLE);
            BMesg.setText("Bites ON");
        }
        else
        {
            BCount.setVisibility(View.VISIBLE);
            BMesg.setText("Bites");
        }

        stop_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                String message = "";

                if(device_state.active == status)/*valid session*/
                {
                    EndSession();
                    status = device_state.idle;

                    message = "Total Bites : "+Integer.toString(BiteCount);
                }
                else if(device_state.timedout == status)
                {
                    message = "Timed Out";
                }

                Intent intent = new Intent(getBaseContext(),EndActivity.class);
                intent.putExtra(EXTRA_MESSAGE, message);
                startActivity(intent);
                finish();
            }
        });
    }

    @Override
    public void onResume(){
        super.onResume();
        final TextView disp_msg = findViewById(R.id.count_txt);
        int count = getCurrentBiteCount();
        disp_msg.setText(Integer.toString(count));
    }

    private void EndSession() {

        /*Stop gyroscope*/
        int ret = stop_bite_detection();
        if(ret < 0)
        {
            Toast.makeText(getApplicationContext(), "Failure to stop sensors",
                    Toast.LENGTH_LONG).show();
        }

        BiteCount = ret;

        /*Get current system date/time*/
        Stop_TS_String = new SimpleDateFormat("yyyy:MM:dd:HH:mm:ss").format(new java.util.Date());

        /*Get timestamp of session stop*/
        StopEvent_TS = Get_EventTimeStamp(
                (Integer.parseInt(Stop_TS_String.substring(0,4))-2000),
                Integer.parseInt(Stop_TS_String.substring(5,7)),
                Integer.parseInt(Stop_TS_String.substring(8,10)),
                Integer.parseInt(Stop_TS_String.substring(11,13)),
                Integer.parseInt(Stop_TS_String.substring(14,16)),
                Integer.parseInt(Stop_TS_String.substring(17,19))
        );

        SessionDuration = StopEvent_TS - StartEvent_TS;

        /*Construct the data to be sent to server*/
        GenerateDataString();

    }

    /* A function called from JNI to end the session after max meal duration*
     */
    private void ForceEndSession(){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final TextView BCount = findViewById(R.id.count_txt);
                final TextView BMesg = findViewById(R.id.textView_Bites);

                BCount.setVisibility(View.INVISIBLE);
                BMesg.setText("session timedout");
                BMesg.setTextColor(Color.RED);
                EndSession();
                status = device_state.timedout;

                /*disable always on screen after timeout*/
                getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                /*set display to ambient mode*/
                setAmbientEnabled();
            }
        });
    }

    /*
     * A function called from JNI to update current bite count
     */
    private void UpdateBiteCount(int aCount){
        BiteCount = aCount;
        final TextView disp_msg = findViewById(R.id.count_txt);
        final TextView Bites_txt = findViewById(R.id.textView_Bites);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                disp_msg.setText(Integer.toString(BiteCount));
                if(BiteCount >= alarmThreshold && alarmState == true)
                {
                    disp_msg.setTextColor(Color.RED);
                    Bites_txt.setTextColor(Color.RED);

                    if(isAudioSupported) {

                        if(BiteCount == alarmThreshold) /*vibrate device for first time*/
                        {
                            /*vibrate the device*/
                            Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                            // Vibrate for 2000 milliseconds
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                v.vibrate(VibrationEffect.createOneShot(1000,VibrationEffect.DEFAULT_AMPLITUDE));
                            }else{
                                //deprecated in API 26
                                v.vibrate(1000);
                            }
                        }

                        /*play sound for each bite*/
                        try {
                            playSound(getApplicationContext());
                        } catch ( Exception e ) {
                            e.printStackTrace();
                        }
                    }
                    else
                    {
                        /*Audio not supported use vibrator*/
                        /*vibrate the device*/
                        Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                        // Vibrate for 2000 milliseconds
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                            v.vibrate(VibrationEffect.createOneShot(1000,VibrationEffect.DEFAULT_AMPLITUDE));
                        }else{
                            //deprecated in API 26
                            v.vibrate(1000);
                        }
                    }
                }
            }
        });
    }

    public void GenerateDataString()
    {
        /*Generate the key for server*/
        DataString = "device_" + SerialNumber + ",Duration,Bites,";
        DataString = DataString + "Year,M,D,Time,Duration,Steps,Goal Bite Count,Goal Step Count,";
        DataString = DataString + "Alarm Mode,Alarm Count,Live Display Mode,Display Modes Active\n";

        /*concatenate the midnight session data*/
        DataString = DataString + MidnightStart_TS + "," + 0 + "," + 0 + ",";
        DataString = DataString + MidnightDateTime[0] + "," + MidnightDateTime[1] + "," + MidnightDateTime[2] + ",";
        DataString = DataString + "00:00:00"+",";
        DataString = DataString + 0 + ":" + 0 + ":" + 0 + ",";
        DataString = DataString + steps + "," + bite_goal + "," + step_goal + ",";
        if( true == alarmState )
            DataString = DataString + "2,";
        else
            DataString = DataString + "0,";

        DataString = DataString + alarmThreshold + ",";

        if(true == displayState)
            DataString = DataString + "1,";
        else
            DataString = DataString + "2,";

        DataString = DataString + displayModesActive + "\n";

        /*concatenate the session data*/
        DataString = DataString + StartEvent_TS + "," + SessionDuration + "," + BiteCount + ",";
        DataString = DataString + TS_String.substring(0,4) + "," + TS_String.substring(5,7) + "," + TS_String.substring(8,10) + ",";
        DataString = DataString + TS_String.substring(11,19)+",";
        DataString = DataString + SessionDuration/3600 + ":" + SessionDuration/60 + ":" + SessionDuration%60 + ",";
        DataString = DataString + steps + "," + bite_goal + "," + step_goal + ",";
        if( true == alarmState )
            DataString = DataString + "2,";
        else
            DataString = DataString + "0,";

        DataString = DataString + alarmThreshold + ",";

        if(true == displayState)
            DataString = DataString + "1,";
        else
            DataString = DataString + "2,";

        DataString = DataString + displayModesActive + "\n";

    }

    public void playSound(Context context) throws IllegalArgumentException,
            SecurityException,
            IllegalStateException,
            IOException {

        Uri soundUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        MediaPlayer mMediaPlayer = new MediaPlayer();
        mMediaPlayer.setDataSource(context, soundUri);
        final AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        if (audioManager.getStreamVolume(AudioManager.STREAM_ALARM) != 0) {
            mMediaPlayer.setAudioStreamType(AudioManager.STREAM_ALARM);
            mMediaPlayer.prepare();
            mMediaPlayer.start();
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void set_ActivityContext();
    public native int getCurrentBiteCount();
}
