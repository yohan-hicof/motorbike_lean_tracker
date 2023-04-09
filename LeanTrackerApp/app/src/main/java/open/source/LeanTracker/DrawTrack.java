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

        // on below line we are initializing our paint variable for our text
        textPaint = new Paint(Paint.LINEAR_TEXT_FLAG | Paint.ANTI_ALIAS_FLAG);
        // on below line we are setting color to it.
        textPaint.setColor(Color.WHITE);

        // on below line we are setting text size to it.
        // In Paint we have to add text size using px so
        // we have created a method where we are converting dp to pixels.
        textPaint.setTextSize(pxFromDp(context, 24));

        // on below line we are initializing our outer paint
        outerPaint = new Paint();

        // on below line we are setting style to our paint.
        outerPaint.setStyle(Paint.Style.FILL);

        // on below line we are setting color to it.
        outerPaint.setColor(getResources().getColor(R.color.colorOn));

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
        list_data_points = new DataPoint[list_dp.length];
        System.arraycopy(list_dp, 0, list_data_points, 0, list_dp.length);
        if (list_data_points.length == 0) return;
        //Scale all the lat/lng between 0 and 1
        double min_lat = list_data_points[0].lat, max_lat = list_data_points[0].lat;
        double min_lng = list_data_points[0].lng, max_lng = list_data_points[0].lng;
        double d_lat, d_lng;
        for (int i = 1; i < list_data_points.length; i++){
            min_lat = min(min_lat, list_data_points[i].lat);
            max_lat = max(max_lat, list_data_points[i].lat);
            min_lng = min(min_lng, list_data_points[i].lng);
            max_lng = max(max_lng, list_data_points[i].lng);
        }
        d_lat = max_lat-min_lat;
        d_lng = max_lng-min_lng;

        for (int i = 0; i < list_data_points.length; i++){
            list_data_points[i].lat = (list_data_points[i].lat-min_lat)/d_lat;
            list_data_points[i].lng = (list_data_points[i].lng-min_lng)/d_lng;
        }
    }

    public void set_nb_points(int nb){
        nb_points = nb;
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
                x = (float) list_data_points[i].lat * getWidth();
                y = (float) list_data_points[i].lng * getWidth();
                canvas.drawCircle(x, y, 2, trackPaint);
            }
            canvas.drawCircle(x, y, 6, positionPaint);
        }
    }
}