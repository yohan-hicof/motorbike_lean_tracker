# motorbike_lean_tracker
A lean angle tracker for motorbike based on a M5stack using also gps position

This is a simple project for a lean angle tracker for motorbike while on racetracks.
It uses a M5Stack core2 with the intgrated IMU (but would work with previous version) and the gps module.
When started it display the current speed, direction, lean angle and a short historic of the last 60 seconds.
It regularly saves on the SD card the data recorded.

The computation of the lean angle is based on the accelerometer and the gyroscope. This allows to have a measure that should be independant from acceleration and only measure the lean angle. The method is based on the exelent series of videos from Paul McWhorter: https://toptechboy.com/arduino-based-9-axis-inertial-measurement-unit-imu-based-on-bno055-sensor/ and modified to fit my need and work with the IMU of the stack.

#TODO

-Improve the description and add images.

-Allow the recorded data to be replayed.

-Create another interface to see the track according to the gps data.

-Change the different background from drawn to images.

-Do the configuration interface for screen brightness, IMU calibration, and time update.

-Create a python script to allow data visualisation from the saved file.

-Find a way to estimate the laptime based on gps data.
