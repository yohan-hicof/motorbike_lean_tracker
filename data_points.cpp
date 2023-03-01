#include "main.hpp"

extern TinyGPSPlus gps;


double_chain* create_new_data_point(){
  /*
  Create a new data point and return its pointer.
  Return NULL if the allocation failed.
  */  
  double_chain* new_link = NULL;
  new_link = (double_chain *) malloc(sizeof(double_chain));
  if (new_link == NULL) return NULL;  
      
  return_pitch_roll(&(new_link->data.pitch), &(new_link->data.roll));  
  //new_link->data.pitch = fabs(new_link->data.pitch);
  //new_link->data.roll = fabs(new_link->data.roll);
  new_link->data.speed = constrain(gps.speed.kmph(), 20, 260); //We do not want to display speed below 20kmph
  new_link->data.direction = gps.course.deg();
  new_link->data.lat = gps.location.lat();
  new_link->data.lng = gps.location.lng();
  new_link->data.date = gps.date.value();
  new_link->data.time = gps.time.value();  
  new_link->saved = false;  
  new_link->next = NULL;
  new_link->previous = NULL;
  return new_link;
}

double_chain* create_dummy_data_point(uint32_t index){
  /*
  This is for testing only.
  The index is saved in the time and date, it allows to track what is going on, the order...
  Create a dummy data point and return its pointer.
  Return NULL if the allocation failed.
  */

  double_chain* new_link = NULL;
  new_link = (double_chain *) malloc(sizeof(double_chain));
  if (new_link == NULL) return NULL;
  
  new_link->data.pitch = 30.0;
  new_link->data.roll = 30.0;
  new_link->data.speed = 100.0;
  new_link->data.direction = 90.0;
  new_link->data.lat = 8.15236;
  new_link->data.lng = 42.123456;
  new_link->data.date = index;
  new_link->data.time = index;
  new_link->saved = false;

  new_link->next = NULL;
  new_link->previous = NULL;
  return new_link;
}

double_chain* find_last_link(double_chain* head){
  /*
  Take the head of the linked list and return the last one
  Probably useless since we have a double linked chain
  */
  double_chain* current = head;
  while (current->next != NULL) {      
      current = current->next;
  }
  return current;
}

double_chain* add_new_head(double_chain* head, double_chain* new_head){    
  if (new_head == NULL) return head;
  head->previous = new_head;
  new_head->previous = NULL;
  new_head->next = head;
  return new_head;
}

double_chain* remove_head(double_chain* head){
  /*
  Remove the head of the queue
  Return the pointer to the new head
  */
  if (head == NULL) return NULL;
  double_chain* retour = head->next;
  retour->previous = NULL;
  free(head);
  return retour;
}

double_chain* remove_tail(double_chain* tail){
  /*
  Remove the tail of the list
  Return the pointer to the new tail
  */
  if (tail == NULL) return NULL;
  double_chain* retour = tail->previous;
  retour->next = NULL;
  free(tail);
  return retour;
}

int count_nb_links(double_chain* head){
  if (head == NULL) return 0;
  int retour = 1;
  double_chain* current = head;
  while (current->next != NULL) {
      /*Serial.print("ID/is saved ?");
      Serial.print(current->data.date);
      Serial.print("/");
      Serial.println(current->saved);*/
      current = current->next;
      retour++;
  }
  return retour;
}

double_chain* delete_after_n_links(double_chain* head, int n, bool only_saved){
  /*
  Follow the head during n links. Deleted all links after that.
  Return the new tails.
  only_saved -> Only delete the links if all the link to be deleted were saved.
  Note that normally if n is saved, n+1 should be saved.
  */

  //Go to the nth element
  int index = 0;
  bool all_saved = true;
  double_chain *current = head, *new_tail = NULL, *to_free = NULL;
  while (current->next != NULL && index < n) {
      current = current->next;
      index++;
  }

  new_tail = current;
  //If we reached the end before we should delete, just return the current tail.
  if (new_tail->next == NULL) return new_tail;  
  //Check if all were saved (if requested)
  if (only_saved){
    while (current->next != NULL) {      
      current = current->next;     
      if (!current->saved) all_saved = false; //They are not all saved, we return the current tail
    }
    if (!all_saved) return current; //We are at the end of the list
  }
  
  //Move back to after the tail and start to delete
  current = new_tail->next;
  while (current != NULL){
    to_free = current;
    current = current->next;
    free(to_free);
  } 
  //All done, return the new tail
  return new_tail;
}

double_chain* delete_n_links_from_tails(double_chain* tail, int n, bool only_saved){
  /*
  Delete up to n link starting from the tail.
  If less than n links in the chain, just delete everything and return null
  */
  
  int nb_removed = 0;  
  double_chain *current = tail, *to_free = NULL;  
  while (current != NULL && nb_removed < n){
    //If this one is not saved, we do not deleted. No need to check up the chain either    
    if (!current->saved && only_saved) break;
    to_free = current;
    current = current->previous;
    current->next = NULL;
    free(to_free);
    nb_removed++;
  }  
  //All done, return the new tail
  return current;
}
