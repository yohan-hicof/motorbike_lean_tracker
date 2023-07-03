#include "main.hpp"

//Contains the functions linked to the Bluetooth 

BluetoothSerial SerialBT;

extern const unsigned char bluetooth[28907];
 
void setupBT(){
  SerialBT.begin("LeanTracker");
  delay(100);
}

void closeBT(){
  delay(500); //To be sure we finish the current file.
  SerialBT.disconnect();
  delay(100);
  SerialBT.end();
  delay(100);
}

void receive_command_CLI(){

  String inputCommand = "";
  M5.lcd.setCursor(0,50);
  M5.Lcd.println("Waiting for a command");
  while (SerialBT.available()) {
    delay(10);  //small delay to allow input buffer to fill
    char c = SerialBT.read();  //gets one byte from serial buffer
    if (c == '\n') break; 
    inputCommand += c; 
    //M5.Lcd.println(inputCommand);
  }
  M5.lcd.setCursor(0,10);
  M5.Lcd.println(inputCommand);
  //A bug in the windows soft, easier that way
  if (inputCommand == "sendfiles") send_list_files();
  if (inputCommand.substring(0,2) == "rf") {
    String fn = inputCommand.substring(3);
    char filename[fn.length()+1];
    fn.toCharArray(filename, fn.length()+1);
    M5.Lcd.println(fn);
    M5.Lcd.println(filename);
    send_requested_file(filename);
  }

}

void receive_command_GUI(){ 

  while(1){
    M5.Lcd.drawJpg(bluetooth, 28907, 0,0,320,240);
    Event& e = M5.Buttons.event;

    if (e & E_TOUCH) {
      if (e.to.x > 240 && e.to.y > 130){
        //Click to quit
        delay(50);
        return;
      }
    }

    String inputCommand = "";  
    while (SerialBT.available()) {
      delay(10);  //small delay to allow input buffer to fill
      char c = SerialBT.read();  //gets one byte from serial buffer
      if (c == '\n') break; 
      inputCommand += c;     
    }
    //if (inputCommand.length() > 0) Serial.printf("Received command: %s\n", inputCommand);
    //A bug in the windows soft, easier that way
    if (inputCommand == "sendfiles") send_list_files();
    if (inputCommand == "backupfiles") backup_all_file(SD);
    if (inputCommand.substring(0,2) == "rf") {
      String fn = inputCommand.substring(3);
      char filename[fn.length()+1];
      fn.toCharArray(filename, fn.length()+1);    
      send_requested_file(filename);
    }
  }
}

void send_list_files(){
  
  int nb_files;
  char** list_files = list_dir_root(SD, list_files, &nb_files);
  SerialBT.printf("sending files\n");
  for (int i = 0; i < nb_files; i++){
     SerialBT.printf("%s\n", list_files[i]);
  }
  SerialBT.printf("EOF\n");  
}

void send_requested_file(char* name){
  
  File dataFile = SD.open(name, FILE_READ);
  uint8_t* data;
  if (!dataFile) {
    //SerialBT.printf("Cannot open %s\n", name);
    SerialBT.printf("ERROR\n");
    delay(200);
    return;
  }
  else{
    //Send the number of bytes to send via BT
    SerialBT.printf("FS %d\n", dataFile.size()); 
  }
  data = (uint8_t*) malloc(dataFile.size()*sizeof(uint8_t));
  int nb_read = dataFile.read((uint8_t *)data, dataFile.size()/sizeof(uint8_t));
  Serial.printf("File name: %s\n", name);
  Serial.printf("File size: %d\n", dataFile.size());
  Serial.printf("Read size: %d\n", nb_read);
  if (nb_read == dataFile.size()){
    M5.Lcd.setCursor(120, 20);
    M5.Lcd.println(name);
    Serial.printf("Send file\n");    
    /*for (int i = 0; i < nb_read; i++){//This work but is slow as hell
     //SerialBT.printf("%c", data[i]);
     SerialBT.write((uint8_t *)data + i, sizeof(uint8_t));
     if (i%5000 == 0){
      M5.Lcd.setCursor(120, 40);
      M5.Lcd.printf("[%d/%d]",i, nb_read);
     }*/
    for (int i = 0; i < nb_read; i+=44){     
      SerialBT.write((uint8_t *)&data[i], 44*sizeof(uint8_t));
      if (i%100*44 == 0){
        M5.Lcd.setCursor(120, 40);
        M5.Lcd.printf("[%d/%d]",i, nb_read);
     }
    }

  }
  else{
    Serial.printf("Send Error message\n");
    SerialBT.println("Error while reading the file");
  }    
  Serial.printf("Done\n");
  dataFile.close();

}