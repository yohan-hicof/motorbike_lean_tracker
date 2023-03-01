#include "main.hpp"

extern char save_file_name[64];
extern File dataFile;
extern bool sd_card_found;


int write_data_to_file(double_chain* tail, int nb_links){
  /*
  Starting from the tail, write n links on the SD card.
  Return the number of link written
  TODO find a solution to not write the pointers
  */
  if (!sd_card_found) return 0;

  int nb_written = 0;
  double_chain *current = tail;
  dataFile = SD.open(save_file_name, FILE_APPEND);

  if (dataFile == NULL) return 0;
  while (current != NULL && nb_written < nb_links){
    if (!current->saved){//Save only if it is not already saved
      dataFile.write((uint8_t *)&(current->data), sizeof(data_point)/sizeof(uint8_t));
      nb_links++;
      current->saved=true;
    }
    current = current->previous;
  }
  dataFile.close();
  return nb_links;
}

double_chain* read_data_to_file(char* file_name){
  /*
  Read the given file data to recreate the linked list containing all the data.
  Return the pointer to the new head
  */
  if (!sd_card_found) return NULL;

  int nb_read = 0;
  double_chain *current = NULL, *head = NULL, *temp_node = NULL;
  dataFile = SD.open(file_name, FILE_READ);
  if (!dataFile) {
    M5.Lcd.printf("File not opened\n"); 
    delay(1000);
    return NULL;
  }
  else{
    M5.Lcd.printf("File size: %d\n", dataFile.size()); 
  }
  
  do{    
    current = (double_chain *) malloc(sizeof(double_chain));
    if (current == NULL) return head;    
    //nb_read = fread(&current, sizeof(struct datapoint), 1, dataFile);    
    nb_read = dataFile.read((uint8_t *)&(current->data), sizeof(data_point)/sizeof(uint8_t));
    
    if (nb_read > 0){
      current->saved = false;
      M5.Lcd.printf("Creating head or new node ...");
      M5.Lcd.printf("for %d (%d)\n", current->data.date, nb_read);
      if (head == NULL){
        head = current;
        head->next = NULL;
        head->previous = NULL;
      }
      else{
        temp_node = head;
        head = current;
        head->previous = NULL;
        head->next = temp_node;
        temp_node->previous = head;
      }
    }
    else{
      free(current);
    }
  }while (nb_read > 0);  
  dataFile.close();

  return head;
}

void check_sd_card(){    
  if (SD.begin()) {
    sd_card_found = true;
  }
}

void create_save_file_name(){  
  if (!sd_card_found) check_sd_card();
  if (sd_card_found) {    
    RTC_TimeTypeDef RTCtime; //If we save, we need a timestamp
    RTC_DateTypeDef RTCDate; //If we save, we need a timestamp
    M5.Rtc.GetTime(&RTCtime);                             
    M5.Rtc.GetDate(&RTCDate);
    sprintf(save_file_name, "/%d_%02d_%02d_%02d_%02d_%02d.csv", RTCDate.Year,
            RTCDate.Month, RTCDate.Date, RTCtime.Hours, RTCtime.Minutes,
            RTCtime.Seconds);    
  }
}

void draw_selection_arrows(){
  /* At the bottom of the screen draw the selection information for the user.
  */
  
  M5.lcd.fillRect(0, 200, 320, 40, BLACK);
  M5.lcd.drawRect(0, 200, 320, 40, TFT_DARKGREY);
  M5.lcd.drawFastVLine(100, 200, 40, TFT_DARKGREY);
  M5.lcd.drawFastVLine(220, 200, 40, TFT_DARKGREY);
  //Down arrow
  M5.lcd.fillTriangle(37, 205, 67, 205, 52, 235, TFT_DARKGREY);
  //Up arrow
  M5.lcd.fillTriangle(247, 235, 277, 235, 262, 205, TFT_DARKGREY);
  //Validate
  M5.Lcd.drawChar('O', 150, 210, 2);
  M5.Lcd.drawChar('K', 160, 210, 2);
}

int compare_files_name(const char *name1, const char *name2){
  
  int i = 0;
  while(name1[i] == name2[i]) {
    if (name1[i] == '\0' && name1[i] == '\0') return 0;
    i++;
  }
  if (name1[i] < name2[i]) return 1;
  return -1;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){

    // Print blank line on screen
    M5.Lcd.setTextSize(1);
    M5.lcd.fillRect(0, 0, 320, 240, BLACK);
    M5.Lcd.printf("\n\n");        
    M5.Lcd.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){        
        M5.Lcd.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){        
        M5.Lcd.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){            
            M5.Lcd.print("  DIR : ");            
            M5.Lcd.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {            
            M5.Lcd.print("  FILE: ");            
            M5.Lcd.print(file.name());            
            M5.Lcd.print("  SIZE: ");            
            M5.Lcd.println(file.size());
        }
        file = root.openNextFile();
    }
    M5.Lcd.setTextSize(2);
}

char** list_dir_root(fs::FS &fs, char** list_files, int* nb_files){
  // Return tre list of files found in the root directory.
  File root = fs.open("/");
  if(!root) return list_files;    
  if(!root.isDirectory()) return list_files;    
  //First count the number of files.  
  *nb_files = 0;
  File file = root.openNextFile();
  while(file){        
    if(!file.isDirectory()) (*nb_files)++;
    file = root.openNextFile();
  }  
  //Now allocate and return the number of files
  root.rewindDirectory();
  list_files = (char**)malloc(*nb_files*sizeof(char*));
  *nb_files = 0;  
  file = root.openNextFile();
  while(file){        
    if(!file.isDirectory()){      
      list_files[*nb_files] = (char*)malloc(64*sizeof(char));      
      sprintf(list_files[*nb_files], "/%s", file.name());
      (*nb_files)++;
    }
    file = root.openNextFile();
  }
  return list_files;
}


bool select_file(fs::FS &fs, char* selected_path){
  //Function to select a file from the root dir
  M5.Lcd.setTextSize(1);
  char **list_files;
  int nb_files, current_selection = 0;
  M5.Lcd.setCursor(0, 10);  
  list_files = list_dir_root(fs, list_files, &nb_files);  
  //If nothing found, return false, else display the selection.
  if (nb_files == 0){
      M5.Lcd.printf("No file found on the SD card\n");      
      return false;
    }
  while (true){
    M5.Lcd.setCursor(0, 10); 
    M5.Lcd.printf("List_files: \n");    
    for (int i = 0; i < nb_files; i++){
      if (i == current_selection){
        M5.Lcd.setTextColor(TFT_PURPLE, TFT_LIGHTGREY);
        M5.Lcd.printf("   %s\n", list_files[i]);
        M5.Lcd.setTextColor( GREEN, BLACK);
      }
      else{
        M5.Lcd.printf("   %s\n", list_files[i]);
      }      
    }
    draw_selection_arrows();
    M5.update();
    if (M5.BtnA.wasPressed()) current_selection = min(nb_files-1, current_selection+1);
    if (M5.BtnB.wasPressed()) {
      sprintf(selected_path, "%s",list_files[current_selection]);
      M5.update();
      delay(250);      
      return true;
    }
    if (M5.BtnC.wasPressed()) current_selection = max(0, current_selection-1);

    delay(100);
  }

  return true;

}