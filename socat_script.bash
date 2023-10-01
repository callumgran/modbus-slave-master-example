#!/bin/bash

# Define port numbers to use as virtual serial ports
port_numbers=(32 33)

# Define the pairs of virtual serial ports
port_pair="pty,raw,echo=0,link=/dev/ttyS${port_numbers[0]} pty,raw,echo=0,link=/dev/ttyS${port_numbers[1]}"

socat -d -d $port_pair &

sleep 2

echo "Virtual serial ports are running."

read -p "Press Enter to exit..."

sudo pkill socat
