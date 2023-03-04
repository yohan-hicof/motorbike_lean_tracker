#include "main.hpp"

extern TinyGPSPlus gps;
extern bool sd_card_found;

void wait_for_gps(){
  uint32_t start_wait = millis();
  uint32_t index = 0;

  while(!gps.location.isValid()){    
    if (millis()-start_wait > 5000 && gps.charsProcessed() < 10){
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 20); 
      M5.Lcd.println("No GPS data received:\r\n\t check wiring.\r\nThen restart the device.");      
    }
    else{
      if (index%10 == 0){
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 20); 
        M5.Lcd.println("Waiting for the GPS\r\nto acquire data.\r\nThe gps can take up\r\nto 30 second to get ready.");
        M5.Lcd.print(".");
      }
      else{
        M5.Lcd.print(".");
      }
    }
    index++;
    delay(500);
    //smartDelay(500);
  }
}

void main_tracker_loop(){
  //This is the main loop.
  //It init the variable, then regularly capture data point.
  //When necessary it will save to the sd card or delete old data point.
  //It calls the display method to show the current data

  double_chain* head = NULL;
  double_chain* tail = NULL;
  double_chain* current = NULL;
  create_save_file_name();
  create_tracker_sprite();
  uint32_t last_save; //When was the last time we save on the sd card.
  uint32_t diff_delay;//When we save of delete link, we have to decrease the delay to stay consistent
  uint32_t wait_time = 100;//the number of ms we wait between two points
  uint32_t number_of_links = 0;
  uint32_t last_GUI_update = millis();

  Serial.begin(9600);

  //Wait for the GPS to be running before starting.
  wait_for_gps();
  M5.Lcd.fillScreen(BLACK);
  //Get the first data point, then start the loop. This avoid a useless if all the time.  
  head = create_new_data_point();
  //head = create_dummy_data_point(number_of_links++);
  tail = head;    
  
  delay(100);
  last_save = millis();
  while (true){
    current = create_new_data_point();
    //current = create_dummy_data_point(number_of_links);
    if (current != NULL){      
      head = add_new_head(head, current);
      number_of_links++;      
    }
    else {
      M5.Lcd.print("Failed to create new data point\n"); 
      continue; //An issue arise, we try again.
    }

    if (millis()-last_GUI_update > 100){
      //display_data_point_CLI(head);
      display_data_point_GUI(head);
      last_GUI_update = millis();
    }

    if (millis()-last_save > 2000 && sd_card_found){//Save every 2 seconds      
      diff_delay = millis();
      write_data_to_file(tail, 50); //We save up to 50 links
      last_save = millis(); //Update the time since last save.      
      diff_delay = last_save-diff_delay;
      if (diff_delay < wait_time)
        delay(wait_time-diff_delay);//Shorter wait to compensate for the time to write      
    }
    else if (number_of_links > 1000){      
      diff_delay = millis();
      tail = delete_n_links_from_tails(tail, 600, !sd_card_found); //We keep at least 200 data points
      number_of_links = count_nb_links(head); //Check how many we really have left (may be not all were saved)      
      diff_delay = millis() - diff_delay;
      if (diff_delay < wait_time)
        delay(wait_time-diff_delay);//Shorter wait to compensate for the time to write
    }
    else{
      delay(100);
      //smartDelay(100);
    }    
  }
}