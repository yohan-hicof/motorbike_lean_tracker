package open.source.LeanTracker;

import static java.lang.Double.max;
import static java.lang.Double.min;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.DisplayMetrics;
import android.view.View;

import com.droiduino.bluetoothconn.R;

public class DrawTrack extends View {

    // below we are creating variables for our paint
    Paint outerPaint, textPaint;
    Paint bgPaint, trackPaint, positionPaint;
    Paint greenPaint, yellowPaint, orangePaint, redPaint;

    //Below are the threshold for the coloring of the replay.
    //We also need to know if we display speed, angle or nothing
    int speed_slow = 100;
    int speed_normal = 150;
    int speed_high = 200;
    int angle_low = 15;
    int angle_mid = 30;
    int angle_high = 45;
    int replay_type = 0; //0: all white, 1 lean, 2 speed;

    DataPoint[] list_data_points;
    int nb_points = 0;

    @SuppressLint("ResourceAsColor")
    public DrawTrack(Context context) {
        super(context);

        bgPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        bgPaint.setColor(Color.BLACK);
        bgPaint.setStyle(Paint.Style.FILL);

        trackPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        trackPaint.setColor(Color.WHITE);
        trackPaint.setStyle(Paint.Style.FILL);

        positionPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        positionPaint.setColor(Color.GREEN);
        positionPaint.setStyle(Paint.Style.FILL);

        greenPaint = new Paint(); greenPaint.setColor(Color.GREEN);
        yellowPaint = new Paint(); yellowPaint.setColor(Color.YELLOW);
        orangePaint = new Paint(); orangePaint.setColor(R.color.Orange);
        redPaint = new Paint(); redPaint.setColor(Color.RED);

        // on below line we are initializing our paint variable for our text
        textPaint = new Paint(Paint.LINEAR_TEXT_FLAG | Paint.ANTI_ALIAS_FLAG);
        // on below line we are setting color to it.
        textPaint.setColor(Color.WHITE);

        // on below line we are setting text size to it.
        // In Paint we have to add text size using px so
        // we have created a method where we are converting dp to pixels.
        textPaint.setTextSize(pxFromDp(context, 24));

        // on below line we are creating a display metrics
        DisplayMetrics displayMetrics = new DisplayMetrics();

        // on below line we are getting display metrics.
        ((Activity) getContext()).getWindowManager()
                .getDefaultDisplay()
                .getMetrics(displayMetrics);

        }

    // below method is use to generate px from DP.
    public static float pxFromDp(final Context context, final float dp) {
        return dp * context.getResources().getDisplayMetrics().density;
    }

    public void set_data_points(DataPoint list_dp[]){
        //list_data_points = new DataPoint[list_dp.length];
        //System.arraycopy(list_dp, 0, list_data_points, 0, list_dp.length);
        list_data_points = list_dp;
    }

    public void set_nb_points(int nb){
        nb_points = nb;
        if (nb_points < -1 || nb_points > list_data_points.length) nb_points = -1;
    }
    public void set_replay_type(int rt){
        replay_type = rt;
        if(replay_type < 0 || replay_type > 2) replay_type = 0;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        canvas.drawPaint(bgPaint);

        if (list_data_points.length != 0){
            float x = 0, y = 0;
            int end_pt = nb_points;
            if (end_pt == -1 || end_pt > list_data_points.length) end_pt = list_data_points.length;
            for (int i = 0; i < end_pt; i++) {
                x = (float) list_data_points[i].lat_scaled * getWidth();
                y = (float) list_data_points[i].lng_scaled * getWidth();
                if (replay_type == 1){
                    if (list_data_points[i].roll < angle_low)
                        canvas.drawCircle(x, y, 2, greenPaint);
                    else if (list_data_points[i].roll < angle_mid)
                        canvas.drawCircle(x, y, 2, yellowPaint);
                    else if (list_data_points[i].roll < angle_high)
                        canvas.drawCircle(x, y, 2, orangePaint);
                    else canvas.drawCircle(x, y, 2, redPaint);
                }
                else if (replay_type == 2){
                    if (list_data_points[i].speed < speed_slow)
                        canvas.drawCircle(x, y, 2, greenPaint);
                    else if (list_data_points[i].speed < speed_normal)
                        canvas.drawCircle(x, y, 2, yellowPaint);
                    else if (list_data_points[i].speed < speed_high)
                        canvas.drawCircle(x, y, 2, orangePaint);
                    else canvas.drawCircle(x, y, 2, redPaint);
                }
                else{//Default
                    canvas.drawCircle(x, y, 2, trackPaint);
                }
            }
            canvas.drawCircle(x, y, 6, positionPaint);
        }
    }
}