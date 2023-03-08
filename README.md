# motorbike_lean_tracker
A lean angle tracker for motorbike based on a M5stack using also gps position

This is a simple project for a lean angle tracker for motorbike while on racetracks.
It uses a M5Stack core2 with the intgrated IMU (but would work with previous version) and the gps module.
When started it display the current speed, direction, lean angle and a short historic of the last 60 seconds.
It regularly saves on the SD card the data recorded.

The computation of the lean angle is based on the accelerometer and the gyroscope. This allows to have a measure that should be independant from acceleration and only measure the lean angle. The method is based on the exelent series of videos from Paul McWhorter: https://toptechboy.com/arduino-based-9-axis-inertial-measurement-unit-imu-based-on-bno055-sensor/ and modified to fit my need and work with the IMU of the stack.

Tool used for the pixelization of the CBR: https://giventofly.github.io/pixelit/#tryit

#TODO

-Improve the description of the readme and add images.

-Allow the recorded data to be replayed. (in progress)

-Create another interface to see the track according to the gps data.

-Change the different background from drawn to images. (in progress)

-Create a python script to allow data visualisation from the saved file.

-Find a way to estimate the laptime based on gps data.

-Add the state of the battery during tracking and visualization.

- Following this: https://www.youtube.com/watch?v=SUAqVUN9AuQ to use sprite to display the needle and stop the flickering.

- Create an image for the main visualization + needle for speed + needle for direction. (in progress)

- Modify the config menu to turn on and off the leds.

- Add in the point capture the led if they are activated.

- Add in the replay the leds if they are activated.


Known issues

- The biker rotate on the center of the bike, should rotate around the tyre.