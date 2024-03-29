#include "main.hpp"

extern char save_file_name[64];
extern File dataFile;
extern bool sd_card_found;


int write_data_to_file(double_chain* tail, int nb_links){
  /*
  Starting from the tail, write n links on the SD card.
  Return the number of link written  
  */
  if (!sd_card_found) return 0;
  if (tail == NULL) return 0;

  int nb_written = 0;
  double_chain *current = tail;
  dataFile = SD.open(save_file_name, FILE_APPEND);

  if (dataFile == NULL) return 0;
  while (current != NULL && nb_written < nb_links){
    if (!current->saved){//Save only if it is not already saved
      dataFile.write((uint8_t *)&(current->data), sizeof(data_point)/sizeof(uint8_t));
      nb_written++;
      current->saved=true;
    }
    current = current->previous;
  }
  dataFile.close();
  return nb_written;
}

int write_data_to_file_v2(double_chain* tail, int nb_links){
  /*
  This version should be optimized.
  1) Allocate memory for all the link to write.
  2) Iterate over the chain to copy all the links
  3) Count the number of link found (in case of)
  4) Write at once all the link on the sd card.
  Starting from the tail, write n links on the SD card.
  Return the number of link written
  
  */
  if (!sd_card_found) return 0;
  if (tail == NULL) return 0;

  int nb_written = 0;
  data_point* list_data_points = (data_point*) malloc(nb_links*sizeof(data_point));
  double_chain *current = tail;
  //Loop over the chain to find up to nb_links
  while (current != NULL && nb_written < nb_links){
    if (!current->saved){//Save only if it is not already saved
      list_data_points[nb_written] = current->data;      
      nb_written++;
      current->saved=true;
    }
    current = current->previous;
  }
  dataFile = SD.open(save_file_name, FILE_APPEND);
  if (dataFile == NULL) return 0;
  dataFile.write((uint8_t *)list_data_points, nb_written*sizeof(data_point)/sizeof(uint8_t));  
  dataFile.close();
  free(list_data_points);
  return nb_written;
}

double_chain* read_data_from_file(char* file_name){
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
      //M5.Lcd.printf("Creating head or new node ...");
      //M5.Lcd.printf("for %d (%d)\n", current->data.date, nb_read);
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
    sprintf(save_file_name, "/%d_%02d_%02d_%02d_%02d_%02d.bin", RTCDate.Year,
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
    if (name1[i] == '\0' && name2[i] == '\0') return 0;
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
  *nb_files = 0;
  File root = fs.open("/");
  if(!root) return list_files;    
  if(!root.isDirectory()) return list_files;      
  //First count the number of files.  
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
  char **list_files;
  int nb_files, current_selection = 0, start_index, stop_index;

  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(BLACK);
  draw_selection_arrows(); 
  M5.Lcd.setCursor(0, 10); 

  if (!sd_card_found){
    M5.Lcd.printf("No SD card detected\n");
    return false;
  }   
   
  list_files = list_dir_root(fs, list_files, &nb_files);  
  //If nothing found, return false, else display the selection.
  if (nb_files == 0){
    M5.Lcd.printf("No file found on the SD card\n");      
    return false;
  }
  //Sort the files, there are few,, we do a bubble sort.
  for (int i = 0; i < nb_files; i++){
    for (int j = i+1; j < nb_files; j++){
      if (compare_files_name(list_files[i], list_files[j]) == -1){
        char* temp = list_files[i];
        list_files[i] = list_files[j];
        list_files[j] = temp;
      }
    }
  }


  while (true){
    M5.Lcd.setCursor(0, 10); 
    M5.Lcd.printf("List_files: \n");    
    start_index = max(0, min(current_selection-15, nb_files-20));
    stop_index = min(20, nb_files);
    for (int i = 0; i < stop_index && i+start_index < nb_files; i++){
      //display differently the selected file to show it clearly
      if (i+start_index == current_selection){
        M5.Lcd.setTextColor(TFT_PURPLE, TFT_LIGHTGREY);
        M5.Lcd.printf("   %-45s\n", list_files[i+start_index]);
        M5.Lcd.setTextColor( GREEN, BLACK);
      }
      else{
        M5.Lcd.printf("   %-45s\n", list_files[i+start_index]);
      }      
    }
    //draw_selection_arrows();
    M5.update();
    if (M5.BtnA.wasPressed()){
      if (current_selection == nb_files-1) current_selection = 0;
      else current_selection++;            
    }
    if (M5.BtnB.wasPressed()) {
      sprintf(selected_path, "%s",list_files[current_selection]);
      M5.update();
      delay(250);      
      return true;
    }
    if (M5.BtnC.wasPressed()) {
      if (current_selection == 0) current_selection = nb_files-1;
      else current_selection--;
    }

    delay(100);
  }
  return true;
}

bool backup_single_file(fs::FS &fs, char* file_path){
  /*
  Take a single file at the root of the SD card.
  Extract the date from the file name, if no date, do nothing.
  If the folder with the date name exists, move the file to this folder, else create the folder then move.
  */
  //Check if the file exists, if its name seems to be what we expect, and if it is large enough
  if (!fs.exists(file_path)) return false;

  char folder_name[16];
  char out_name[64];
  uint8_t buf[64];
  uint8_t n;
  strncpy(folder_name, file_path, 11); //The format is /YYYY_MM_DD_HH_MM_SS.bin, we keep /YYYY_MM_DD
  folder_name[11] = '\0'; //Add the end of string char.  

  strncpy(out_name, folder_name, 11);//First the folder name  
  strcpy(out_name+11, file_path);//Then the file name with the ending char

  //Create the folder is necessary
  if (!fs.exists(folder_name)) fs.mkdir(folder_name);

  if (fs.exists(out_name)) return false;
  
  File file_in = fs.open(file_path, FILE_READ);
  if (!file_in) return false;
  File file_out = fs.open(out_name, FILE_WRITE);
  if (!file_out) return false;

  while ((n = file_in.read(buf, sizeof(buf))) > 0) {
    file_out.write(buf, n);
  }
  file_in.close();
  file_out.close();

  fs.remove(file_path);
  return true;
}

bool backup_all_file(fs::FS &fs){
  //Serial.println("Received the command to backup");
  int nb_files = 0;
  char** list_files = list_dir_root(fs, list_files, &nb_files);  
  for (int i = 0; i < nb_files; i++){
    //Serial.printf("Back up: %s\n", list_files[i]);
    backup_single_file(fs, list_files[i]);
  }
  //Serial.println("Done backing");
  return true;
}

bool M5Screen2bmp(const char * path){
  //Function to take a screenshot and write on the SD card.
  //Copied from there:
  //https://www.hackster.io/hague/m5stack-screen-capture-and-remote-control-142cfe
    
  File file = SD.open(path, FILE_WRITE);

  if(file != NULL){
    int image_height = M5.Lcd.height();
    int image_width = M5.Lcd.width();
    const uint pad=(4-(3*image_width)%4)%4;
    uint filesize=54+(3*image_width+pad)*image_height; 
    unsigned char header[54] = { 
      'B','M',  // BMP signature (Windows 3.1x, 95, NT, …)
      0,0,0,0,  // image file size in bytes
      0,0,0,0,  // reserved
      54,0,0,0, // start of pixel array
      40,0,0,0, // info header size
      0,0,0,0,  // image width
      0,0,0,0,  // image height
      1,0,      // number of color planes
      24,0,     // bits per pixel
      0,0,0,0,  // compression
      0,0,0,0,  // image size (can be 0 for uncompressed images)
      0,0,0,0,  // horizontal resolution (dpm)
      0,0,0,0,  // vertical resolution (dpm)
      0,0,0,0,  // colors in color table (0 = none)
      0,0,0,0 };// important color count (0 = all colors are important)
    // fill filesize, width and heigth in the header array
    for(uint i=0; i<4; i++) {
        header[ 2+i] = (char)((filesize>>(8*i))&255);
        header[18+i] = (char)((image_width   >>(8*i))&255);
        header[22+i] = (char)((image_height  >>(8*i))&255);
    }
    // write the header to the file
    file.write(header, 54);
    
    // To keep the required memory low, the image is captured line by line
    unsigned char line_data[image_width*3+pad];
    // initialize padded pixel with 0 
    for(int i=(image_width-1)*3; i<(image_width*3+pad); i++){
      line_data[i]=0;
    }    
    for(int y=image_height; y>0; y--){     
      M5.Lcd.readRectRGB(0, y-1, image_width, 1, line_data);
      for(int x=0; x<image_width; x++){
        unsigned char r_buff = line_data[x*3];
        line_data[x*3] = line_data[x*3+2];
        line_data[x*3+2] = r_buff;
      }
      // write the line to the file
      file.write(line_data, (image_width*3)+pad);
    }
    file.close();    
    return true;
  }
  return false;
}