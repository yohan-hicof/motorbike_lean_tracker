package open.source.LeanTracker;

import static java.lang.Integer.max;
import static java.lang.Integer.min;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.droiduino.bluetoothconn.R;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Timer;
import java.util.TimerTask;

class DataPoint implements Serializable {
    float pitch;
    float pitch_abs;
    float roll;
    float roll_abs;
    float acceleration;
    float speed;
    float direction;
    double lat;
    double lng;
    double lat_scaled;
    double lng_scaled;
    int date;
    int time;
}

class SingleLap implements Serializable {
    boolean found_lap = false;
    float max_speed = 0;
    float mean_speed = 0;
    float max_lean = 0;
    double lap_approximation = 0; //The minimum distance between the two point defining the lap
    int lap_time = 0;
    int start_index, end_index;
    String lap_time_string = "";


    public int int2Seconds(int input){
        int nb_seconds = 0;
        nb_seconds += (input/100)%100;
        nb_seconds += 60*((input/10000)%100);
        nb_seconds += 3600*((input/1000000)%100);
        return nb_seconds;
    }
    public int int2CentiSeconds(int input){
        int nb_centi_seconds = input%100;
        int nb_seconds = (input/100)%100;
        int nb_minutes = (input/10000)%100;
        int nb_hours = (input/1000000)%100;
        int retour = 360000*nb_hours + 6000*nb_minutes + 100*nb_seconds + nb_centi_seconds;
        return retour;
    }
    public String seconds2String(int input){
        int nb_hours = input/3600;
        input -= nb_hours*3600;
        int nb_minutes = input/60;
        input -= nb_minutes*60;
        int nb_seconds = input;
        String retour = nb_hours + ":" + nb_minutes + ":" + nb_seconds;
        return retour;
    }
    public String centiSeconds2String(int input){
        int nb_hours = input/360000;
        input -= nb_hours*360000;
        int nb_minutes = input/6000;
        input -= nb_minutes*6000;
        int nb_seconds = input/100;
        int nb_centi_second = input%100;
        String retour = nb_hours + ":" + nb_minutes + ":" + nb_seconds + ":" + nb_centi_second;
        return retour;
    }
    public double distance_two_pt_accurate(DataPoint pt1, DataPoint pt2){
        // Return the distance in meter between the two coordinates

        final int R = 6371; // Radius of the earth

        double latDistance = Math.toRadians(pt2.lat - pt1.lat);
        double lonDistance = Math.toRadians(pt2.lng - pt1.lng);
        double a = Math.sin(latDistance / 2) * Math.sin(latDistance / 2)
                + Math.cos(Math.toRadians(pt1.lat)) * Math.cos(Math.toRadians(pt2.lat))
                * Math.sin(lonDistance / 2) * Math.sin(lonDistance / 2);
        double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
        double distance = R * c * 1000; // convert to meters
        //Log.e("Computed distance: ", "Distance: " + distance);
        return distance;
    }

    public double distance_two_pt_fast(DataPoint pt1, DataPoint pt2){
        // Return the distance in meter between the two coordinates
        // This is a faster version for gaining a bit of time.
        final int R = 6371; // Radius of the earth

        double X = (Math.toRadians(pt2.lng) - Math.toRadians(pt1.lng)) * Math.cos(0.5* (Math.toRadians(pt2.lat) + Math.toRadians(pt1.lat)));
        double Y = Math.toRadians(pt2.lat) - Math.toRadians(pt1.lat);
        double d = 1000 * R * Math.sqrt(X*X + Y*Y);
        //Log.e("Computed distance: ", "Distance: " + d);
        return d;
    }
    public boolean compute_lap_time(DataPoint[] list_data_points, int first_index, double max_dist, double min_dist){
               

        int curr_index = first_index+1;
        double best_dist = 100*max_dist, curr_dist;
        int best_index = first_index;

        if (curr_index >= list_data_points.length) return false;

        //As long as we are to close from the starting point
        while (curr_index < list_data_points.length){
            curr_dist = distance_two_pt_fast(list_data_points[first_index], list_data_points[curr_index]);
            if (curr_dist > min_dist) break;
            curr_index++;
        }
        //Check if we found a point at least min_wait second later.
        if (curr_index >= list_data_points.length) return false;
        //Now move until we find a point that is close enough to the first_index
        while (curr_index < list_data_points.length){
            //No need to compute accurate at these distances
            /*if (fast) curr_dist = distance_two_pt_fast(list_data_points[first_index], list_data_points[curr_index]);
            else curr_dist = distance_two_pt_accurate(list_data_points[first_index], list_data_points[curr_index]);*/

            curr_dist = distance_two_pt_fast(list_data_points[first_index], list_data_points[curr_index]);

            if (curr_dist < max_dist) {//We found a potential position
                //fast = false;
                if (curr_dist < best_dist){best_dist = curr_dist; best_index = curr_index;}
            }
            //We found a close position, but are not close anymore, so we stop here
            else if (best_index != first_index) break;

            if (curr_dist > 10*max_dist) curr_index+=4; //Move a bit faster if we are very far
            curr_index++;
        }
        if (best_index == first_index) return false;
        //We found a lap
        found_lap = true;
        lap_approximation = best_dist;
        //Now fill the values
        for(int i = first_index; i < best_index; i++){
            max_speed = Float.max(list_data_points[i].speed, max_speed);
            mean_speed += list_data_points[i].speed;
            max_lean = Float.max(list_data_points[i].roll, max_lean);
        }

        //lap_time = int2Seconds(list_data_points[best_index].time) - int2Seconds(list_data_points[first_index].time);
        lap_time = int2CentiSeconds(list_data_points[best_index].time) - int2CentiSeconds(list_data_points[first_index].time);
        lap_time_string = centiSeconds2String(lap_time);
        mean_speed /= (best_index-first_index);
        //Save the start and end position. Use to find other laps
        start_index = first_index;
        end_index = best_index;

        return true;
    }
}

class DataPointList{

    DataPoint[] list_data_points = new DataPoint[0];

    public static double reverse_double(double x) {
        return ByteBuffer.allocate(8)
                .order(ByteOrder.BIG_ENDIAN).putDouble(x)
                .order(ByteOrder.LITTLE_ENDIAN).getDouble(0);
    }

    public static float reverse_float(float x) {
        return ByteBuffer.allocate(4)
                .order(ByteOrder.BIG_ENDIAN).putFloat(x)
                .order(ByteOrder.LITTLE_ENDIAN).getFloat(0);
    }

    public static int reverse_int(int x) {
        return ByteBuffer.allocate(4)
                .order(ByteOrder.BIG_ENDIAN).putInt(x)
                .order(ByteOrder.LITTLE_ENDIAN).getInt(0);
    }

    public void read_input_file(FileInputStream fileIn){

        try {
            long fileSize = fileIn.getChannel().size();
            int nb_data_points = (int)(fileSize/44);

            //Log.e("DPL File size: ", String.valueOf(fileSize));
            //Log.e("DPL Number of points: ", String.valueOf(nb_data_points));

            DataInputStream inputReader =new DataInputStream (fileIn);
            //Init the different array that will get the data
            list_data_points = new DataPoint[nb_data_points];
            //We have to reverse what we read because of the different endian between the two systems
            for (int i = 0; i < nb_data_points; i++){
                list_data_points[i] = new DataPoint();
                list_data_points[i].pitch = reverse_float(inputReader.readFloat());
                list_data_points[i].roll = reverse_float(inputReader.readFloat());
                list_data_points[i].acceleration = reverse_float(inputReader.readFloat());
                list_data_points[i].speed = reverse_float(inputReader.readFloat());
                list_data_points[i].direction = reverse_float(inputReader.readFloat());
                list_data_points[i].lat = reverse_double(inputReader.readDouble());
                list_data_points[i].lng = reverse_double(inputReader.readDouble());
                list_data_points[i].date = reverse_int(inputReader.readInt());
                list_data_points[i].time = reverse_int(inputReader.readInt());
            }
            inputReader.close();
        } catch (Exception e) {
            e.printStackTrace();
        }

        //Scale the latitude and longitude between 0-1w
        if (list_data_points.length == 0)return;
        double min_lat = list_data_points[0].lat, max_lat = list_data_points[0].lat;
        double min_lng = list_data_points[0].lng, max_lng = list_data_points[0].lng;
        double max_delta;
        for (int i = 1; i < list_data_points.length; i++){
            min_lat = Double.min(min_lat, list_data_points[i].lat);
            max_lat = Double.max(max_lat, list_data_points[i].lat);
            min_lng = Double.min(min_lng, list_data_points[i].lng);
            max_lng = Double.max(max_lng, list_data_points[i].lng);
        }
        max_delta = Double.max(max_lat-min_lat, max_lng-min_lng);

        for (int i = 0; i < list_data_points.length; i++){
            list_data_points[i].lat_scaled = (list_data_points[i].lat-min_lat)/max_delta;
            list_data_points[i].lng_scaled = (list_data_points[i].lng-min_lng)/max_delta;
            //We just want the positive values.
            if (list_data_points[i].roll < 0) list_data_points[i].roll_abs = -list_data_points[i].roll;
            else list_data_points[i].roll_abs = list_data_points[i].roll;
            if (list_data_points[i].pitch < 0) list_data_points[i].pitch_abs = -list_data_points[i].pitch;
            else list_data_points[i].pitch_abs = list_data_points[i].pitch;
        }
        //Try to interpolate the data point time.
        //The issue is that the GPS always return HH:MM:SS:00, never any tenth of a second.
        //So if we have 5 points with the same second, the first one is 00, then 20, then 40...
        for (int i = 0; i < list_data_points.length-1; i++){
            int end_index = i, nb_same;
            while (list_data_points[i].time == list_data_points[end_index+1].time && end_index < list_data_points.length-2)
                end_index++;
            if (i==end_index) continue;
            nb_same = end_index-i+1;
            //Log.e("Nb same ", String.valueOf(nb_same));
            for (int j = i+1; j <= end_index; j++){
                double cento = 100.0*(j-i)/nb_same;
                //Log.e("Old time ", String.valueOf(list_data_points[j].time));
                list_data_points[j].time += (int)cento;
                //Log.e("New time ", String.valueOf(list_data_points[j].time));
            }
            i = end_index+1;
        }

        //Log 100 data points to see if interpolation works
        for (int i = list_data_points.length/3; i < list_data_points.length/3+100; i++){
            Log.e("Point Time ", String.valueOf(list_data_points[i].time));
        }
    }

}


public class DrawTrackActivity extends Activity{

    // creating a variable for our relative layout
    //private RelativeLayout relativeLayout;
    private LinearLayout track_linear;
    //private ConstraintLayout draw_track_layout;
    //private View track_view;
    String fileName;
    TextView textFileName;
    TextView textDate;


    DataPointList datapointlist;
    int curr_point = 0, resume_point = 0; //The last points of the list of data points we display
    int pt_per_update = 5; //How many more points we display every second
    boolean was_running;

    public String intToTime(int input){
        /*
        Convert the given int into a string representing the time
        The int is in the form: HHMMSSCC (Hour, Minute, Second, Centosecond)
         */
        int centoSeconds = input%100;
        int seconds = (input/100)%100;
        int minutes = (input/10000)%100;
        int hours = (input/1000000)%100;
        return hours + ":" + minutes + ":" + seconds + ":" + centoSeconds;
    }

    public String intToDate(int input){
        /*
        Convert the given int into a string representing the time
        The int is in the form: YYMMDD (Yeah, Month, Day)
         */
        int year = input%100;
        int month = (input/100)%100;
        int day = (input/10000)%100;
        return year + ":" + month + ":" + day;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_draw_track);

        //Get the parameters from the calling activity
        Intent intent=getIntent();
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
        final ToggleButton buttonOnOff = (ToggleButton) findViewById(R.id.buttonOnOff);
        final Button buttonShowAll = (Button) findViewById(R.id.buttonShowFull);
        final Button buttonBackLot = (Button) findViewById(R.id.buttonBackLot);
        final Button buttonBackLittle = (Button) findViewById(R.id.buttonBaclLittle);
        final Button buttonForwardLittle = (Button) findViewById(R.id.buttonForwardLittle);
        final Button buttonForwardLot = (Button) findViewById(R.id.buttonForwardLot);
        final Button buttonReplayType = (Button) findViewById(R.id.buttonReplayType);

        final SeekBar seekReplaySpeed = (SeekBar) findViewById(R.id.seekReplaySpeed);
        final SeekBar seekProgress = (SeekBar) findViewById(R.id.seekProgress);

        final TextView textViewLapTime = (TextView) findViewById(R.id.textViewLapTime);
        final TextView textViewSpeed = (TextView) findViewById(R.id.textViewSpeed);
        final TextView textViewLean = (TextView) findViewById(R.id.textViewLean);

        Timer timerMovePosition = new Timer();

        //Set the progression bar limit
        seekProgress.setMax(datapointlist.list_data_points.length+1);

        track_linear = findViewById(R.id.track_linear);
        //draw_track_layout = findViewById(R.id.draw_track_layout);
        textFileName = findViewById(R.id.textFileName);
        textDate = findViewById(R.id.textDate);

        // calling our  paint view class and adding
        // its view to our relative layout.
        DrawTrack drawTrack = new DrawTrack(this);
        drawTrack.set_data_points(datapointlist.list_data_points);

        track_linear.addView(drawTrack);
        textFileName.setText(fileName);
        if (datapointlist.list_data_points.length > 0) {
            String d = intToDate(datapointlist.list_data_points[0].date);
            String t = intToTime(datapointlist.list_data_points[0].time);
            String display = "The " + d + " at " + t;
            textDate.setText(display);
        }
        else{
            textDate.setText("Error with the file");
        }

        //This is used to set the speed at witch we display the points
        timerMovePosition.scheduleAtFixedRate(new TimerTask() {
            @SuppressLint({"DefaultLocale", "SetTextI18n"})
            @Override
            public void run() {
            if (buttonOnOff.isChecked()) {
                curr_point = min(curr_point + pt_per_update, datapointlist.list_data_points.length-1);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
                seekProgress.setProgress(curr_point);
                //Display the lap time, speed and lean in text
                SingleLap SL = new SingleLap();
                SL.compute_lap_time(datapointlist.list_data_points, curr_point, 30, 100);

                textViewSpeed.setText(String.format("%3.1f", datapointlist.list_data_points[curr_point].speed) + "Km/h");
                textViewLean.setText(String.format("%2.1f", datapointlist.list_data_points[curr_point].roll_abs) + "°");

                if (SL.found_lap) textViewLapTime.setText(SL.lap_time_string);
                else textViewLapTime.setText("No lap time");
            }
            }
        }, 0, 100);//put here time 100 milliseconds = 0.1 second

        buttonShowAll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (curr_point >= 0){
                    resume_point = curr_point;
                    curr_point = -1;
                    was_running = buttonOnOff.isChecked();
                    buttonOnOff.setChecked(false);
                }
                else{
                    curr_point = resume_point;
                    if (was_running) buttonOnOff.setChecked(true);
                }
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        //Set the button to navigate in the replay
        buttonBackLot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                curr_point = max(0, curr_point-50);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });
        buttonBackLittle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                curr_point = max(0, curr_point-5);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        buttonForwardLittle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                curr_point = min(datapointlist.list_data_points.length-1, curr_point+5);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        buttonForwardLot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                curr_point = min(datapointlist.list_data_points.length-1, curr_point+50);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        buttonReplayType.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String btnState = buttonReplayType.getText().toString().toLowerCase();
                switch (btnState){
                    case "default":
                        buttonReplayType.setText("Lean");
                        drawTrack.set_replay_type(1);
                        break;
                    case "lean":
                        buttonReplayType.setText("Speed");
                        drawTrack.set_replay_type(2);
                        break;
                    default:
                        buttonReplayType.setText("Default");
                        drawTrack.set_replay_type(0);
                        break;
                }
                drawTrack.invalidate();
            }
        });

        //Handle the two seek bar
        seekReplaySpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                //pt_per_update = progress;
                pt_per_update = (int)((progress*progress)/(150.0*150.0)*150);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        seekProgress.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                curr_point = progress;
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
                //Stop the auto progress while we look what we want.
                if (buttonOnOff.isChecked()) was_running = true;
                else was_running = false;
                buttonOnOff.setChecked(false);
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (was_running) buttonOnOff.setChecked(true);
            }
        });

    }

}

