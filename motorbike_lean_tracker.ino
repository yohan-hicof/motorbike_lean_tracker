
#include "main.hpp"
#include "icons.c"

TinyGPSPlus gps; //Creat The TinyGPS++ object.
HardwareSerial ss(2); // The serial connection to the GPS device.
//static const uint32_t GPSBaud = 9600; //For the gps 
static const uint32_t GPSBaud = 115200; //For the gps 

char save_file_name[64]; //Timestamp name
File dataFile;
bool sd_card_found = false;

Preferences preferences;

extern TFT_eSprite battery_sprite;
TFT_eSprite main_menu_sprite = TFT_eSprite(&M5.Lcd);

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

void setup_gps(){
  /*
  This function should be called only once on a new gps unit to set it to 115200 Baud
  By default the gps unit is set at 9600Baud, but this is to slow to transfer the data @10Hz
  */
  /*ss.begin(GPSBaud, SERIAL_8N1, 13, 14);
  delay(100);
  ss.println("$PCAS10,3*1F");//Reboot in factory setting mode.
  delay(60000);*/

  ss.begin(9600, SERIAL_8N1, 13, 14);
  delay(20);    
  ss.println("$PCAS01,5*19");
  //delay(20);
  //ss.println("$PCAS10,0*1C");
  ss.flush();
  delay(20);
  ss.end();

  //ss.begin(GPSBaud, SERIAL_8N1, 13, 14);
  //delay(100);    
  //ss.println("$PCAS04,7*1E");//GPS, BDS, GLONASS
  //ss.println("$PCAS04,5*1C");//GPS, GLONASS
  //ss.println("$PCAS04,3*1A");//GPS, BDS
  //ss.println("$PCAS04,2*1B");//BDS
  //ss.println("$PCAS04,1*18");//GPS

  //ss.println("$PCAS10,3*1F");//Reboot in factory setting mode.

  delay(20);
}

void setup() {
  
  M5.begin();
  //Connect to the gps
  setup_gps();
  delay(100);

  ss.begin(GPSBaud, SERIAL_8N1, 13, 14);
  delay(100);  
  //$PCAS02,100*1E  10HZ
  //$PCAS02,200*1D   5HZ
  //$PCAS02,250*18   4Hz
  //$PCAS02,500*1A   2HZ
  ss.println("$PCAS02,100*1E");
  ss.println("$PCAS04,7*1E");
  ss.flush();  

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
  
  Serial.begin(115200);  
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

  float roll = 0.0, pitch = 0.0, acceleration = 0.0;
  float old_roll = 0.0, old_pitch = 0.0;
  M5.Lcd.fillScreen(BLACK);
  while (1){
    M5.update();
    if (M5.BtnB.wasPressed()) return;    
    return_pitch_roll(&pitch, &roll, &acceleration);
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
    M5.Lcd.drawJpg(icon_config, 43446, 0,0,320,240);    
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
      if (e.to.x > 160 && e.to.x < 235 && e.to.y > 130){
        //Fourth quadrant right
        delay(50);        
        setupBT();
        delay(200);
        receive_command_GUI();
        closeBT();        
      }      
      if (e.to.x > 245 && e.to.y > 130){
        //Fourth quadrant right
        delay(50);        
        return;      
      }
    }
  }
  
}

void tracker_menu(){

  while (1){
    M5.Lcd.drawJpg(tracker_menu_icon, 44204, 0,0,320,240);   
    delay(50);
    M5.update();  
    Event& e = M5.Buttons.event;    
    if (e & E_TOUCH) {
      if (e.to.x < 150 && e.to.y < 110){
        //First quadrant
        delay(50);
         main_tracker_loop(0);
      }
      if (e.to.x > 170 && e.to.y < 110){
        //Second quadrant
        delay(50);
         main_tracker_loop(2);
      }
      if (e.to.x < 150 && e.to.y > 130){
        //Third quadrant
        delay(50);
        return;
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

  create_battery_sprite(volt_to_percent(M5.Axp.GetBatVoltage()));  
  main_menu_sprite.createSprite(320,240);
  main_menu_sprite.drawJpg(main_menu, 26618, 0,0,320,240);  
  main_menu_sprite.setPivot(25, 12);
  battery_sprite.pushRotated(&main_menu_sprite, 0, TFT_TRANSPARENT);

  main_menu_sprite.pushSprite(0,0,TFT_TRANSPARENT);

  delay(50);
  M5.update();  
  Event& e = M5.Buttons.event;    
  if (e & E_TOUCH) {
    if (e.to.x < 150 && e.to.y < 110){
      //First quadrant      
      tracker_menu();
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
