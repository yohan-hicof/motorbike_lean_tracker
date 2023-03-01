#include "main.hpp"

void main_replay(){
  //Main function handling the replay.
  //First, we select the file we should replay.
  //Then we load it in memory.
  //Then we select the type of replay
  //Then we call the function doing the replay.

  char replay_file_name[64];

  bool is_valid = select_file(SD, replay_file_name);
  if (!is_valid){
     M5.Lcd.printf("Press any button to return to the menu\n");
     while(1){
      M5.update();
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) 
        return;
      delay(20);
     }
    return;
  }



}