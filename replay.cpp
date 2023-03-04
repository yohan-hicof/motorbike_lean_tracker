#include "main.hpp"

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

  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setCursor(0, 10);  
  //Read the selected file.
  double_chain* head = read_data_to_file(replay_file_name);  
  double_chain* tail = find_last_link(head);
  //extract_abstract_data(head);
  //replay_standard_GUI(tail);
  replay_track_GUI(tail);
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
  uint32_t start_date, end_date;
  uint32_t start_time, end_time;
  int32_t nb_links, start_link = 0, end_link = 0, curr_link;

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

  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setCursor(0, 10); 
  M5.lcd.printf("Number of links: %d, start/end %d/%d\n", nb_links, start_link, end_link);
  M5.lcd.printf("Start time: %d:%d\n", start_date, start_time);
  M5.lcd.printf("End time: %d:%d\n", end_date, end_time);
  M5.lcd.printf("Max speed: %3.2f\n", max_speed);
  M5.lcd.printf("Mean speed: %3.2f\n", average_speed);
  M5.lcd.printf("Max pitch: %3.2f\n", max_pitch);
  M5.lcd.printf("Max roll: %3.2f\n", max_roll);
  M5.lcd.printf("Min lat: %3.6f\n", min_lat);
  M5.lcd.printf("Max lat: %3.6f\n", max_lat);
  M5.lcd.printf("Min lng: %3.6f\n", min_lng);
  M5.lcd.printf("Max lng: %3.6f\n", max_lng);

  M5.Lcd.printf("Press any button to continue\n");
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
    delay(20);
  }
  
  M5.Lcd.setCursor(0,110);
  M5.Lcd.printf("End of replay.\n Press any button to continue.");
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
  uint32_t start_time, end_time;
  int shift_x = 5, shift_y = 20;
  
  char starting[20], ending[20];

  if(current == NULL){
    M5.Lcd.printf("Cannot extract data, null pointer given\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }
  //Set the starting values
  end_date = current->data.date;
  end_time = current->data.time;
  min_lat = current->data.lat;
  max_lat = current->data.lat;
  min_lng = current->data.lng;
  max_lng = current->data.lng;
  
  while(current != NULL){//Get the min/max lat and the end time    
    min_lat = min(min_lat, current->data.lat);
    max_lat = max(max_lat, current->data.lat);
    min_lng = min(min_lng, current->data.lng);
    max_lng = max(max_lng, current->data.lng);    
    start_date = current->data.date;
    start_time = current->data.time;
    current = current->previous;  
  }
  //Now we get the min max for latitude and longitude, we can start displaying
  current = tail;

  format_date_time(start_date, start_time, starting);
  format_date_time(end_date, end_time, ending);

  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setCursor(0, 0); 
  M5.lcd.printf("Sarting time: %s\n", starting);
  M5.lcd.printf("Ending time: %s\n", ending);

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

    M5.Lcd.setCursor(0, 20); 
    M5.lcd.printf("Display @ %d, %d", x, y);

    M5.Lcd.drawPixel(x, y, color);
    delay(10);
    current = current->previous;
  }

  if(current == NULL){
    M5.Lcd.printf("End of replay\n Press any button to end.");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) return;
      delay(20);
     }    
  }

}

