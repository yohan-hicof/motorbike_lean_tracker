package open.source.LeanTracker;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import com.droiduino.bluetoothconn.R;

import java.util.ArrayList;
import java.util.List;

public class LapTimeAdapter extends RecyclerView.Adapter<LapTimeAdapter.ViewHolder>{

    ArrayList<SingleLap> list_laps;
    private LayoutInflater mInflater;
    private ItemClickListener mClickListener;

    // data is passed into the constructor
    LapTimeAdapter(Context context, ArrayList<SingleLap> laps) {
        this.mInflater = LayoutInflater.from(context);
        this.list_laps = laps;
    }

    // inflates the row layout from xml when needed
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = mInflater.inflate(R.layout.lap_time_layout, parent, false);
        return new ViewHolder(view);
    }

    // binds the data to the TextView in each row
    @SuppressLint("DefaultLocale")
    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        SingleLap lap = list_laps.get(position);
        String lapID = "Lap number: " + position;
        String lapTime = lap.lap_time_string;
        String max_speed = "Max speed: " + String.format("%3.1f", lap.max_speed);
        String mean_speed = "Mean speed: " + String.format("%3.1f", lap.mean_speed);
        String max_lean = "Max lean: " + String.format("%2.1f", lap.max_lean);

        holder.TextViewLapID.setText(lapID);
        holder.TextViewLapTime.setText(lapTime);
        holder.TextViewMaxSpeed.setText(max_speed);
        holder.TextViewMeanSpeed.setText(mean_speed);
        holder.TextViewMaxLean.setText(max_lean);
    }

    // total number of rows
    @Override
    public int getItemCount() {
        return list_laps.size();
    }


    // stores and recycles views as they are scrolled off screen
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        TextView TextViewLapID;
        TextView TextViewLapTime;
        TextView TextViewMaxSpeed;
        TextView TextViewMeanSpeed;
        TextView TextViewMaxLean;


        ViewHolder(View itemView) {
            super(itemView);
            TextViewLapID = itemView.findViewById(R.id.lapID);
            TextViewLapTime = itemView.findViewById(R.id.lapTime);
            TextViewMaxSpeed = itemView.findViewById(R.id.maxSpeed);
            TextViewMeanSpeed = itemView.findViewById(R.id.meanSpeed);
            TextViewMaxLean = itemView.findViewById(R.id.maxLean);
            itemView.setOnClickListener(this);
        }

        @Override
        public void onClick(View view) {
            if (mClickListener != null) mClickListener.onItemClick(view, getAdapterPosition());
        }
    }

    // convenience method for getting data at click position
    SingleLap getItem(int id) {
        return list_laps.get(id);
    }

    // allows clicks events to be caught
    void setClickListener(ItemClickListener itemClickListener) {
        this.mClickListener = itemClickListener;
    }

    // parent activity will implement this method to respond to click events
    public interface ItemClickListener {
        void onItemClick(View view, int position);
    }
}