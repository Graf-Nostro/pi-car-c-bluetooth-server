#!/bin/bash

# Init bluetooth for server

# restart bluetooth deamon neet root
sudo kill $(ps ax | pgrep "bluetoothd")

sudo bluetoothd -C &

# set bluetooth to discovery mode and set interface active
sudo hciconfig hci0 up

sudo hciconfig hci0 piscan


