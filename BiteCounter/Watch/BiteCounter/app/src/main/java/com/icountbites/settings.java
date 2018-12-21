/*******************************************************************************************************
 *
 *  FILE NAME	: settings.java
 *
 *  DESCRIPTION : Implements the  activity of the Bite Counter WearOS app which
 *                displays settings page.
 *
 *  PLATFORM    : Android
 *
 *  DATE	               NAME					 REFERENCE			REASON
 *  2nd July,2018          Shashi Shivaraju      Initial code		BiteCounter
 *******************************************************************************************************/

package com.icountbites;

import android.content.Intent;
import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

public class settings extends MainActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        // Enables Always-on
        setAmbientEnabled();

        final Switch alarm_switch = findViewById(R.id.alarm_switch);
        final Switch display_switch = findViewById(R.id.Disp_Switch);
        final TextView alarm_txt = findViewById(R.id.bite_txt);
        final TextView deviceID = findViewById(R.id.Device_txt);
        final EditText alarm_count = findViewById(R.id.bite_count);

        deviceID.setText("ID :"+SerialNumber);

        if(alarmState == false)
        {
            alarm_txt.setVisibility(View.INVISIBLE);
            alarm_count.setVisibility(View.INVISIBLE);
            alarm_switch.setChecked(false);
        }
        else
        {
            alarm_txt.setVisibility(View.VISIBLE);
            alarm_count.setVisibility(View.VISIBLE);
            alarm_count.setText(Integer.toString(alarmThreshold));
            alarm_switch.setChecked(true);
        }

        if(displayState == false)
        {
            display_switch.setChecked(false);
        }
        else
        {
            display_switch.setChecked(true);
        }

        display_switch.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                displayState = display_switch.isChecked();
            }
        });

        alarm_switch.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                alarmState = alarm_switch.isChecked();
                if(alarmState == false)
                {
                    alarm_txt.setVisibility(View.INVISIBLE);
                    alarm_count.setVisibility(View.INVISIBLE);
                }
                else
                {
                    alarm_txt.setVisibility(View.VISIBLE);
                    alarm_count.setVisibility(View.VISIBLE);
                }

            }
        });

        final Button ok_bt = findViewById(R.id.ok_button);
        ok_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                alarmThreshold = Integer.parseInt(alarm_count.getText().toString());
                writeConfigtoFile();
                Intent intent = new Intent(getBaseContext(),MainActivity.class);
                startActivity(intent);
                finish();
            }
        });
    }
}
