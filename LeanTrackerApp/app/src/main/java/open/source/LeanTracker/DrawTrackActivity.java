package open.source.LeanTracker;

import static java.lang.Integer.max;
import static java.lang.Integer.min;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.droiduino.bluetoothconn.R;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Timer;
import java.util.TimerTask;

class DataPoint implements Serializable {
    float pitch;
    float roll;
    float acceleration;
    float speed;
    float direction;
    double lat;
    double lng;
    int date;
    int time;
}

public class DrawTrackActivity extends Activity{

    // creating a variable for our relative layout
    private RelativeLayout relativeLayout;
    private LinearLayout track_linear;
    private ConstraintLayout draw_track_layout;
    //private View track_view;
    String fileName;
    TextView textFileName;
    TextView textDate;
    DataPoint[] list_data_points;
    int curr_point = 0, resume_point = 0; //The last points of the list of data points we display
    int pt_per_second = 20; //How many more points we display every second
    boolean was_running;

    //Timer running_timer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_draw_track);

        //Get the parameters from the calling activity
        Intent intent=getIntent();
        fileName = intent.getStringExtra("file_name");
        read_input_file();
        // initializing our view.
        final ToggleButton buttonOnOff = (ToggleButton) findViewById(R.id.buttonOnOff);
        final Button buttonShowAll = (Button) findViewById(R.id.buttonShowFull);
        final Button buttonBackLot = (Button) findViewById(R.id.buttonBackLot);
        final Button buttonBackLittle = (Button) findViewById(R.id.buttonBaclLittle);
        final Button buttonForwardLittle = (Button) findViewById(R.id.buttonForwardLittle);
        final Button buttonForwardLot = (Button) findViewById(R.id.buttonForwardLot);

        final SeekBar seekReplaySpeed = (SeekBar) findViewById(R.id.seekReplaySpeed);
        final SeekBar seekProgress = (SeekBar) findViewById(R.id.seekProgress);
        Timer timerMovePosition = new Timer();

        //Set the progression bar limit
        seekProgress.setMax(list_data_points.length+1);


        //test_view = findViewById(R.id.test_linear_id);
        track_linear = findViewById(R.id.track_linear);
        draw_track_layout = findViewById(R.id.draw_track_layout);
        //track_view = findViewById(R.id.viewTrack);
        textFileName = findViewById(R.id.textFileName);
        textDate = findViewById(R.id.textDate);
        // calling our  paint view class and adding
        // its view to our relative layout.
        DrawTrack drawTrack = new DrawTrack(this);
        drawTrack.set_data_points(list_data_points);

        track_linear.addView(drawTrack);
        //draw_track_layout.addView(drawTrack);
        textFileName.setText(fileName);

        //This is used to set the speed at witch we display the points
        //new Timer().scheduleAtFixedRate(new TimerTask() {
        timerMovePosition.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (buttonOnOff.isChecked()) {
                    curr_point = min(curr_point+pt_per_second, list_data_points.length-1);
                    drawTrack.set_nb_points(curr_point);
                    drawTrack.invalidate();
                    textDate.setText(String.valueOf(curr_point));
                    seekProgress.setProgress(curr_point);
                }
            }
        }, 0, 100);//put here time 1000 milliseconds=1 second

        buttonShowAll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (curr_point >= 0){
                    resume_point = curr_point;
                    curr_point = -1;
                    if (buttonOnOff.isChecked()) was_running = true;
                    else was_running = false;
                    buttonOnOff.setChecked(false);
                    drawTrack.set_nb_points(curr_point);
                    drawTrack.invalidate();
                }
                else{
                    curr_point = resume_point;
                    if (was_running) buttonOnOff.setChecked(true);
                    drawTrack.set_nb_points(curr_point);
                    drawTrack.invalidate();
                }
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
                curr_point = min(list_data_points.length-1, curr_point+5);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        buttonForwardLot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                curr_point = min(list_data_points.length-1, curr_point+50);
                drawTrack.set_nb_points(curr_point);
                drawTrack.invalidate();
            }
        });

        //Handle the two seek bar
        seekReplaySpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                pt_per_second = progress;
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

    public void read_input_file(){

        try {
            FileInputStream fileIn = openFileInput(fileName);
            long fileSize = fileIn.getChannel().size();
            int nb_data_points = (int)(fileSize/44);
            Log.e("Read file: ",fileName);
            Log.e("File size: ", String.valueOf(fileSize));
            Log.e("Number of points: ", String.valueOf(nb_data_points));

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
    }
}

