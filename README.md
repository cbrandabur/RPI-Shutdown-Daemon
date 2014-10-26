RPI-Shutdown-Daemon
===================

Simple daemon to monitor a Raspberry PI B/B+ gpio pin and safely shut it down when the button is pressed.
This file is part of Casa-Automata http://casa-automata.com
  

Requires WiringPI: https://projects.drogon.net/raspberry-pi/wiringpi/

Modify the GPIO_IN define to change the pin number to the one you use. If not, by default it will use GPIO 21 (pin 40) on B+

To compile it on Raspberry Pi B+:
```
gcc shutdown_daemon.c -l wiringPi -o shutdown_daemon
```

To start, stop or see the daemons status:
```
./shutdown_daemon start|stop|status
```

B+ Schematic
![Alt text](rpib+button.png?raw=true "Raspberry PI B+ shutdown button wiring diagram")
