package open.source.LeanTracker;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;
import java.util.ArrayList;
import java.util.List;

import static android.content.ContentValues.TAG;

import com.droiduino.bluetoothconn.R;

public class MainActivity extends AppCompatActivity {

    private String deviceName = null;
    private String deviceAddress;
    public static Handler handler;
    public static BluetoothSocket mmSocket;
    public static ConnectedThread connectedThread;
    public static CreateConnectThread createConnectThread;
    public static int LAUNCH_SECOND_ACTIVITY = 1;

    private final static int CONNECTING_STATUS = 1; // used in bluetooth handler to identify message status
    private final static int MESSAGE_READ = 2; // used in bluetooth handler to identify message update
    private final static int UPLOAD = 3; // used in bluetooth handler to know when downloading

    private static String[] PERMISSIONS_LOCATION = {
        Manifest.permission.ACCESS_FINE_LOCATION,
        Manifest.permission.ACCESS_COARSE_LOCATION,
        Manifest.permission.BLUETOOTH_SCAN,
        Manifest.permission.BLUETOOTH_CONNECT,
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // UI Initialization
        final Button buttonConnect = findViewById(R.id.buttonConnect);
        final Toolbar toolbar = findViewById(R.id.toolbar);
        final ProgressBar progressBar = findViewById(R.id.progressBar);
        progressBar.setVisibility(View.GONE);

        final TextView textLog = findViewById(R.id.textLog);
        final TextView textFileSelected = findViewById(R.id.selected_file);
        final Button buttonSynchronize = findViewById(R.id.buttonSynchronize);
        final Button buttonListFile = findViewById(R.id.buttonSelectFile);
        final Button buttonOpenFile = findViewById(R.id.buttonReplayFile);
        final Button buttonDeleteFile = findViewById(R.id.buttonDeleteFile);
        final Button buttonRecapFile = findViewById(R.id.buttonRecapFile);

        buttonSynchronize.setEnabled(false);

        checkPermissions();

        // If a bluetooth device has been selected from SelectDeviceActivity
        deviceName = getIntent().getStringExtra("deviceName");
        if (deviceName != null){
            // Get the device address to make BT Connection
            deviceAddress = getIntent().getStringExtra("deviceAddress");
            // Show progree and connection status
            toolbar.setSubtitle("Connecting to " + deviceName + "...");
            progressBar.setVisibility(View.VISIBLE);
            buttonConnect.setEnabled(false);

            /*
            This is the most important piece of code. When "deviceName" is found
            the code will call a new thread to create a bluetooth connection to the
            selected device (see the thread code below)
             */
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            createConnectThread = new CreateConnectThread(bluetoothAdapter,deviceAddress);
            createConnectThread.start();
        }

        /*
        Second most important piece of Code. GUI Handler
         */
        handler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message msg){
                switch (msg.what){
                    case CONNECTING_STATUS:
                        switch(msg.arg1){
                            case 1:
                                toolbar.setSubtitle("Connected to " + deviceName);
                                progressBar.setVisibility(View.GONE);
                                buttonConnect.setEnabled(true);
                                buttonSynchronize.setEnabled(true);
                                break;
                            case -1:
                                toolbar.setSubtitle("Device fails to connect");
                                progressBar.setVisibility(View.GONE);
                                buttonConnect.setEnabled(true);
                                break;
                        }
                        break;

                    case MESSAGE_READ:
                        String arduinoMsg = msg.obj.toString(); // Read message from Arduino
                        if (arduinoMsg.indexOf("listfiles") == 0){
                            //textViewInfo.setText("Received files: ");
                            textLog.setText(arduinoMsg);
                        }
                        break;
                    case UPLOAD:
                        Log.e("HANDLER", "Receive the message");
                        switch(msg.arg1){
                            case 1:
                                Log.e("HANDLER", "Set visible");
                                progressBar.setVisibility(View.VISIBLE);
                                break;
                            case 0:
                                Log.e("HANDLER", "Set invisible");
                                progressBar.setVisibility(View.GONE);
                                break;
                        }
                        break;
                }
            }
        };

        // Select Bluetooth Device
        buttonConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Move to adapter list
                Intent intent = new Intent(MainActivity.this, SelectDeviceActivity.class);
                startActivity(intent);
            }
        });

        // Button to ON/OFF LED on Arduino Board
        buttonSynchronize.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String cmdText = "sendfiles";
                connectedThread.write(cmdText);

            }
        });

        buttonListFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, ListFiles.class);
                startActivityForResult(intent, LAUNCH_SECOND_ACTIVITY);
            }
        });

        buttonOpenFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String fileName = textFileSelected.getText().toString();
                if (fileName.equals("No file selected")){
                    textLog.setText("You have to select a file");
                }
                else{
                    Intent intent = new Intent(MainActivity.this, DrawTrackActivity.class);
                    intent.putExtra("file_name", fileName);
                    startActivity(intent);
                }
            }
        });

        buttonRecapFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String fileName = textFileSelected.getText().toString();
                if (fileName.equals("No file selected")){
                    textLog.setText("You have to select a file");
                }
                else{
                    Intent intent = new Intent(MainActivity.this, RecapFileActivity.class);
                    intent.putExtra("file_name", fileName);
                    startActivity(intent);
                }
            }
        });

        buttonDeleteFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String fileName = textFileSelected.getText().toString();
                if (fileName.equals("No file selected")){
                    textLog.setText("You have to select a file");
                }
                else{
                    textLog.setText("File " + fileName + "was deleted");
                    deleteFile(fileName);
                    textFileSelected.setText("No file selected");
                }
            }
        });
    }

    private void checkPermissions(){
        int permission2 = ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN);
        if (permission2 != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(
                    this,
                    PERMISSIONS_LOCATION,
                    1
            );
        }
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == LAUNCH_SECOND_ACTIVITY) {
            if(resultCode == Activity.RESULT_OK){
                String result=data.getStringExtra("result");
                Log.e("Returned file:", result);
                TextView textFileSelected = findViewById(R.id.selected_file);
                textFileSelected.setText(result);
            }
        }
    } //onActivityResult

    /* ============================ Thread to Create Bluetooth Connection =================================== */
    public class CreateConnectThread extends Thread {

        public CreateConnectThread(BluetoothAdapter bluetoothAdapter, String address) {
            /*
            Use a temporary object that is later assigned to mmSocket
            because mmSocket is final.
             */
            BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
            BluetoothSocket tmp = null;
            UUID uuid = bluetoothDevice.getUuids()[0].getUuid();

            try {
                /*
                Get a BluetoothSocket to connect with the given BluetoothDevice.
                Due to Android device varieties,the method below may not work fo different devices.
                You should try using other methods i.e. :
                tmp = device.createRfcommSocketToServiceRecord(MY_UUID);
                 */
                tmp = bluetoothDevice.createInsecureRfcommSocketToServiceRecord(uuid);

            } catch (IOException e) {
                Log.e(TAG, "Socket's create() method failed", e);
            }
            mmSocket = tmp;
        }

        public void run() {
            // Cancel discovery because it otherwise slows down the connection.
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            bluetoothAdapter.cancelDiscovery();
            try {
                // Connect to the remote device through the socket. This call blocks
                // until it succeeds or throws an exception.
                mmSocket.connect();
                Log.e("Status", "Device connected");
                handler.obtainMessage(CONNECTING_STATUS, 1, -1).sendToTarget();
            } catch (IOException connectException) {
                // Unable to connect; close the socket and return.
                try {
                    mmSocket.close();
                    Log.e("Status", "Cannot connect to device");
                    handler.obtainMessage(CONNECTING_STATUS, -1, -1).sendToTarget();
                } catch (IOException closeException) {
                    Log.e(TAG, "Could not close the client socket", closeException);
                }
                return;
            }

            // The connection attempt succeeded. Perform work associated with
            // the connection in a separate thread.
            connectedThread = new ConnectedThread(mmSocket);
            connectedThread.run();
        }

        // Closes the client socket and causes the thread to finish.
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "Could not close the client socket", e);
            }
        }
    }

    /* =============================== Thread for Data Transfer =========================================== */
    public class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public List<String> get_list_files() {
            byte[] buffer = new byte[1024];  // buffer store for the stream
            List<String> result = new ArrayList<>();
            //result.add("listfiles");
            int bytes = 0; // bytes returned from read()
            while (true) {
                try {
                    buffer[bytes] = (byte) mmInStream.read();
                    String readMessage;
                    if (buffer[bytes] == '\n'){
                        readMessage = new String(buffer,0,bytes);
                        Log.e("File name: ",readMessage);
                        if (readMessage.equals("EOF")) {
                            Log.e("Good to go: ",readMessage);
                            break; //We received all the files
                        }
                        result.add(readMessage);
                        bytes = 0;
                    } else {
                        bytes++;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
            return result;
        }

        public int download_file(String fileName){
            /*
            Download the requested file.
            Send the "rf namefile" command to the M5stack
            Get in return FS XXX for the file size, or ERROR.
            Then read XXX chars and write them locally

            The given filename is /xxx.bin
            The filename locally is xxx.bin
             */
            byte[] buffer = new byte[1024];
            int bytes = 0, file_size = 0;

            String out_call = "rf " + fileName;
            String local_name = fileName.substring(1);
            Log.e("out call: ",out_call);
            Log.e("local name : ",local_name);
            //Send the command to the M5
            byte[] b_out_Call = out_call.getBytes(); //converts entered String into bytes
            try {
                mmOutStream.write(b_out_Call);
            } catch (IOException e) {
                Log.e("Send Error","Unable to send message",e);
            }
            //Get the return of the command, Error or the file size
            while (true) {
                try {
                    buffer[bytes] = (byte) mmInStream.read();
                    String readMessage;
                    if (buffer[bytes] == '\n'){
                        readMessage = new String(buffer,0,bytes);
                        Log.e("Returned command: ",readMessage);
                        if (readMessage.equals("ERROR")) {
                            Log.e("No file received: ",readMessage);
                            return -1;
                        }
                        //Get the file size
                        file_size = Integer.parseInt(readMessage.substring(3));
                        Log.e("File size : ", String.valueOf(file_size));
                        bytes = 0;
                        break;
                    } else {
                        bytes++;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
            Log.e("Received bytes: ", "Going to DL");
            buffer = new byte[file_size];
            for (int i = 0; i < file_size; i++){
                try {
                    buffer[i] = (byte) mmInStream.read();
                    //Log.e("Received bytes: ",String.valueOf(i+1));
                    if (i%5000 == 4999) Log.e("Received bytes: ",String.valueOf(i+1));
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }

            //Write the result to the file
            try {
                FileOutputStream fileout = openFileOutput(local_name, MODE_PRIVATE);
                DataOutputStream outputWriter=new DataOutputStream (fileout);
                outputWriter.write(buffer);
                outputWriter.close();
                Log.e("Wrote file: ",local_name);
            } catch (Exception e) {
                e.printStackTrace();
            }

            return 0;
        }

        public void update_files(){
            /*
            Request the list of files from the Arduino.
            Check which files are already here
            Request the files that are not on the local storage yet
             */
            //Get the list of local files
            handler.obtainMessage(UPLOAD,1, 0).sendToTarget();

            String[] list_local_files = fileList();
            List<String> list_M5_files = get_list_files();
            //Now compare and download the files we do not have here.
            for (int i = 0; i < list_M5_files.size(); i++){
                boolean found = false;
                String curr_name = list_M5_files.get(i).substring(1);
                if (curr_name.equals("EOF")) continue;
                for (int j = 0; j < list_local_files.length; j++){
                    if (curr_name.equals(list_local_files[j])) {found = true; break;}
                }
                if (!found ){
                    Log.e("Download : ",list_M5_files.get(i));
                    download_file(list_M5_files.get(i));
                }
                else{
                    Log.e("File was found : ",list_M5_files.get(i));
                }
            }
            handler.obtainMessage(UPLOAD,0, 0).sendToTarget();
        }

        public void run() {
            byte[] buffer = new byte[1024];  // buffer store for the stream
            int bytes = 0; // bytes returned from read()
            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    /*
                    Read from the InputStream from Arduino until termination character is reached.
                    Then send the whole String message to GUI Handler.
                     */
                    buffer[bytes] = (byte) mmInStream.read();
                    String readMessage;
                    if (buffer[bytes] == '\n'){
                        readMessage = new String(buffer,0,bytes);
                        if (readMessage.equals("sending files")){
                            Log.e("Arduino Message",readMessage);
                            update_files();
                        }
                        else{
                            Log.e("Arduino Message",readMessage);
                            handler.obtainMessage(MESSAGE_READ,readMessage).sendToTarget();
                        }
                        bytes = 0;
                    } else {
                        bytes++;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(String input) {
            byte[] bytes = input.getBytes(); //converts entered String into bytes
            try {
                mmOutStream.write(bytes);
            } catch (IOException e) {
                Log.e("Send Error","Unable to send message",e);
            }
        }

        /* Call this from the main activity to shutdown the connection */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) { }
        }
    }

    /* ============================ Terminate Connection at BackPress ====================== */
    @Override
    public void onBackPressed() {
        // Terminate Bluetooth Connection and close app
        if (createConnectThread != null){
            createConnectThread.cancel();
        }
        Intent a = new Intent(Intent.ACTION_MAIN);
        a.addCategory(Intent.CATEGORY_HOME);
        a.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(a);
    }
}
