#include "main.hpp"

//Contains the functions linked to the Bluetooth 

BluetoothSerial SerialBT;
 
void setupBT(){
  SerialBT.begin("LeanTracker");
  delay(100);
}

void receive_command(){

  String inputCommand = "";
  M5.lcd.setCursor(0,50);
  while (SerialBT.available()) {
    delay(10);  //small delay to allow input buffer to fill
    char c = SerialBT.read();  //gets one byte from serial buffer
    if (c == '\n') break; 
    inputCommand += c; 
    M5.Lcd.println(inputCommand);
  }
  M5.lcd.setCursor(10,10);
  M5.Lcd.println(inputCommand);
  //A bug in the windows soft, easier that way
  if (inputCommand == "sendfiles" || inputCommand == "sendfilessendfiles") send_list_files();

}

void send_list_files(){
  
  int nb_files;
  char** list_files = list_dir_root(SD, list_files, &nb_files);  

  for (int i = 0; i < nb_files; i++){
     SerialBT.printf("%s\n", list_files[i]);
  }
  //SerialBT.print("Sending file\n");
  //send_requested_file(list_files[0]);
  //SerialBT.print("\nFile send\n");
}

void send_requested_file(char* name){
  
  File dataFile = SD.open(name, FILE_READ);
  uint8_t* data;
  if (!dataFile) {
    SerialBT.printf("Cannot open %s\n", name);
    delay(1000);
    return;
  }
  else{
    //Send the number of bytes to send via BT
    SerialBT.printf("FS %d\n", dataFile.size()); 
  }
  data = (uint8_t*) malloc(dataFile.size()*sizeof(uint8_t*));
  int nb_read = dataFile.read((uint8_t *)data, dataFile.size()/sizeof(uint8_t));
  if (nb_read == dataFile.size())
    SerialBT.write((uint8_t *)data, dataFile.size()/sizeof(uint8_t));  
  else{
    M5.Lcd.println("Error while reading the file");
    SerialBT.println("Error while reading the file");
  }
  dataFile.close();

}