# motorbike_lean_tracker
A lean angle tracker for motorbike based on a M5stack using also gps position

This is a simple project for a lean angle tracker for motorbike while on racetracks.
It uses a M5Stack core2 with the intgrated IMU (but would work with previous version) and the gps module.
When started it display the current speed, direction, lean angle and a short historic of the last 60 seconds.
It regularly saves on the SD card the data recorded.

#TODO
-Improve the description and add images.
-Allow the recorded data to be replayed.
-Create another interface to see the track according to the gps data.
-Change the different background from drawn to images.
-Do the configuration interface for screen brightness, IMU calibration, and time update.
-Create a python script to allow data visualisation from the saved file.
-Find a way to estimate the laptime based on gps data.
