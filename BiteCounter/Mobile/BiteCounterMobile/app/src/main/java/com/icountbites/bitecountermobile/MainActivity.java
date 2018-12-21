/*******************************************************************************************************
 *
 *  FILE NAME	: MainActivity.java
 *
 *  DESCRIPTION : Implements the  main activity of
 *                the Bite Counter Android Mobile app.
 *
 *  PLATFORM    : Android
 *
 *  DATE	               NAME					 REFERENCE			REASON
 *  2nd July,2018          Shashi Shivaraju      Initial code		BiteCounter
 *******************************************************************************************************/

package com.icountbites.bitecountermobile;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Iterator;
import java.util.Set;
import java.util.UUID;

import javax.net.ssl.HttpsURLConnection;

import static android.provider.Settings.NameValueTable.NAME;

public class MainActivity extends AppCompatActivity {

    /*Version of the release for play store*/
    private static final String Version = "Bite_Counter_Mobile_v_1_0_0";
    public static boolean Storage_Permission = false;
    public static boolean Bluetooth_State = false;
    public static boolean isAppActive = true;
    String[] PERMISSIONS = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
    private static final int REQUEST_PERMISSIONS = 0x01;
    private static final int REQUEST_ENABLE_BT = 0x02;
    public final UUID APP_UUID = UUID.fromString("7881177e-284f-4f1c-a1f4-81d55f5055fe");
    private static String BC_Website = "http://bc.ces.clemson.edu/device/";
    public static  String readBTMessage ;
    public static  String mBTDeviceID = "000000000";
    TextView WatchID = null;
    public static final String HostURL = "http://bc.ces.clemson.edu/upload/eventData";


    public BluetoothAdapter mBluetoothAdapter;
    public static Thread BTSocketThread;
    public static BTAcceptConnection mBTConnection;

    Handler handler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ActionBar actionBar = getSupportActionBar();
        ImageButton viewGraph = (ImageButton)findViewById(R.id.graph_bt);
        if(WatchID == null)
            WatchID = findViewById(R.id.WatchID);

        try {
            actionBar.hide();
        } catch ( NullPointerException e ) {
            System.out.println("actionBar might be NULL\n");
        }

        /*check for storage permission*/
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {

            Storage_Permission = false;
            // Permission is not granted
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                Toast.makeText(getApplicationContext(), "Provide Permissions for App ",
                        Toast.LENGTH_LONG).show();
            } else {

                ActivityCompat.requestPermissions(this,
                        PERMISSIONS,
                        REQUEST_PERMISSIONS);
            }
        } else {
            Storage_Permission = true;
        }

        /*read the wearOS device ID from the config file*/
        if(Storage_Permission)
        {
            readConfigFile();
        }

        WatchID.setText("Watch ID : " + mBTDeviceID);

        handler = new Handler(callback);

        /*Get Bluetooth Adapter*/
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            Log.e("Main Activity", "Bluetooth Not Supported");
        }
        else
        {
            /*Check if bluetooth is enabled*/
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            } else {
                Bluetooth_State = true;
                /*Start the bluetooth server*/
                StartBTServer();
            }
        }


        viewGraph.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(BC_Website+mBTDeviceID+"/view"));
                startActivity(browserIntent);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();

        /*This will be called when the user clicks on
        * home screen or view graphs*/

        /*clear this activity*/
        if(Storage_Permission && Bluetooth_State)
        {
            finish();
            System.exit(0);
        }

    }

    Handler.Callback callback = new Handler.Callback() {
        public boolean handleMessage(Message msg) {

            /*got message to update the device id*/
            String DeviceID;
            DeviceID = "Watch ID : " + mBTDeviceID;
            WatchID.setText(DeviceID);
            return true;
        }
    };


    public BluetoothDevice GetBTPairedDevices()
    {

        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        BluetoothDevice WearableDevice;

        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address
                Log.d("Main Activity","Device = "+deviceName+" with MAC address = "+deviceHardwareAddress);

                if(device.getBluetoothClass().getMajorDeviceClass() == BluetoothClass.Device.Major.WEARABLE)
                {
                    Log.d("End Activity","Found Mobile Device = "+deviceName+" with MAC address = "+deviceHardwareAddress);
                    WearableDevice = device;
                    return WearableDevice;
                }
            }
        }
        else
        {
            Log.e("Main Activity","Watch not paired with mobile");
            Toast.makeText(getApplicationContext(), "Pair your WearOS watch with Android Mobile ",
                    Toast.LENGTH_LONG).show();
        }

        return null;
    }

    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case REQUEST_PERMISSIONS: {
                // If request is cancelled, the result arrays are empty.
                if(grantResults.length == 1)
                {
                    if(grantResults[0] == PackageManager.PERMISSION_GRANTED)
                    {
                        Log.i("Main Activity", "App Version = "+Version);
                        Storage_Permission = true;
                        /*initialise the config file once the storage permission is granted*/
                        writeConfigtoFile();
                    }
                    else
                        {
                            Storage_Permission = false;
                        }
                }
                else
                    {
                        Storage_Permission = false;
                    }
            }
            break;
        }
    }

    protected void onActivityResult(int requestCode, int resultCode,
                                    Intent data)
    {
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_OK) {
                Bluetooth_State = true;
                /*Start the bluetooth server*/
                StartBTServer();
            }
            else
            {
                Bluetooth_State = false;
            }
        }
    }

    /*Start the bluetooth server*/
    public void StartBTServer()
    {
        BluetoothDevice mWearable = GetBTPairedDevices();
        if(mWearable == null)
        {
            Log.e("Main Activity","Watch not paired with mobile");
        }
        else
        {
            mBTConnection = new BTAcceptConnection();
            BTSocketThread = new Thread(mBTConnection);
            BTSocketThread.start();
        }
    }


    private class BTAcceptConnection implements Runnable {

        private final BluetoothServerSocket mmServerSocket;
        private  InputStream mmInStream;
        public byte[] mmBuffer;

        public BTAcceptConnection() {

            // Use a temporary object that is later assigned to mmServerSocket
            // because mmServerSocket is final.
            BluetoothServerSocket tmp = null;
            try {
                // MY_UUID is the app's UUID string, also used by the client code.
                tmp = mBluetoothAdapter.listenUsingRfcommWithServiceRecord(NAME, APP_UUID);
            } catch (IOException e) {
                Log.e("BTThread", "Socket's listen() method failed", e);
            }
            mmServerSocket = tmp;
        }

        public void run() {

            BluetoothSocket socket = null;
            int numBytes = 0;

            // Keep listening until exception occurs or a socket is returned.
            while (true && isAppActive == true) {
                try {
                    socket = mmServerSocket.accept();
                } catch (IOException e) {
                    Log.e("BTThread", "Socket's accept() method failed", e);
                    break;
                }

                if (socket != null) {

                    // A connection was accepted. Perform work associated with
                    // the connection in a separate thread.
                    InputStream tmpIn = null;
                    try {
                        tmpIn = socket.getInputStream();
                    } catch (IOException e) {
                        Log.e("BTThread", "Error occurred when creating input stream", e);
                    }

                    mmInStream = tmpIn;

                    mmBuffer = new byte[1024];

                    // Keep listening to the InputStream until an exception occurs.
                    while (true && isAppActive == true) {
                        try {
                            // Read from the InputStream.
                           numBytes = mmInStream.read(mmBuffer);

                        } catch (IOException e) {
                            Log.e("BTThread", "Input stream was disconnected", e);
                            break;
                        }

                        if(numBytes != 0)
                        {
                            int ret = 0;
                            readBTMessage = new String(mmBuffer,0,numBytes);
                            if(numBytes > 16)
                            {
                                if(!mBTDeviceID.equals(readBTMessage.substring(7,16)))
                                {
                                    mBTDeviceID = readBTMessage.substring(7,16);
                                    writeConfigtoFile();

                                    handler.sendEmptyMessage(0);
                                }

                                /*send the data to the server*/
                                new UploadDatatoServer().execute();
                            }

                            break;
                        }
                    }


                    try {
                        socket.close();
                    } catch (IOException e) {
                        Log.e("BTThread", "Could not close the connect socket", e);
                    }
                    socket = null;

                }
            }
        }

        // Closes the connect socket and causes the thread to finish.
        public void cancel() {
            try {
                if(mmServerSocket != null)
                    mmServerSocket.close();
            }catch (IOException e )
            {
                Log.e("BTThread", "Socket's close() method failed", e);
            }
        }
    }

    /* Checks if external storage is available for read and write */
    public boolean isExternalStorageWritable()
    {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state))
        {
            return true;
        }
        return false;
    }

    /*read the config file*/
    public void readConfigFile()
    {
        boolean Storage_State = isExternalStorageWritable();
        if(Storage_State)
        {
            try {
                File directory = getApplicationContext().getExternalFilesDir(null);
                File Config = new File(directory, "BCMobile_config.txt");

                FileInputStream ConfigFile = new FileInputStream(Config);
                BufferedReader Reader = new BufferedReader(new InputStreamReader(ConfigFile));

                /*1st line is serial number*/
                String line = Reader.readLine();
                mBTDeviceID = line.substring(14,23);

                ConfigFile.close();

            }catch ( Exception e )
            {
                e.printStackTrace();
            };
        }
    }

    /*read the config file*/
    public void writeConfigtoFile(){

        String SN_header  = "Serial Number:";

        boolean Storage_State = isExternalStorageWritable();
        if(Storage_State == true)
        {
            try {
                File directory = getApplicationContext().getExternalFilesDir(null);
                File Config = new File(directory, "BCMobile_config.txt");

                FileOutputStream ConfigFile = new FileOutputStream(Config);

                ConfigFile.write(SN_header.getBytes());
                ConfigFile.write(mBTDeviceID.getBytes());

                ConfigFile.close();

            }catch ( Exception e )
            {
                e.printStackTrace();
            };
        }
    }


    /*code the upload the data to server*/
    public class UploadDatatoServer extends AsyncTask<String, Void, String> {

        protected void onPreExecute()
        {

        }

        protected String doInBackground(String... arg0)
        {
            try{

                URL url = new URL(HostURL);

                JSONObject postDataParams = new JSONObject();
                postDataParams.put("data", readBTMessage);
                Log.d("HTTP request params",postDataParams.toString());

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

                        Log.d("Main Activity","Both 200OK and DB-Post-Success=true recieved! = " + dbStatus);

                        /*display the update to app*/
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getApplicationContext(), "Uploaded Data to Server Successfully",
                                        Toast.LENGTH_LONG).show();
                            }
                        });

                        conn.disconnect();
                        return new String("UploadDatatoServer success");
                    } else {
                        Log.e("Main Activity","DB-Post-Success not found!");
                        conn.disconnect();

                        /*display the update to app*/
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getApplicationContext(), "Failed to Upload Data to Server",
                                        Toast.LENGTH_LONG).show();
                            }
                        });

                        return new String("UploadDatatoServer failed due to DB-Post-Success");
                    }
                }
                else {
                        conn.disconnect();

                    /*display the update to app*/
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(getApplicationContext(), "Failed to Upload Data to Server",
                                    Toast.LENGTH_LONG).show();
                        }
                    });
                        return new String("Uploading Data to Server Failed : "+responseCode);
                }


            }
            catch(Exception e){

                /*display the update to app*/
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(getApplicationContext(), "Failed to Upload Data to Server",
                                Toast.LENGTH_LONG).show();
                    }
                });

                return new String("Exception: " + e.getMessage());
            }

        }

        @Override
        protected void onPostExecute(String result)
        {
            Log.d("Main Activity","onPostExecute : " +result);
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

}