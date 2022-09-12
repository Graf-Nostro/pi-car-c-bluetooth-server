#bluez discovery mode
sudo hciconfig hci0 piscan

#bluez interface 
sudo haciconfig hci0 up

#sudo hciconfig -a should list results
sudo hciconfig -a

#need to start bluethooth server with "-C" command and root

#kill deamon
ps ax | grep bluetooth
sudo kill PID

#restart with root and -C for compatibility reasons
sudo bluetoothd -C &


