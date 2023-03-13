#include "main.hpp"

TFT_eSprite tracker_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite menu_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite needle_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite direction_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite lean_bar_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite bike_sprite = TFT_eSprite(&M5.Lcd);

int course_x = 80, course_y = 65, course_r = 50;
int counter_x = 235, counter_y = 65, counter_r = 50;
int x_min = 60, x_max = 300, y_min = 150, y_max = 230;

extern CRGB leds[10];
extern Preferences preferences;
extern const unsigned char tracker_bg[14442];
extern const unsigned char CBR600[4188];

void format_date_time(uint32_t date, uint32_t time, char* string_time){
  //Convert two int containing date and time into a single char*
  //We assume the char* has at least 25 char allocated.
  //The date as the following format: YYMMDD, the time: HHMMSSCC
  int year, month, day, hour, minute, second, centi;
  year = date/10000;
  month = (date/100)%100;
  day = date%100;
  hour = time/1000000;
  minute = (time/10000)%100;
  second = (time/100)%100;
  centi = time%100;

  sprintf(string_time, "%02d-%02d-%02d %02d-%02d-%02d-%02d\0", year, month, day, hour, minute, second, centi);
}

void format_time(uint32_t time, char* string_time){
  //Convert two int containing date and time into a single char*
  //We assume the char* has at least 25 char allocated.
  //The date as the following format: YYMMDD, the time: HHMMSSCC
  int hour, minute, second, centi;  
  hour = time/1000000;
  minute = (time/10000)%100;
  second = (time/100)%100;
  centi = time%100;

  sprintf(string_time, "%02d-%02d-%02d-%02d\0", hour, minute, second, centi);
}

void time_difference(uint32_t time_beg, uint32_t time_end, uint32_t* time_diff){
  //We assume that this is the same day.
  //The time is as follow: HHMMSSCC
  uint32_t bhour, bminute, bsecond, bcenti;
  uint32_t ehour, eminute, esecond, ecenti; 
  
  bhour = time_beg/1000000;
  bminute = (time_beg/10000)%100;
  bsecond = (time_beg/100)%100;
  bcenti = time_beg%100;

  ehour = time_end/1000000;
  eminute = (time_end/10000)%100;
  esecond = (time_end/100)%100;
  ecenti = time_end%100;

  //Convert into centi second, take the diff
  time_beg = bcenti + 100*bsecond + 60*100*bminute + 60*60*100*bhour;
  time_end = ecenti + 100*esecond + 60*100*eminute + 60*60*100*ehour;
  time_end -= time_beg;
  //Convert back to hours, minutes, second, centisecond
  bhour = time_end/(60*60*100);
  time_end -= bhour*60*60*100;
  bminute = time_end/(60*100);
  time_end -= bminute*60*100;
  bsecond = time_end/100;
  time_end -= bsecond*100;
  bcenti = time_end;
  //Store the data
  *time_diff = 1000000*bhour + 10000*bminute + 100*bsecond + bcenti;
}

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour) {
    byte seg = 6;  // Segments are 3 degrees wide = 120 segments for 360 degrees
    byte inc = 6;  // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Calculate first pair of coordinates for segment start
    float deg2grad = 0.0174532925;
    float sx    = cos((start_angle - 90) * deg2grad);
    float sy    = sin((start_angle - 90) * deg2grad);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
        // Calculate pair of coordinates for segment end
        float sx2 = cos((i + seg - 90) * deg2grad);
        float sy2 = sin((i + seg - 90) * deg2grad);
        int x2    = sx2 * (rx - w) + x;
        int y2    = sy2 * (ry - w) + y;
        int x3    = sx2 * rx + x;
        int y3    = sy2 * ry + y;

        lean_bar_sprite.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
        lean_bar_sprite.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

        // Copy segment end to sgement start for next segment
        x0 = x2;
        y0 = y2;
        x1 = x3;
        y1 = y3;
    }
}

void create_needle_sprite(){
    uint16_t couleur = 0x9185;
    needle_sprite.createSprite(20,80);
    needle_sprite.fillSprite(TFT_TRANSPARENT);
    needle_sprite.fillCircle(10,10,10,couleur);
    needle_sprite.fillRect(8,2,4,75,couleur);
    needle_sprite.fillTriangle(6,4,14,4,10,72,couleur);
    needle_sprite.fillCircle(10,10,6,TFT_BLACK);
    needle_sprite.setPivot(10,10);
}

void create_direction_sprite(){    
    direction_sprite.createSprite(8,65);
    direction_sprite.fillSprite(TFT_TRANSPARENT);    
    //direction_sprite.fillTriangle(0,60,8,60,4,65,TFT_BLUE);
    direction_sprite.fillCircle(4,60,4,TFT_BLUE);
    direction_sprite.setPivot(4,0);
}

void display_data_point_CLI(double_chain* head){
  /*
  A simple interface to just show the main data of the last points, maybe 3~5 last points.
  */

  uint32_t nb_links = 0;
  double_chain* current = head;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20); 
  M5.Lcd.print("Pitch  Roll   Speed   Lat   Lon\n");
  while(current != NULL && nb_links < 10){
    M5.Lcd.printf("%2.1f  %2.1f  %3.1f  %3.6f %3.6f\n", current->data.pitch, current->data.roll, current->data.speed, current->data.lat, current->data.lng);    
    nb_links++;
    current = current->next;
  }

}

void draw_lean_angle_bar(float lean, int w, int h){
  //Draw a rectangle on the bottom left of the screen to indicate the current lean angle.
  //The color is also proportional to the lean angle  

  //Limit the lean angle to 0-65, only positive   
  int max_lean = 65, centre_x = w/2;
  lean = constrain(lean, -max_lean, max_lean);

  uint8_t green = map(fabs(lean), 0, max_lean, 63, 0);
  uint8_t red = map(fabs(lean), 0, max_lean, 0, 31);
  uint16_t color = green*32 + red*2048;
  
  lean_bar_sprite.createSprite(w, h);
  lean_bar_sprite.setPivot(w/2,h/2);

  if (lean < 0){
    lean_bar_sprite.fillRect(0, 0, max_lean+lean, h, BLACK);
    lean_bar_sprite.fillRect(centre_x, 0, max_lean, h, BLACK);
    lean_bar_sprite.fillRect(centre_x+lean, 0, -lean, h, color);
  }
  else{
    lean_bar_sprite.fillRect(0, 0, max_lean, h, BLACK);    
    lean_bar_sprite.fillRect(centre_x, 0, lean, h, color);
    lean_bar_sprite.fillRect(centre_x+lean, 0, max_lean-lean, h, BLACK);
  }
  lean_bar_sprite.drawFastVLine(centre_x, 0, h, TFT_NAVY);
  lean_bar_sprite.drawRect(0, 0, w, h, TFT_LIGHTGREY);

  /*
  lean_bar_sprite.createSprite(100, 100);
  lean_bar_sprite.setPivot(50,50);
  fillArc(50, 50, 0, lean, 40, 40, 4, TFT_WHITE);*/

}

void update_led(int angle){

  //Display the LED if need depending on the angle.

  uint8_t green, red;
  angle = constrain(angle, -65, 65);
  if (angle < 10 && angle > -10){
    fill_solid(&leds[5], 5, CRGB(0, 255, 0));
  }
  else if (angle < 0){
    green = map(angle, 0, -65, 255, 0) ;
    red = map(angle, 0, -65, 0, 255) ;  
    fill_solid(&leds[0], 5, CRGB(0, 0, 0));
    fill_solid(&leds[5], 5, CRGB(red, green, 0));
  }
  else{
    green = map(angle, 0, 65, 255, 0) ;
    red = map(angle, 0, 65, 0, 255) ;
    fill_solid(&leds[0], 5, CRGB(red, green, 0));
    fill_solid(&leds[5], 5, CRGB(0, 0, 0));
  }  
  FastLED.show();
}


void display_data_point_GUI(double_chain* head){
  /*
  A gui to show the current results
  */
  //Display the background:
  //Top left: direction
  //Top right: Speed counter
  //Bottom left current lean angle.
  //Bottm right: lean angle historic (and speed ?)
  //We also display the LED if required

  if(head == NULL){ //Nothing to display
    return;
  }
  bike_sprite.createSprite(35,70);
  bike_sprite.drawJpg(CBR600, 4188, 0,0,35,70);
  bike_sprite.setPivot(17, 35);

  tracker_sprite.createSprite(320, 240);
  tracker_sprite.drawJpg(tracker_bg, 14442, 0,0,320,240);
  //Needle for the speed.
  tracker_sprite.setPivot(218, 101); //Pivot for the speed needle
  int angle = map(head->data.speed, 0, 280, 75, 285);
  needle_sprite.pushRotated(&tracker_sprite, angle, TFT_TRANSPARENT);
  //Needle for the direction.
  tracker_sprite.setPivot(66, 173);
  direction_sprite.pushRotated(&tracker_sprite, head->data.direction, TFT_TRANSPARENT);
  //Lean angle bar
  tracker_sprite.setPivot(75, 10);
  //tracker_sprite.setPivot(50, 50);
  draw_lean_angle_bar(head->data.roll, 131, 15);
  lean_bar_sprite.pushRotated(&tracker_sprite, 0, TFT_TRANSPARENT);
  
  //display the biker
  tracker_sprite.setPivot(65, 70);
  bike_sprite.pushRotated(&tracker_sprite, head->data.roll, TFT_TRANSPARENT);
  
  tracker_sprite.pushSprite(0,0,TFT_TRANSPARENT);
   
  if (preferences.getBool("show_led", false)) update_led((int)head->data.roll);

  double_chain* current = head;  
  int index = 310, prev_roll = current->data.roll, prev_speed = current->data.speed;
  prev_speed = map(prev_speed, 20, 260, y_max, y_min);
  prev_roll = map(fabs(prev_roll), 0, 65, y_max, y_max-65);
  
  while (current != NULL && index > 140){
    int curr_roll = constrain(current->data.roll, -65, 65);
    int curr_speed = current->data.speed;
    curr_speed = map(curr_speed, 0, 260, 232, 142);
    curr_roll = map(fabs(curr_roll), 0, 65, 232, 167);
        
    //M5.lcd.drawLine(index-2, curr_roll, index, prev_roll, GREEN);
    //M5.lcd.drawLine(index-2, curr_speed, index, prev_speed, BLUE);
    M5.lcd.drawLine(index-1, curr_roll, index, prev_roll, GREEN);
    M5.lcd.drawLine(index-1, curr_speed, index, prev_speed, BLUE);
    
    prev_roll = curr_roll;
    prev_speed = curr_speed;
    //index-=2;
    index--;
    current = current->next;    
  }
}

void display_real_time_GUI(double_chain* head, uint32_t start_time){
  /*
  A simple GUI to show very little information:
     Speed.
     Lean angle.
     Elapsed time since start.
     Last lap (when implemented), not sure if it will be possible in real time though.
  */
  
  if(head == NULL){ //Nothing to display
    return;
  }
  uint32_t elapsed_time;
  char elapsed[25];
  time_difference(start_time, head->data.time, &elapsed_time);
  format_time(elapsed_time, elapsed);

  M5.Lcd.setTextSize(3);
  //Display the speed
  M5.Lcd.setCursor(15, 60);   
  M5.Lcd.setTextColor(BLUE, BLACK);
  M5.lcd.printf("%3.1fKm/h", head->data.speed);

  //Display the lean angle
  M5.Lcd.setCursor(180, 60);
  if (head->data.roll <= -45 || head->data.roll >= 45)
    M5.Lcd.setTextColor(RED, BLACK);
  else if (head->data.roll <= -30 || head->data.roll >= 30)
    M5.Lcd.setTextColor(ORANGE, BLACK);
  else if (head->data.roll <= -15 || head->data.roll >= 15)
    M5.Lcd.setTextColor(YELLOW, BLACK);
  else    
    M5.Lcd.setTextColor(GREEN, BLACK);
  M5.lcd.printf("\t%2.1f ", head->data.roll);

  //Display the elapsed time.
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setCursor(25, 160);
  M5.lcd.printf("%s", elapsed);
  
}
