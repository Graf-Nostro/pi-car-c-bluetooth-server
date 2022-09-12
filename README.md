# PI Car 

Toying around with a bluetooth controlled remote car. Sporting the raspberry pi and an Android client controlling a little car with electric motors and wheels.

## Components

 - Server written in C taking orders over bluetooth
 - Client Java Android App, Software analog stick driving the car.

### Setup with Bluez (linux bluetooth library) in discovery mode 

`sudo hciconfig hci0 piscan`

*Bluetooth interface up and running*

`sudo haciconfig hci0 up`

*list results*

`sudo hciconfig -a`

### Server start

need to start bluethooth server with "-C" command and root

*kill deamon*

`ps ax | grep bluetooth`

then

`sudo kill PID`

restart with root and -C for compatibility reasons

`sudo bluetoothd -C &`


