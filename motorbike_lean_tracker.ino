
#include "main.hpp"
#include "icons.c"

TinyGPSPlus gps; //Creat The TinyGPS++ object.
HardwareSerial ss(2); // The serial connection to the GPS device.
static const uint32_t GPSBaud = 9600; //For the gps 

char save_file_name[64]; //Timestamp name
File dataFile;
bool sd_card_found = false;

Preferences preferences;

CRGB leds[10];

void feed_gps_bg(void* pvParameters){
  //Process in the background.
  //It feed regularly the gps object with new data from the gps.
  //This is better than smart delay since it does not require a process to actually need a delay to get new data.
  while(1){//The delay between points is at least 100ms, so refreshing every 25ms is more than enough
    while (ss.available()) gps.encode(ss.read());
    delay(25);
  }
}

void test_write_read(){
  create_save_file_name();

  double_chain* head = NULL; //First data point.
  double_chain* tail = NULL; //Last data point.

  head = create_dummy_data_point(1);
  tail = head;
  double_chain* temp;
  for (int i = 2; i < 10; i++){
    temp = create_dummy_data_point(i);
    tail->next = temp;
    temp->previous = tail;
    tail = temp;
  }
  temp = head;
  while(temp != NULL){
    M5.Lcd.printf("Index: %d\n", temp->data.date);
    temp = temp->next;
  }
  M5.Lcd.printf("Writing to file ...");
  write_data_to_file(tail, 10);
  M5.Lcd.printf("Done\n Cleaning the chain ...");
  tail = delete_n_links_from_tails(tail, 10);
  M5.Lcd.printf("Done\n Reading the file ...");
  head = NULL;
  head = read_data_to_file(save_file_name);
  M5.Lcd.printf("Done\n Finding last link ...");
  tail = find_last_link(head);
  M5.Lcd.printf("Done\n");
  
  M5.Lcd.printf("Reread the file\n");
  temp = head;
  while(temp != NULL){
    M5.Lcd.printf("Index: %d\n", temp->data.date);
    temp = temp->next;
  }
  
}



void setup() {
  
  M5.begin();
  //Connect to the gps
  ss.begin(GPSBaud, SERIAL_8N1, 13, 14);
  M5.IMU.Init();  

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor( GREEN, BLACK);
  M5.Lcd.setTextSize(1);

  check_sd_card();

  preferences.begin("imu-calibration", false);
  get_imu_preferences(&preferences);

  //Set the brightness of the screen to a lower value by default to save battery  
  M5.Axp.SetLcdVoltage(preferences.getInt("brightness", 2900));

  //Create the sprite we might use
  //create_tracker_sprite();
  //create_menu_sprite();
  create_needle_sprite();
  create_direction_sprite();

  FastLED.addLeds<NEOPIXEL, 25>(leds, 10);
  fill_solid(leds, 10, CRGB(0, 0, 0));
  FastLED.show();

  //Run them on the second core.
  //Start the computation of pitch and roll in the bg.
  xTaskCreatePinnedToCore(compute_pitch_roll_bg, "pitch_roll_bg", 4096, NULL, 1, NULL, 1);
  //Start the background process of getting feed from the gps
  xTaskCreatePinnedToCore(feed_gps_bg, "feed_gps_bg", 4096, NULL, 2, NULL, 1);
  
  
}

void test_display(){

  //TODO do an infinite while in which we create more data points.
  //The next data point has the value of the previous +- a random value. To keep some consistency
  
  int cpt = 0;
  double_chain *head = create_dummy_data_point(cpt++), *next;

  head->data.speed = random(50, 200);
  head->data.direction = random(0, 360);
  head->data.roll = random(0, 60);

  while(1){
    display_data_point_GUI(head);
    next = create_dummy_data_point(cpt++);
    next->next = head;
    head->previous = next;
    head = next;

    head->data.speed = 0.8*head->next->data.speed + 0.2*random(50, 200);;
    head->data.direction = 0.8*head->next->data.direction + 0.2*random(0, 360);
    head->data.roll = 0.8*head->next->data.roll + 0.2*random(0, 60);
    
  }
}

void drawSpot(float ax, float ay, float* old_x, float* old_y){

  int x = map(constrain(ax, -90, 90), -90, 90, 40, 280);
  int y = map(constrain(ay, -90, 90), -90, 90, 240, 0);
  
  M5.Lcd.fillCircle(*old_x, *old_y, 7, BLACK);  
  M5.Lcd.drawLine(41, 120, 279, 120, CYAN);
  M5.Lcd.drawLine(160, 1, 160, 239, CYAN);
  M5.Lcd.drawCircle(160, 120, 119, CYAN);
  M5.Lcd.drawCircle(160, 120, 60, CYAN);  
  M5.Lcd.fillCircle(x, y, 7, WHITE);  
    
  *old_x = x;
  *old_y = y;  
}

void display_bubble(){

  float roll = 0.0, pitch = 0.0;
  float old_roll = 0.0, old_pitch = 0.0;
  M5.Lcd.fillScreen(BLACK);
  while (1){
    M5.update();
    if (M5.BtnB.wasPressed()) return;    
    return_pitch_roll(&pitch, &roll);
    drawSpot(roll, pitch, &old_roll, &old_pitch);  

    uint8_t green, red;
    int Croll;
    Croll = constrain(roll, -65, 65);
    if (Croll < 10 && Croll > -10){
      fill_solid(&leds[5], 5, CRGB(0, 255, 0));
    }
    else if (Croll < 0){
      green = map(Croll, 0, -65, 255, 0) ;
      red = map(Croll, 0, -65, 0, 255) ;  
      fill_solid(&leds[0], 5, CRGB(0, 0, 0));
      fill_solid(&leds[5], 5, CRGB(red, green, 0));
    }
    else{
      green = map(Croll, 0, 65, 255, 0) ;
      red = map(Croll, 0, 65, 0, 255) ;
      fill_solid(&leds[0], 5, CRGB(red, green, 0));
      fill_solid(&leds[5], 5, CRGB(0, 0, 0));
    }    
    //fill_solid(leds, 10, CRGB(red, green, 0));
    FastLED.show();
    delay(100);
  }
}

void set_time(){  
  
  uint16_t position = 1;
  uint16_t posx[] = {0, 27, 72, 110, 142, 173, 217};
  //Get the current time and data
  RTC_TimeTypeDef RTCtime; 
  RTC_DateTypeDef RTCDate; 
  M5.Rtc.GetTime(&RTCtime);                             
  M5.Rtc.GetDate(&RTCDate);  
  //Prepare the display
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawJpg(buttons_clock, 11764, 0,182,320,57);
  //Let the user configure.
  while (1){
    
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("%02d-%02d-%02d %02dh%02dm%02ds", 
                  RTCDate.Year, RTCDate.Month, RTCDate.Date, RTCtime.Hours, 
                  RTCtime.Minutes, RTCtime.Seconds);
    //Draw a small triangle under the current position
    M5.Lcd.fillTriangle(posx[position], 85, posx[position]+14, 85, posx[position]+7, 70, TFT_RED);
    //Get the area pressed by the user.    
    M5.update();  
    Event& e = M5.Buttons.event;    
    if (e & E_TOUCH) {
      if (e.to.x < 64 && e.to.y > 180){
        M5.Lcd.fillTriangle(posx[position], 85, posx[position]+14, 85, posx[position]+7, 70, TFT_BLACK);
        position = max(position-1, 1);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.printf("Curr cursor: %d\n", position);
        continue;
      }
      if (e.to.x > 64 && e.to.x < 128 && e.to.y > 180){
        M5.Lcd.fillTriangle(posx[position], 85, posx[position]+14, 85, posx[position]+7, 70, TFT_BLACK);
        position = min(position+1, 6);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.printf("Curr cursor: %d\n", position);
        continue;
      }
      if (e.to.x > 128 && e.to.x < 192 && e.to.y > 180){//We increase the value by one
        switch(position){
          case 1:
            RTCDate.Year = min(RTCDate.Year+1, 2050);
            break;
          case 2:
            RTCDate.Month = min(RTCDate.Month+1, 12);
            break;
          case 3:
            RTCDate.Date = min(RTCDate.Date+1, 31);
            break;
          case 4:
            RTCtime.Hours = min(RTCtime.Hours+1, 23);
            break;
          case 5:
            RTCtime.Minutes = min(RTCtime.Minutes+1, 59);
            break;
          case 6:
            RTCtime.Seconds = min(RTCtime.Seconds+1, 59);            
            break;
        }
        continue;
      }
      if (e.to.x > 192 && e.to.x < 256 && e.to.y > 180){//We decrease the value by one
        switch(position){
          case 1:
            RTCDate.Year = max(RTCDate.Year-1, 2020);
            break;
          case 2:
            RTCDate.Month = max(RTCDate.Month-1, 1);
            break;
          case 3:
            RTCDate.Date = max(RTCDate.Date-1, 1);
            break;
          case 4:
            if (RTCtime.Hours > 0) RTCtime.Hours--;            
            break;
          case 5:
            if (RTCtime.Minutes > 0) RTCtime.Minutes--;            
            break;
          case 6:
            if (RTCtime.Seconds > 0) RTCtime.Seconds--;            
            break;
        }
        continue;
      }
      if (e.to.x > 256 && e.to.y > 180){//We save the time and date and return
        M5.Rtc.SetDate(&RTCDate);
        M5.Rtc.SetTime(&RTCtime);
        return;
      }
      
      delay(50);
    }

  }
}

void set_brightness(){

  M5.Lcd.drawJpg(brightness, 20950, 0,0,320,240);
  int curr_brightness = preferences.getInt("brightness", 2900);  
  while (true){     
    M5.update();
    Event& e = M5.Buttons.event;    
    if (e & E_TOUCH) {
      if (e.to.x < 64 && e.to.y > 40 && e.to.y < 200){
        //Turn of the led, save the preference
        fill_solid(leds, 10, CRGB(0, 0, 0));
        FastLED.show();
        preferences.putBool("show_led", false);
        continue;
      }
      if (e.to.x > 256 && e.to.y > 40 && e.to.y < 200){
        //Turn on the led, save the preference
        fill_solid(leds, 10, CRGB(0, 255, 0));
        FastLED.show();
        preferences.putBool("show_led", true);
        continue;
      }
    }


    if (M5.BtnA.wasPressed()){
      curr_brightness = max(curr_brightness-100, 2500);
      M5.Axp.SetLcdVoltage(curr_brightness);
      preferences.putInt("brightness", curr_brightness);
    }
    if (M5.BtnB.wasPressed()){
      return;
    }
    if (M5.BtnC.wasPressed()){
      curr_brightness = min(curr_brightness+100, 3300);
      M5.Axp.SetLcdVoltage(curr_brightness);
      preferences.putInt("brightness", curr_brightness);
    }
    delay(20);        
  }
}

void config_menu(){    
  while (1){
    M5.Lcd.drawJpg(icon_config, 20861, 0,0,320,240);    
    delay(50);
    M5.update();  
    Event& e = M5.Buttons.event;    
    if (e & E_TOUCH) {
      if (e.to.x < 150 && e.to.y < 110){
        //First quadrant
        delay(50);
        set_brightness();
      }
      if (e.to.x > 170 && e.to.y < 110){
        //Second quadrant
        delay(50);
        calibrationGryo(&preferences);
        calibrationAccel(&preferences);
        get_imu_preferences(&preferences);        
      }
      if (e.to.x < 150 && e.to.y > 130){
        //Third quadrant
        delay(50);
        set_time();        
      }
      if (e.to.x > 170 && e.to.y > 130){
        //Fourth quadrant
        delay(50);        
        return;      
      }
    }
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //menu_sprite.pushSprite(0, 0);
  //main_menu 54750
  M5.Lcd.drawJpg(main_menu, 26618, 0, 0, 320, 240);  
  delay(50);
  M5.update();  
  Event& e = M5.Buttons.event;    
  if (e & E_TOUCH) {
    if (e.to.x < 150 && e.to.y < 110){
      //First quadrant
      main_tracker_loop();
    }
    if (e.to.x > 170 && e.to.y < 110){
      //Second quadrant
      delay(50);
      main_replay();   
    }
    if (e.to.x < 150 && e.to.y > 130){
      //Third quadrant
      delay(50);
      display_bubble();
    }
    if (e.to.x > 170 && e.to.y > 130){
      //Fourth quadrant
      delay(50);
      config_menu();
    }
      
  }
  delay(50);

}
