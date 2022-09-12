##
## Project: BluetoothRobot
## File:    Makefile
## Version: 0.1
##
## Author:  stf
##

APP      =  BluetoothRobot
SERVER   =  BtRobotRaspPIServer
DRIVER   =  MotorHatDriver
GCC      =  gcc
ARM      =  
OPT      = -lbluetooth -lm -l wiringPi
DEBUG    = -g

all: main

main: $(SERVER).c $(DRIVER).c $(DRIVER).h
	$(GCC) -o $(APP) $(SERVER).c $(DRIVER).c $(DRIVER).h  $(OPT)

clean:
	rm $(APP)
