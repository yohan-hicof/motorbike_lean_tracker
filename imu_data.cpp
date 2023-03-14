#include "main.hpp"

float accel[3];
float gyro[3];
float offset_accel[3];
float offset_gyro[3];

float thetaM;
float phiM;
float theta = 0;
float phi = 0;
float ratio_gyro = 0.95F;
float ratio_accel = 1.0-ratio_gyro;

float dt;
unsigned long millisOld;

void GetGyroData(uint16_t times, float* gyro) {
  double x = 0, y = 0, z = 0;
  float gyro_x, gyro_y, gyro_z;
  for (size_t i = 0; i < times; i++) {
    //M5.IMU.getGyroAdc(&gyro_x, &gyro_y, &gyro_z);
    M5.IMU.getGyroData(&gyro_x, &gyro_y, &gyro_z);
    x += gyro_x;
    y += gyro_y;
    z += gyro_z;
  }
  gyro[0] = x / times;
  gyro[1] = y / times;
  gyro[2] = z / times;
}

void GetAccelData(uint16_t times, float* accel) {
  double x = 0, y = 0, z = 0;
  float gyro_x, gyro_y, gyro_z;
  for (size_t i = 0; i < times; i++) {
    //M5.IMU.getGyroAdc(&gyro_x, &gyro_y, &gyro_z);
    M5.IMU.getAccelData(&gyro_x, &gyro_y, &gyro_z);    
    x += gyro_x;
    y += gyro_y;
    z += gyro_z;
  }

  accel[0] = x / times;
  accel[1] = y / times;
  accel[2] = z / times;
}

void GetAhrsData(uint16_t times, float* ahrs) {
  double x = 0, y = 0, z = 0;
  float pi, ro, ya;
  for (size_t i = 0; i < times; i++) {
    //M5.IMU.getGyroAdc(&gyro_x, &gyro_y, &gyro_z);
    M5.IMU.getAhrsData(&pi, &ro, &ya);
    x += pi;
    y += ro;
    z += ya;
  }

  ahrs[0] = x / times;
  ahrs[1] = y / times;
  ahrs[2] = z / times;
}

void compute_pitch_roll_bg(void* pvParameters) {
  /*
  This is a task to run in the BG that will only compute the pitch and roll.
  The goal is to do it very frequently to increase the precision, in particular in fast movement.
  */
  float pitch, roll;
  while (1) {
    compute_pitch_roll(&pitch, &roll);    
    delay(5);
  }
}

void return_pitch_roll(float *pitch, float *roll, float *accel_y){
  /*
  The only goal of this function is to access from outside to the computed pitch and roll variable.
  This is to avoid declaring extern variable everywhere.
  */
  *pitch = phi;
  *roll = theta;  
}

void compute_pitch_roll(float *pitch, float *roll){

  GetGyroData(250, gyro);
  GetAccelData(250, accel);

  float curr_accel[3], curr_gyro[3];
  for (int i = 0; i < 3; i++){
    curr_accel[i] = accel[i] - offset_accel[i];
    curr_gyro[i] = gyro[i] - offset_gyro[i];    
  }

  thetaM=-atan2(curr_accel[0],curr_accel[2])/3.141592654*180;
  phiM=-atan2(curr_accel[1],curr_accel[2])/3.141592654*180;
   
  dt=(millis()-millisOld)/1000.0;
  millisOld=millis();
  //Theta = around Y axis
  theta=(theta+curr_gyro[1]*dt)*ratio_gyro+thetaM*ratio_accel;
  //Phi = around X axis
  phi=(phi-curr_gyro[0]*dt)*ratio_gyro+ phiM*ratio_accel;
  
  *pitch = phi;
  *roll = theta;  
}

void compute_lean(float *lean, float *accel, float *offset_accel){  
  float z = accel[2] - offset_accel[2];
  if (z > 1.0) z = 1.0;  
  *lean = 180 * acos(z)/PI;
}

void calibrationGryo(Preferences* preferences) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20); 
  M5.Lcd.printf("Start gryo calibration\r\n");
  M5.Lcd.printf("Please keep M5Stack still for 2 seconds\r\n");
  delay(2500);

  float offset[3] = {0.0F, 0.0F, 0.0F};  
  GetGyroData(2500, offset);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.printf("Finish calibration !!!\r\n");

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("offset gyro X: %f\r\n", offset[0]);  
  M5.Lcd.printf("offset gyro Y: %f\r\n", offset[1]);  
  M5.Lcd.printf("offset gyro Z: %f\r\n", offset[2]);
  
  preferences->putFloat("offset_gyro_x", offset[0]);
  preferences->putFloat("offset_gyro_y", offset[1]);
  preferences->putFloat("offset_gyro_z", offset[2]);
    
  delay(1000);
  M5.Lcd.fillScreen(BLACK);
}

void calibrationAccel(Preferences* preferences) {
  //Warning, the Z acceleration data should be 1.0, not 0.0
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20); 
  M5.Lcd.printf("Start acceleration calibration\r\n");
  M5.Lcd.printf("Please keep M5Stack still for 2 seconds\r\n");
  delay(2500);

  float offset[3];
  
  GetAccelData(2500, offset);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.printf("Finish calibration !!!\r\n");

  //The offset from the Z axis should be offsetZ-1.0
  offset[2] = offset[2]-1.0;

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("Accel X: %f\r\n", offset[0]);  
  M5.Lcd.printf("Accel Y: %f\r\n", offset[1]);  
  M5.Lcd.printf("Accel Z: %f\r\n", offset[2]);
  
  preferences->putFloat("offset_acc_x", offset[0]);
  preferences->putFloat("offset_acc_y", offset[1]);
  preferences->putFloat("offset_acc_z", offset[2]);
    
  delay(1000);
  M5.Lcd.fillScreen(BLACK);
}

void calibrationAhrs(Preferences* preferences) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20); 
  M5.Lcd.printf("Start Ahrs calibration\r\n");
  M5.Lcd.printf("Please keep M5Stack still for 5 seconds\r\n");
  delay(2500);

  float offset[3];  
  GetAhrsData(2000, offset);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.printf("Finish calibration !!!\r\n");

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("Pitch: %f\r\n",  offset[0]);  
  M5.Lcd.printf("Roll: %f\r\n",  offset[1]);  
  M5.Lcd.printf("Yaw: %f\r\n",  offset[2]);
  
  preferences->putFloat("offset_pitch",  offset[0]);
  preferences->putFloat("offset_roll",  offset[1]);
  preferences->putFloat("offset_yaw",  offset[2]);
    
  delay(5000);
  M5.Lcd.fillScreen(BLACK);
}

void get_imu_preferences(Preferences* preferences){

  offset_gyro[0] = preferences->getFloat("offset_gyro_x", 0.0);
  offset_gyro[1] = preferences->getFloat("offset_gyro_y", 0.0);
  offset_gyro[2] = preferences->getFloat("offset_gyro_z", 0.0);

  offset_accel[0] = preferences->getFloat("offset_acc_x", 0.0);
  offset_accel[1] = preferences->getFloat("offset_acc_y", 0.0);
  offset_accel[2] = preferences->getFloat("offset_acc_z", 0.0);  

}