package open.source.LeanTracker;

import static android.content.ContentValues.TAG;
import static java.lang.Float.max;
import static java.lang.Float.sum;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.droiduino.bluetoothconn.R;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Timer;

public class RecapFileActivity extends Activity {

    private ConstraintLayout recap_file_layout;

    String fileName;
    TextView textViewGeneralOverview;
    //DataPoint[] list_data_points;
    DataPointList datapointlist;
    //SingleLap[] list_laps;
    ArrayList<SingleLap> list_laps = new ArrayList<SingleLap>();

    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_draw_track);
        setContentView(R.layout.activity_recap_file);

        //Get the parameters from the calling activity
        Intent intent = getIntent();
        fileName = intent.getStringExtra("file_name");
        //Open the list of points
        datapointlist = new DataPointList();
        try {
            FileInputStream fileDPL = openFileInput(fileName);
            datapointlist.read_input_file(fileDPL);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        }

        // initializing our view.
        textViewGeneralOverview = findViewById(R.id.textViewGeneralOverview);

        recap_file();
        extract_lap_time();

    }

    public int intTime2Seconds(int input){
        //Return in second the current time
        int seconds = (input/100)%100;
        int minutes = (input/10000)%100;
        int hours = (input/1000000)%100;
        return 3600*hours+60*minutes+seconds;
    }

    public int diff2Times(int input1, int input2){
        //Return the difference in seconds between the two given time
        int t1 = intTime2Seconds(input1);
        int t2 = intTime2Seconds(input2);
        if (t1 < t2) return t2-t1;
        return t1-t2;
    }

    public void recap_file(){
        /*
        Take all the points in the list of point, and create a string that contains all the
        information.
         */
        float max_speed = 0, mean_speed = 0, max_lean = 0;
        int running_time;

        for (int i = 0; i < datapointlist.list_data_points.length; i++){
            max_speed = max(max_speed, datapointlist.list_data_points[i].speed);
            mean_speed += datapointlist.list_data_points[i].speed;
            max_lean = max(max_lean, max(datapointlist.list_data_points[i].roll, -datapointlist.list_data_points[i].roll));
        }
        mean_speed /= datapointlist.list_data_points.length;
        running_time = diff2Times(datapointlist.list_data_points[0].time,
                                  datapointlist.list_data_points[datapointlist.list_data_points.length-1].time);

        String summary;
        summary = "File:" + fileName + "\n" + "Running time: " + running_time + "s\n";
        summary += "Maximum speed: " + max_speed + "Km/h\n" + "Mean speed: " + mean_speed + "Km/h\n";
        summary += "Max lean angle: " + max_lean + "\n";

        Log.e("To display: ",summary);
        textViewGeneralOverview.setText(summary);
    }

    public void extract_lap_time(){
        int curr_index = 0, next_index = 0;
        boolean new_lap_found = false;

        while(curr_index < datapointlist.list_data_points.length-1){
            SingleLap curr_lap = new SingleLap();
            while (!curr_lap.found_lap && curr_index < datapointlist.list_data_points.length-1) {
                curr_lap.compute_lap_time(datapointlist.list_data_points, curr_index, 30, 20);
                if (!curr_lap.found_lap) curr_index++;
            }
            if (!curr_lap.found_lap) break;
            list_laps.add(curr_lap);
            Log.e("To display: ", "Lap found: " + curr_lap.start_index + "-" + curr_lap.end_index);
            curr_index = curr_lap.end_index;
        }
        Log.e("To display: ", "Number of lap found: " + list_laps.size());
    }


}
