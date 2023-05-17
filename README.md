# motorbike_lean_tracker
A lean angle tracker for motorbike based on a M5stack using also gps position

This is a simple project for a lean angle tracker for motorbike while on racetracks.
It uses a M5Stack core2 with the intgrated IMU (but would work with previous version) and the gps module.
When started it display the current speed, direction, lean angle and a short historic of the last 60 seconds.
It regularly saves on the SD card the data recorded.

The computation of the lean angle is based on the accelerometer and the gyroscope. This allows to have a measure that should be independant from acceleration and only measure the lean angle. The method is based on the exelent series of videos from Paul McWhorter: https://toptechboy.com/arduino-based-9-axis-inertial-measurement-unit-imu-based-on-bno055-sensor/ and modified to fit my need and work with the IMU of the stack.

Source for the battery: https://community.m5stack.com/topic/2994/core2-how-to-know-the-current-battery-level/9

Tool used for the pixelization of the CBR: https://giventofly.github.io/pixelit/#tryit

Illustration of the interface

![Main menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/main_screen.png)
![Main capture screen](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/capture_screen.png)
![Replay menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/replay_screen.png)
![Configuration menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/config_screen.png)

Source of the modified 3D printed support: https://www.thingiverse.com/thing:2834201
https://www.thingiverse.com/thing:5357667
https://www.thingiverse.com/thing:740221

#TODO
-----For the M5Stack-----

- Improve the description of the readme and add images. (done, will need to be updated)

- Allow the recorded data to be replayed. (in progress): in the track replay, show more info, like current speed, current lean

- Find a way to estimate the laptime based on gps data. (From the python and java app)

- Add simple capture visualization: Just speed and lean. (in progress)

- Add a visualization that create the track in real time (might be to slow, will have to save the last 3 minutes or something)

- Modify the bluetooth function to get a nicer view

- Check the performance, the first test shows only 5 points per seconds. I would like to reach 10. -> Improved to 8pps

-----For the App-----

- Make the main screen background rotating.

- Add a background for the recap screen.

- Improve the design of the buttons

- Try to replay another file after the first one. It might not work, some stuff might have to be reset (onResume ?)

- Improve the lap time computation (speed up the search).

- Interpolate points for having tens of seconds, since the GPS does not give the centi seconds.

Known issues

On the m5 when running the BT, we have to reboot to leave the BT menu.

The lap time are only accurate to the second. This is due to the fact that the GPS only gives update every second.
Even if I interpolate the time, the coordinate is the same, thus it pick the first point that "close" the lap, 
i.e. the one on the second.

According to this post: 
https://forum.arduino.cc/t/gps-module-change-update-rate-to-10hz/665212/8
Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); //GPS baud rate now at original 9600  
Serial2.print("$PCAS02,100*1E\r\n"); 
Should allow to set the frequency to 10hz, I will try that.
This does not work correctly. The GPS only goes up to 2Hz instead of 10. Not sure why.