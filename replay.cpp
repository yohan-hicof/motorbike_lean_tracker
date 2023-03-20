#include "main.hpp"

extern const unsigned char replay_menu[17530];
extern Preferences preferences;

void main_replay(){
  //Main function handling the replay.
  //First, we select the file we should replay.
  //Then we load it in memory.
  //Then we select the type of replay
  //Then we call the function doing the replay.

  char replay_file_name[64];

  bool is_valid = select_file(SD, replay_file_name);
  if (!is_valid){
     M5.Lcd.printf("Press any button to return to the menu\n");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) 
        return;
      delay(20);
     }
    return;
  }

  //Read the selected file.
  double_chain* head = read_data_to_file(replay_file_name);  
  double_chain* tail = find_last_link(head);
  
  while (true){
    M5.Lcd.drawJpg(replay_menu, 17530, 0,0,320,240);        
    delay(20);
    M5.update();
    Event& e = M5.Buttons.event;    
    if (e & E_TOUCH) {
      if (e.to.x < 155 && e.to.y < 115){
        //Replay with track
        replay_track_GUI(tail);
        continue;
      }
      if (e.to.x > 165 && e.to.y < 115){
        //Replay with the standard display
        replay_standard_GUI(tail);
        continue;
      }
      if (e.to.x < 155 && e.to.y > 125 && e.to.y < 210){
        //Select a file
        is_valid = select_file(SD, replay_file_name);
        if (!is_valid) return;
        double_chain* head = read_data_to_file(replay_file_name);  
        double_chain* tail = find_last_link(head);
        continue;
      }
      if (e.to.x > 160 && e.to.x < 240 && e.to.y > 125 && e.to.y < 210){
        //summary of the file.
        extract_abstract_data(head);
        continue;
      }
      if (e.to.x > 240 && e.to.y > 125 && e.to.y < 210){
        //Quit the replay menu        
        return;
      }
    }
  }
}

void extract_abstract_data(double_chain* head){

  /*
  This function will display a simple abstract of the data contained in the file.
  It counts the number of link and ignore the few first and last (depending on the number of links)
  Then it compute the max speed, mean speed, max pitch and roll ...
  Finally it displays all these values.
  */

  double_chain* current = head;
  float max_pitch = 0;
  float max_roll = 0;
  double max_speed = 0, average_speed = 0;  
  double min_lat, max_lat; 
  double min_lng, max_lng; 
  uint32_t start_date, end_date, start_time, end_time, diff_time;
  int32_t nb_links, start_link = 0, end_link = 0, curr_link;

  char starting[25], ending[25], duration[25];

  if(current == NULL){
    M5.Lcd.printf("Cannot extract data, null pointer given\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }

  nb_links = count_nb_links(head);
  //This is a way to avoid the first and last links that do not give much data since
  //This is the moment we start/stop the device. Maybe cut 30 seconds instead of this stuff
  if (nb_links > 500){
    start_link = max(20, min(100, nb_links/20));
    end_link = nb_links - start_link;
  }
  else{
    start_link = 0;
    end_link = nb_links;
  }

  curr_link = start_link;
  //Set the default values
  end_date = current->data.date;
  end_time = current->data.time;
  min_lat = current->data.lat;
  max_lat = current->data.lat;
  min_lng = current->data.lng;
  max_lng = current->data.lng;
  

  while(current != NULL && curr_link < end_link){//Get the data to display    
    max_pitch = max(max_pitch, current->data.pitch);
    max_roll = max(max_roll, current->data.roll);
    max_speed = max(max_speed, current->data.speed);
    average_speed += current->data.speed;
    min_lat = min(min_lat, current->data.lat);
    max_lat = max(max_lat, current->data.lat);
    min_lng = min(min_lng, current->data.lng);
    max_lng = max(max_lng, current->data.lng);
    
    start_date = current->data.date;
    start_time = current->data.time;

    current = current->next;
    curr_link++;    
  }

  average_speed /= end_link-start_link;

  format_date_time(start_date, start_time, starting);
  format_date_time(end_date, end_time, ending);
  time_difference(start_time, end_time, &diff_time);
  format_time(diff_time, duration);

  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setCursor(0, 10); 
  //M5.lcd.printf("Number of links: %d, start/end %d/%d\n", nb_links, start_link, end_link);
  M5.lcd.printf("Start time: \n  %s\n", starting);
  M5.lcd.printf("End time: \n  %s\n", ending);
  M5.lcd.printf("Duration: %s\n", duration);
  M5.lcd.printf("Max speed: %3.2f\n", max_speed);
  M5.lcd.printf("Mean speed: %3.2f\n", average_speed);
  M5.lcd.printf("Max pitch: %3.2f\n", max_pitch);
  M5.lcd.printf("Max roll: %3.2f\n", max_roll);
  M5.lcd.printf("Min lat: %3.6f\n", min_lat);
  M5.lcd.printf("Max lat: %3.6f\n", max_lat);
  M5.lcd.printf("Min lng: %3.6f\n", min_lng);
  M5.lcd.printf("Max lng: %3.6f\n", max_lng);

  M5.Lcd.printf("Press any button to quit\n");
  while(1){
    M5.update();
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) 
      return;
    delay(20);
  }  
  
}

void replay_standard_GUI(double_chain* tail){
  // This one start from the tail since we want to replay from the beginning.
  // We first move about 50 nodes, then every nodes, we reload the gui and wait for a given time.

  double_chain* current = tail;
  int curr_link = 0;
  uint32_t wait_time = 20;

  if(current == NULL){
    M5.Lcd.printf("Cannot extract data, null pointer given\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }

  //First we move a bit to show a few data points already
  while (current->previous != NULL && curr_link < 25){ //We know current is not NULL
    current = current->previous;
    curr_link++;
  }

  if (current->previous == NULL && current->next == NULL) return; //Stupid case, should not happend

  while (current->previous != NULL){ //We know current is not NULL
    display_data_point_GUI(current);
    current = current->previous;
    delay(wait_time);
    M5.update();
    if (M5.BtnA.wasPressed()) wait_time = max(10.0, wait_time*0.75);
    if (M5.BtnB.wasPressed()) wait_time = min(100.0, wait_time*1.5);
    if (M5.BtnC.wasPressed()) break;
  }
  
  M5.Lcd.setCursor(0,110);
  M5.Lcd.printf("End of replay.\nPress any button to continue.");
  while(1){
    M5.update();
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
    delay(20);
  }
}

void replay_track_GUI(double_chain* tail){

  double_chain* current = tail;
  double min_lat, max_lat; 
  double min_lng, max_lng; 
  double diff_lat, diff_lng, ratio;
  uint32_t start_date, end_date;
  uint32_t start_time, end_time, remaining_time;
  int shift_x = 5, shift_y = 20, wait_time = 25;
  
  char remaining[25];

  if(current == NULL){
    M5.Lcd.printf("Cannot extract data, null pointer given\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }
  //Set the starting values
  start_date = current->data.date;
  start_time = current->data.time;  
  min_lat = current->data.lat;
  max_lat = current->data.lat;
  min_lng = current->data.lng;
  max_lng = current->data.lng;
  
  while(current != NULL){//Get the min/max lat and the end time    
    min_lat = min(min_lat, current->data.lat);
    max_lat = max(max_lat, current->data.lat);
    min_lng = min(min_lng, current->data.lng);
    max_lng = max(max_lng, current->data.lng);    
    end_date = current->data.date;
    end_time = current->data.time;
    current = current->previous;  
  }
  //Now we get the min max for latitude and longitude, we can start displaying
  current = tail;
 
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);    

  //Do the dot per dot display
  diff_lat = max_lat-min_lat; //y
  diff_lng = max_lng-min_lng; //x

  if (210.0/diff_lat < 310/diff_lng) {
    ratio = 210.0/diff_lat;
    shift_x = (290-diff_lng*ratio)/2;
  }
  else{    
    ratio = 290.0/diff_lng;
    shift_y = (210-diff_lat*ratio)/2;
  }

  while(current != NULL){    
    // Find the coordinate, then compute the color for this pixel
    int x = shift_x + ratio * (current->data.lng - min_lng);
    int y = shift_y + ratio * (current->data.lat - min_lat);
    int speed = speed = constrain(current->data.speed, 20 ,260);
    uint8_t green = map(speed, 20, 260, 63, 0);
    uint8_t red = map(speed, 20, 260, 0, 31);
    uint16_t color = green*32 + red*2048;

    //Get the remaining time.
    time_difference(current->data.time, end_time, &remaining_time);
    format_time(remaining_time, remaining);

    M5.Lcd.setCursor(5, 5); 
    M5.lcd.printf("Remaining: %s\n", remaining);
    M5.lcd.printf("Speed: %3.2f  Lean: %2.1f    \n", current->data.speed, current->data.roll);    

    M5.Lcd.drawPixel(x, y, color);

    if (preferences.getBool("show_led", false)) update_led((int)current->data.roll);

    //Allow to slow down, speed up of pause the current replay
    M5.update();
    if (M5.BtnA.wasPressed()){wait_time = max(10, wait_time-5);}//Slow down
    if (M5.BtnC.wasPressed()){wait_time = min(200, wait_time+5);}//Speed up
    if (M5.BtnB.wasPressed()){
      delay(100);      
      while(1){
        M5.update();
        if (M5.BtnB.wasPressed()) break;
      }
    }

    delay(wait_time);
    current = current->previous;

  }

  if(current == NULL){
    M5.Lcd.setCursor(0, 205); 
    M5.Lcd.printf("End of replay\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }

}

