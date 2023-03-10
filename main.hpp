/*
TODO next
Change the creation of the backgroun to a sprite.
Learn to load/save sprites.
*/


#include <M5Core2.h>
#include <TinyGPSPlus.h>
#include <Preferences.h>
#include <FastLED.h>

//All the part related to the data points
//So many points
typedef struct data_point {
    float pitch; // 
    float roll; // 
    double speed; //
    double direction; // Direction of the bike from the GPS
    double lat; // Current latitude
    double lng; // Current longitude
    uint32_t date; // Current date
    uint32_t time; // Current time    
} data_point_t;

typedef struct double_chain{
  struct data_point data;
  bool saved = false; //Was this point saved on the sd card ?    
  struct double_chain* next;
  struct double_chain* previous;
}double_chain_t;

void feed_gps_bg(void* pvParameters); //To replace smartDelay, feed the gps in the background

//Functions linked to the data structure.
double_chain* create_new_data_point();
double_chain* create_dummy_data_point(uint32_t index);
double_chain* find_last_link(double_chain* head);
double_chain* add_new_head(double_chain* head, double_chain* new_head);
double_chain* remove_head(double_chain* head);
double_chain* remove_tail(double_chain* tail);
int count_nb_links(double_chain* head);
double_chain* delete_after_n_links(double_chain* head, int n = 200, bool only_saved = true);
double_chain* delete_n_links_from_tails(double_chain* tail, int n, bool only_saved = true);

//Configuration functions
void tracker_menu();
void config_menu();
void set_brightness();
void set_time();

//Function to write the data to SD, and to recreate a chain from a file
int write_data_to_file(double_chain* tail, int nb_links);
double_chain* read_data_to_file(char* file_name);
void check_sd_card();
void create_save_file_name();
bool M5Screen2bmp(const char * path);


//Functions linked to the IMU
void GetGyroData(uint16_t times, float* gyro);
void GetAccelData(uint16_t times, float* accel);
void GetAhrsData(uint16_t times, float* ahrs);

void calibrationGryo(Preferences* preferences);
void calibrationAccel(Preferences* preferences);
void calibrationAhrs(Preferences* preferences);

void compute_pitch_roll_bg(void* pvParameters);
void return_pitch_roll(float *pitch, float *roll);

void return_pitch_roll(float *pitch, float *roll);
void compute_pitch_roll(float *pitch, float *roll);
void compute_lean(float *lean, float *accel, float *offset_accel);

void get_imu_preferences(Preferences* preferences);

//Main function for the tracker
void wait_for_gps();
void main_tracker_loop(uint8_t display_type = 0);

//Displaying the current state
void format_date_time(uint32_t date, uint32_t time, char* string_time);
void format_time(uint32_t time, char* string_time);
void time_difference(uint32_t time_beg, uint32_t time_end, uint32_t* time_diff);

void update_led(int angle);

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
void create_needle_sprite();
void create_direction_sprite();

void draw_lean_angle_bar(float lean, int w, int h);
void display_data_point_CLI(double_chain* head);
void display_data_point_GUI(double_chain* head);
void display_real_time_GUI(double_chain* head, uint32_t start_time);

//Function linked to the sd card
void draw_selection_arrows();
int compare_files_name(const char *name1, const char *name2);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
char** list_dir_root(fs::FS &fs, char** list_files, int* nb_files);
bool select_file(fs::FS &fs, char* selected_path);

//Function to replay old files
void main_replay();
void extract_abstract_data(double_chain* head);
void replay_standard_GUI(double_chain* tail);
void replay_track_GUI(double_chain* tail);
