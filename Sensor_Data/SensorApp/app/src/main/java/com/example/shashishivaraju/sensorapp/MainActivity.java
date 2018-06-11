/*******************************************************************************************************
*
*  FILE NAME	: MainActivity.java
*
*  DESCRIPTION  : Android Application code to capture sensor data.
*
*  DATE	               NAME					 REFERENCE			REASON
*  27-May-2018         Shashi Shivaraju      Initial code		AOSP
*******************************************************************************************************/
package com.example.shashishivaraju.sensorapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("Sensors");
        System.loadLibrary("SensorsJNI");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        int ret = 0;
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /*initialize logging*/
        ret = init_logging();
        if(ret != 0)
        {
            Toast.makeText(getApplicationContext(), "Logging NOT supported:\nPROVIDE STORAGE PERMISSION FOR APP ",
                    Toast.LENGTH_LONG).show();
        }




        final Button start_bt = findViewById(R.id.start_button);
        final Button stop_bt = findViewById(R.id.stop_button);
        final Button exit_bt = findViewById(R.id.exit_button);

        start_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                start_bt.setClickable(false);
                int ret = init_logging();
                if(ret != 0)
                {
                    Toast.makeText(getApplicationContext(), "Logging NOT supported",
                            Toast.LENGTH_LONG).show();
                }

                /*Check if device supports accelerometer and gyroscope */
                ret = check_sensor_capabilities();
                if(ret != 0)
                {
                    Toast.makeText(getApplicationContext(), "Sensors NOT supported",
                            Toast.LENGTH_LONG).show();
                }

                /*Capture data from accelerometer and gyroscope*/
                ret = capture_sensor_data();
                if(ret != 0)
                {
                    Toast.makeText(getApplicationContext(), "Failure to capture sensor data",
                            Toast.LENGTH_LONG).show();
                }

            }
        });


        stop_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                stop_bt.setClickable(false);
                start_bt.setClickable(true);
                /*Stop accelerometer and gyroscope*/
                int ret = stop_sensors();
                if(ret != 0)
                {
                    Toast.makeText(getApplicationContext(), "Failure to stop sensors",
                            Toast.LENGTH_LONG).show();
                }

                /*deinitialize logging*/
                deinit_logging();
            }
        });

        exit_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                Toast.makeText(getApplicationContext(), "Exit button pressed",
                        Toast.LENGTH_LONG).show();

                System.exit(0);
            }
        });

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int init_logging();
    public native void deinit_logging();
    public native int  check_sensor_capabilities();
    public native int  capture_sensor_data();
    public native int  stop_sensors();

}

