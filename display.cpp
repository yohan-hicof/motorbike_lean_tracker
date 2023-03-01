#include "main.hpp"

TFT_eSprite tracker_sprite = TFT_eSprite(&M5.Lcd);
TFT_eSprite menu_sprite = TFT_eSprite(&M5.Lcd);
int course_x = 80, course_y = 65, course_r = 50;
int counter_x = 235, counter_y = 65, counter_r = 50;
int x_min = 60, x_max = 300, y_min = 150, y_max = 230;


void create_menu_sprite(){

  int x_min = 0, y_min = 0, x_mid = 160, y_mid = 120, x_max = 320, y_max = 240;
  menu_sprite.createSprite(x_max, y_max);
  //Divide in four
  menu_sprite.drawFastHLine(x_min, y_mid, x_max-x_min, TFT_DARKGREY);      
  menu_sprite.drawFastVLine(x_mid, y_min, y_max-y_min, TFT_DARKGREY);
  //Draw the main icon to start capture
  menu_sprite.drawRightString("Start new tracking", (x_min+x_mid)/2+45, (y_min+y_mid)/2-50, 2);
  menu_sprite.drawRect(x_min+20, y_min+25, x_mid-x_min-40, y_mid-y_min-40, TFT_WHITE);
  for (int x = x_min+20, i = 0; x <= x_mid-x_min-40; x += 20, i++){
    for (int y = y_min+25, j = 0; y <= y_mid-y_min-20; y += 20, j++){
      if ((i+j)%2 == 0) continue;
      menu_sprite.fillRect(x, y, 20, 20, TFT_WHITE);
    }
  }  

  //Draw the icon to check captured file
  menu_sprite.drawRightString("Show recording", (x_mid+x_max)/2+45, (y_min+y_mid)/2-50, 2);
  menu_sprite.drawRect(x_mid+40, y_min+40, 90, 60, TFT_WHITE);
  menu_sprite.drawRect(x_mid+100, y_min+30, 30, 10, TFT_WHITE);
  menu_sprite.drawFastHLine(x_mid+40, y_min+50, 90, TFT_WHITE);      

  //Draw the icon to check levels
  menu_sprite.drawRightString("Display angles", (x_min+x_mid)/2+45, (y_mid+y_max)/2-50, 2);
  menu_sprite.drawCircle((x_min+x_mid)/2, (y_mid+y_max)/2+15, 20, TFT_WHITE);
  menu_sprite.drawCircle((x_min+x_mid)/2, (y_mid+y_max)/2+15, 30, TFT_WHITE);
  menu_sprite.fillCircle((x_min+x_mid)/2+5, (y_mid+y_max)/2+22, 4, TFT_WHITE); 
  

  //Draw icon to calibrate IMU
  menu_sprite.drawRightString("Calibrate IMU", (x_mid+x_max)/2+40, (y_mid+y_max)/2-50, 2);
  menu_sprite.drawCircle((x_mid+x_max)/2, (y_mid+y_max)/2+15, 25, TFT_WHITE);
  menu_sprite.drawCircle((x_mid+x_max)/2, (y_mid+y_max)/2+15, 30, TFT_WHITE);
  menu_sprite.drawFastHLine(x_mid+30, (y_mid+y_max)/2+15, x_max-x_mid-60, TFT_WHITE);      
  menu_sprite.drawFastVLine((x_mid+x_max)/2, y_mid+30, y_max-y_mid-20, TFT_WHITE);



}

void create_tracker_sprite(){
  
  tracker_sprite.createSprite(320, 240);

  //The position of the sprite for the direction
  tracker_sprite.drawChar('N', course_x-3, course_y-course_r, 2);
  tracker_sprite.drawChar('S', course_x-3, course_y+course_r-13, 2);
  tracker_sprite.drawChar('W', course_x-course_r+2, course_y-5, 2);
  tracker_sprite.drawChar('E', course_x+course_r-10, course_y-5, 2);
  tracker_sprite.drawCircle(course_x, course_y, course_r, WHITE);
  tracker_sprite.drawCircle(course_x, course_y, course_r-13, WHITE);

  //The circle for the speed counter
  tracker_sprite.drawCircle(counter_x, counter_y, counter_r, WHITE);  
  tracker_sprite.drawCircle(counter_x, counter_y, counter_r-4, WHITE);
  tracker_sprite.drawLine(counter_x, counter_y-counter_r, counter_x, counter_y-counter_r+6, WHITE); // 140
  tracker_sprite.drawLine(counter_x-47, counter_y-13, counter_x-40, counter_y-11, WHITE); // 80
  tracker_sprite.drawLine(counter_x+40, counter_y-11, counter_x+47, counter_y-13, WHITE); // 200
  tracker_sprite.drawLine(counter_x-25, counter_y+34, counter_x-29, counter_y+40, WHITE); // 20
  tracker_sprite.drawLine(counter_x+25, counter_y+34, counter_x+29, counter_y+40, WHITE); // 260

  M5.Lcd.setTextSize(2);  
  tracker_sprite.drawRightString("140", counter_x+12, counter_y-counter_r-19, 2);
  tracker_sprite.drawRightString("20", counter_x-32, counter_y+43, 2);
  tracker_sprite.drawRightString("260", counter_x+50, counter_y+43, 2);
  tracker_sprite.drawRightString("80", counter_x-54, counter_y-20, 2);
  tracker_sprite.drawRightString("200", counter_x+75, counter_y-20, 2);  
  M5.Lcd.setTextSize(1);

  
  //The rectangle for historic data
  tracker_sprite.fillRect(x_min,y_min, x_max-x_min, y_max-y_min, BLACK);
  tracker_sprite.drawRect(x_min,y_min, x_max-x_min, y_max-y_min, TFT_DARKGREY);
  //Orange line @45 angle, and red @60 angle
  tracker_sprite.drawFastHLine(x_min, y_max-15, x_max-x_min, GREEN);
  tracker_sprite.drawFastHLine(x_min, y_max-30, x_max-x_min, YELLOW);
  tracker_sprite.drawFastHLine(x_min, y_max-45, x_max-x_min, ORANGE);
  tracker_sprite.drawFastHLine(x_min, y_max-60, x_max-x_min, RED);    
  //Draw line to show time a bit
  tracker_sprite.drawFastVLine(x_min+60, y_min, y_max-y_min, TFT_DARKGREY);
  tracker_sprite.drawFastVLine(x_min+120, y_min, y_max-y_min, TFT_DARKGREY);
  tracker_sprite.drawFastVLine(x_min+180, y_min, y_max-y_min, TFT_DARKGREY); 
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

void draw_speed_triangle(float speed, int cx, int cy, int r){
  //We want to display the speed on the circle.
  //The minimum is 20kmh with the angle 234 degree
  //The maximum is 260kmh with the angle 54
  //The color is dertemined by the distance
  
  //Limit the speed to 20-260
  speed = constrain(speed, 20 ,260);

  uint8_t green = map(speed, 20, 260, 63, 0);
  uint8_t red = map(speed, 20, 260, 0, 31);
  uint16_t color = green*32 + red*2048;
   
  float angle = map(speed, 20, 260, 234, -54)*PI/180;
   
  //pt1: end of needle, pt2,pt3 base of needle
  int x1 = cx + cos(angle)*(r-1), y1 = cy - sin(angle)*(r-1);
  int x2 = cx + sin(angle)*5, y2 = cy + cos(angle)*5;
  int x3 = cx - sin(angle)*5, y3 = cy - cos(angle)*5;

  //M5.Lcd.drawLine(cx, cy, x, y, WHITE);
  M5.Lcd.fillCircle(cx, cy, r, BLACK);
  M5.Lcd.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void draw_direction_triangle(float course, int cx, int cy, int r){
  //We want to display the current direction.
  float angle = course*PI/180;
   
  //pt1: end of needle, pt2,pt3 base of needle
  int x1 = cx + cos(angle)*(r-1), y1 = cy - sin(angle)*(r-1);
  int x2 = cx + sin(angle)*5, y2 = cy + cos(angle)*5;
  int x3 = cx - sin(angle)*5, y3 = cy - cos(angle)*5;
  //M5.Lcd.drawLine(cx, cy, x, y, WHITE);
  M5.Lcd.fillCircle(cx, cy, r, BLACK);
  M5.Lcd.fillTriangle(x1, y1, x2, y2, x3, y3, LIGHTGREY);
}

void draw_lean_angle_bar(float lean, int cx, int cy, int w, int h){
  //Draw a rectangle on the bottom left of the screen to indicate the current lean angle.
  //The color is also proportional to the lean angle  

  //Limit the lean angle to 0-65, only positive   
  int max_lean = 65;
  lean = constrain(fabs(lean), 0, max_lean);  
  float height = map(lean, 0, max_lean, 0, h);

  uint8_t green = map(lean, 0, max_lean, 63, 0);
  uint8_t red = map(lean, 0, max_lean, 0, 31);
  uint16_t color = green*32 + red*2048;
  
  M5.lcd.fillRect(cx, cy+h-height, w, height, color);
  M5.lcd.fillRect(cx, cy, w, h-height, BLACK);
  M5.lcd.drawRect(cx, cy, w, h, TFT_LIGHTGREY);  
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

  if(head == NULL){ //Nothing to display
    return;
  }

  tracker_sprite.pushSprite(0, 0);
  draw_direction_triangle(head->data.direction, course_x, course_y, course_r-15);  
  draw_speed_triangle(head->data.speed, counter_x, counter_y, counter_r-10);  
  draw_lean_angle_bar(head->data.roll, 20, 130, 25, 100);  
  double_chain* current = head;  
  int index = x_max, prev_roll = current->data.roll, prev_speed = current->data.speed;
  prev_speed = map(prev_speed, 20, 260, y_max, y_min);
  prev_roll = map(fabs(prev_roll), 0, 65, y_max, y_max-65);
  
  while (current != NULL && index > x_min){
    int curr_roll = current->data.roll;
    int curr_speed = current->data.speed;
    curr_speed = map(curr_speed, 20, 260, y_max, y_min);
    curr_roll = map(fabs(curr_roll), 0, 65, y_max, y_max-65);
        
    M5.lcd.drawLine(index-2, curr_roll, index, prev_roll, GREEN);
    M5.lcd.drawLine(index-2, curr_speed, index, prev_speed, BLUE);
    
    prev_roll = curr_roll;
    prev_speed = curr_speed;
    index-=2;
    current = current->next;
  }

  
  delay(1200);
  /*
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
  */

}
