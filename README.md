# motorbike_lean_tracker
A lean angle tracker for motorbike based on a M5stack using also gps position

This is a simple project for a lean angle tracker for motorbike while on racetracks.
It uses a M5Stack core2 with the intgrated IMU (but would work with previous version) and the gps module.
When started it display the current speed, direction, lean angle and a short historic of the last 60 seconds.
It regularly saves on the SD card the data recorded.

The computation of the lean angle is based on the accelerometer and the gyroscope. This allows to have a measure that should be independant from acceleration and only measure the lean angle. The method is based on the exelent series of videos from Paul McWhorter: https://toptechboy.com/arduino-based-9-axis-inertial-measurement-unit-imu-based-on-bno055-sensor/ and modified to fit my need and work with the IMU of the stack.

Tool used for the pixelization of the CBR: https://giventofly.github.io/pixelit/#tryit

Illustration of the interface

![Main menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/main_screen.png)
![Main capture screen](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/capture_screen.png)
![Replay menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/replay_screen.png)
![Configuration menu](https://github.com/yohan-hicof/motorbike_lean_tracker/blob/main/screen/config_screen.png)


#TODO

-Improve the description of the readme and add images.

-Allow the recorded data to be replayed. (in progress): in the track replay, show more info, like current speed, current lean

-Create another interface to see the track according to the gps data.

-Change the different background from drawn to images. (in progress)

-Create a python script to allow data visualisation from the saved file.

-Find a way to estimate the laptime based on gps data. (Need data to test that.)

-Add the state of the battery during tracking and visualization. : https://community.m5stack.com/topic/2994/core2-how-to-know-the-current-battery-level/9

- Add in the replay the leds if they are activated.

- Add simple capture visualization: Just speed and lean.

- Add a menu to select visualizationt type.

- In replay track, add button to pause, speed up/ slow down the replay

Known issues

- The biker rotate on the center of the bike, should rotate around the tyre.