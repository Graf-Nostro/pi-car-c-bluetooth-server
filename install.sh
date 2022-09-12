#!/bin/sh

#update package lists
apt-get update

#install bluez
apt-get install bluez libbluetooth-dev -y

#make bluetooth device discoverable
hciconfig hci0 piscan

