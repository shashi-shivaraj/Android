/*******************************************************************************************************
 *
 *  FILE NAME	: EndActivity.java
 *
 *  DESCRIPTION : Implements the  activity of the Bite Counter WearOS app which
 *                displays session summary and UI to access main activity or settings activity.
 *
 *  PLATFORM    : Android
 *
 *  DATE	               NAME					 REFERENCE			REASON
 *  2nd July,2018          Shashi Shivaraju      Initial code		BiteCounter
 *******************************************************************************************************/

package com.icountbites;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.text.SimpleDateFormat;
import java.util.Iterator;
import java.util.Set;

import javax.net.ssl.HttpsURLConnection;

public class EndActivity extends MainActivity {

    public BluetoothAdapter mBluetoothAdapter;
    public static BluetoothDevice mBTMobileDevice;
    public static BluetoothSocket mBTSocket;
    public static OutputStream mBTOutStream;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_end);
        setAmbientEnabled();

        Intent intent = getIntent();
        String message = intent.getStringExtra(DisplayCountActivity.EXTRA_MESSAGE);

        final TextView fin_msg = findViewById(R.id.total_bites_txt);
        final TextView status_msg = findViewById(R.id.complete_txt);
        fin_msg.setText(message);

        final Button end_bt = findViewById(R.id.OK_btn);

        if((device_state.idle == status || device_state.timedout == status) && (0 < BiteCount))/*valid session*/
        {
            end_bt.setVisibility(View.INVISIBLE);
            // call AsynTask to perform network operation on separate thread
            new HttpAsyncTask().execute(HostURL);
        }
        else
        {
            status_msg.setText("Session Complete");
        }

        end_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                /*close the BT connection*/
                if(mBTSocket != null) {
                    try {
                        mBTSocket.close();
                    } catch ( IOException closeException ) {
                        Log.e("End Activity", "Could not close the client socket", closeException);
                    }
                }

                /*deinitialize logging*/
                deinit_logging();

                Intent intent = new Intent(getBaseContext(),MainActivity.class);
                startActivity(intent);
                finish();
            }
        });

        final ImageButton settings_bt = findViewById(R.id.settings);
        settings_bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                /*close the BT connection*/
                if(mBTSocket != null) {
                    try {
                        mBTSocket.close();
                    } catch ( IOException closeException ) {
                        Log.e("End Activity", "Could not close the client socket", closeException);
                    }
                }

                /*deinitialize logging*/
                deinit_logging();

                Intent intent = new Intent(getBaseContext(),settings.class);
                startActivity(intent);
                finish();
            }
        });
    }

    public boolean isWifiConnected(){
        WifiManager wifi = (WifiManager)getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        if (wifi.isWifiEnabled())
        {
            Log.d("End Activity", "Device connected to Wifi");
            return true;
        }
        else
        {
            Log.d("End Activity", "Device not connected to Wifi");
            return false;
        }
    }

    public boolean isBluetoothConnected(){

        /*Get Bluetooth Adapter*/
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            Log.e("End Activity", "Bluetooth Not Supported");
            return false;
        }
        else
        {
            /*Check if bluetooth is enabled*/
            if (!mBluetoothAdapter.isEnabled())
            {
                Log.d("End Activity", "Bluetooth is not Enabled");
                return false;
            }
            else
            {
                Log.d("End Activity", "Bluetooth is Enabled transmit data to phone");
                return true;
            }
        }

    }

    public int BluetoothSendData()
    {
        mBTMobileDevice = GetBTPairedMobileDevice();
        if(mBTMobileDevice != null)
        {
            BluetoothSocket tmp = null;
            try {
                // Get a BluetoothSocket to connect with the given BluetoothDevice.
                // MY_UUID is the app's UUID string, also used in the server code.
                tmp = mBTMobileDevice.createRfcommSocketToServiceRecord(APP_UUID);
            } catch (IOException e) {
                Log.e("End Activity", "Socket's create() method failed", e);
                return -1;
            }
            mBTSocket = tmp;

            try {
                // Connect to the remote device through the socket. This call blocks
                // until it succeeds or throws an exception.
                mBTSocket.connect();
            } catch (IOException connectException) {
                // Unable to connect; close the socket and return.
                try {
                    mBTSocket.close();
                } catch (IOException closeException) {
                    Log.e("End Activity", "Could not close the client socket", closeException);
                }
                return -1;
            }

            // The connection attempt succeeded. Perform work associated with
            // the connection in a separate thread.

            OutputStream tmpOut = null;
            try {
                tmpOut = mBTSocket.getOutputStream();
            } catch (IOException e) {
                Log.e("End Activity", "Error occurred when creating output stream", e);
            }

            mBTOutStream = tmpOut;

            if(null !=  mBTOutStream)
            {
                try {
                    mBTOutStream.write(DataString.getBytes());
                } catch (IOException e) {
                    Log.e("End Activity", "Error occurred when sending data", e);
                }
            }

            return 0;
        }
        return -1;
    }


    public BluetoothDevice GetBTPairedMobileDevice()
    {

        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        BluetoothDevice Mobiledevice;

        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address
                Log.d("End Activity","Device = "+deviceName+" with MAC address = "+deviceHardwareAddress);

                if(device.getBluetoothClass().getMajorDeviceClass() == BluetoothClass.Device.Major.PHONE)
                {
                    Log.d("End Activity","Found Mobile Device = "+deviceName+" with MAC address = "+deviceHardwareAddress);
                    Mobiledevice = device;
                    return Mobiledevice;
                }
            }

            Log.e("End Activity","Watch not paired with Mobile device");
            return null;

        }
        else
        {
            Log.e("End Activity","Watch not paired with any devices");
            return null;
        }

    }

    private class HttpAsyncTask extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... urls) {
            return POST(urls[0]);
        }

        // onPostExecute displays the results of the AsyncTask.
        @Override
        protected void onPostExecute(String result) {
            final Button end_bt = findViewById(R.id.OK_btn);
            final TextView status_msg = findViewById(R.id.complete_txt);
            end_bt.setVisibility(View.VISIBLE);
            status_msg.setText("Session Complete");
        }
    }

    public String POST(String host) {
        File directory = getApplicationContext().getExternalFilesDir(null);
        File Config = new File(directory, "BC_DataBackup.txt");

        boolean isStoretoFile = false;

        /*upload the details to server*/

        if(isWifiConnected()) /*check if connected to wifi/cellular data*/
        {
            int ret = 0;

            ret = sendViaHTTP(DataString);
            if(ret != 0)
            {
                Log.e("End Activity", "sendViaHTTP failed");
                isStoretoFile = true;
            }
            else
            {
                /*Current record uploaded successfully
                * Now upload the backup data from file*/
                if(Config.exists() && !Config.isDirectory())
                {
                    String BackupFilePath = Config.getAbsolutePath();
                    ret = UploadBackupData(BackupFilePath);
                    if(ret != 0)
                    {
                        Log.e("End Activity", "UploadBackupData failed");
                    }
                }
            }

        }
        else if(isBluetoothConnected())/*check if bluetooth is on and watch is paired with mobile*/
        {
            int ret = 0;
            isStoretoFile = true;
            ret = BluetoothSendData();
            if(ret != 0)
            {
                Log.e("End Activity", "BluetoothSendData failed" + ret);
            }
        }
        else
        {
            isStoretoFile = true;
            Log.d("End Activity", "Storing the data to file");
        }

        /* UploadToServer native function is being used to store the record in backup file */
        /*UploadToServer has raw socket implementation to upload data to server
         *which is not being used.Instead we are using HttpURLConnection */
        if(isStoretoFile)
            UploadToServer(host,DataString,Config.getAbsolutePath(),1);

        return host;
    }

    public int sendViaHTTP(String Data)
    {
        try{
                URL url = new URL(HostURL);

                JSONObject postDataParams = new JSONObject();
                postDataParams.put("data", Data);

                 HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                 conn.setReadTimeout(15000 /* milliseconds */);
                 conn.setConnectTimeout(15000 /* milliseconds */);
                 conn.setRequestMethod("POST");
                 conn.setDoInput(true);
                 conn.setDoOutput(true);


                 OutputStream os = conn.getOutputStream();
                 BufferedWriter writer = new BufferedWriter(
                                                new OutputStreamWriter(os, "UTF-8"));
                 writer.write(getPostDataString(postDataParams));

                 writer.flush();
                 writer.close();
                 os.close();

                /*check for server response*/
                int responseCode=conn.getResponseCode();
                if (responseCode == HttpsURLConnection.HTTP_OK)
                {
                    /*server reponded with 200OK*/
                    /*Check for  DB-Post-Success in response header */
                    String dbStatus = conn.getHeaderField("DB-Post-Success");
                    if (dbStatus.equals("true")) {
                    Log.d("End Activity","Both 200OK and DB-Post-Success=true recieved! = " + dbStatus);

                    conn.disconnect();
                    return 0;
                } else {
                    Log.e("End Activity","DB-Post-Success not found!");
                    conn.disconnect();
                    return -1;
                }
            }
            else {
                    Log.e("End Activity","200OK not returned!");
                    conn.disconnect();
                    return -1;
            }
        }
        catch(Exception e){
                    Log.e("End Activity","Exception: " + e.getMessage());
                    return -1;
        }
    }


    public String getPostDataString(JSONObject params) throws Exception {

        StringBuilder result = new StringBuilder();
        boolean first = true;

        Iterator<String> itr = params.keys();

        while(itr.hasNext()){

            String key= itr.next();
            Object value = params.get(key);

            if (first)
                first = false;
            else
                result.append("&");

            result.append(URLEncoder.encode(key, "UTF-8"));
            result.append("=");
            result.append(URLEncoder.encode(value.toString(), "UTF-8"));

        }
        return result.toString();
    }

    public int UploadBackupData(String DataFile)
    {
        try
        {
            BufferedReader br = new BufferedReader(new FileReader(DataFile));
            String Record = "";
            String st = "";
            int count  = 0;
            int ret = 0;
            boolean failure = false;

            while ((st = br.readLine()) != null)
            {
                count ++;
                Record = Record.concat(st+"\n");
                if(0 == (count % 3) && !failure) /*every session record stored in backupfile is made up of three lines*/
                {
                    Log.d("End Activity", "String to  be uploaded is =: " +Record);

                    ret = sendViaHTTP(Record);
                    if(ret != 0)
                    {
                        Log.e("End Activity", "sendViaHTTP failed");
                        failure = true;
                    }
                    else
                    {
                        Record = "";
                    }

                }
            }

            /*check failure status and determine to write the backup file*/
            FileWriter fstream = new FileWriter(DataFile, false);
            if(failure)
            {
                fstream.write(Record);
                fstream.flush();
            }

            fstream.close();
        }
        catch(Exception e) {
            Log.e("End Activity", "UploadBackupData Exception: " + e.getMessage());
            return -1;
        }

        return 0;
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int UploadToServer(String HostURL,String Data,String File,int isStoretoFile);
}