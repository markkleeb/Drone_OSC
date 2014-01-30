#OSC Sending Commands to Node.JS to control AR Parrot Drone

This sketch runs an OpenCV window on top of a node-browser window and transmits commands to Node.JS to fly the AR Parrot Drone. Current code includes automatic color tracking commands, as well as key press commands for the 8 movements, takeoff, and landing.

OSC commands transmit every 100 ms, which are then parsed by the Node server and relayed to the drone.