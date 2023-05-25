import struct

nb_data_point = 0
with open("2023_05_21_15_53_25.bin",'rb') as f:
    chunk = f.read(44)
    while chunk != "":
       data_point = struct.unpack("<fffffddII", chunk)
       print(data_point)
       chunk = f.read(44)
       nb_data_point += 1
       if nb_data_point >= 200:
          break


# typedef struct __attribute__ ((packed)) data_point {
#    float pitch; // The whillie/stoppy of the bike
#    float roll; // The lean of the motorbike
#    float acceleration; // Y acceleration
#    float speed; //
#    float direction; // Direction of the bike from the GPS
#    double lat; // Current latitude
#    double lng; // Current longitude
#    uint32_t date; // Current date
#    uint32_t time; // Current time    
#} data_point_t;
