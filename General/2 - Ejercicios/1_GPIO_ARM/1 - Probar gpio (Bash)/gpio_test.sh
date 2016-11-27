#!/bin/bash

if [ ! -e /sys/class/gpio/gpio67/value ] #If file doesn't exists
then
    echo 67 > /sys/class/gpio/export #We told the system to export the settings for GPIO_67. It responded by building the folder 'gpio67'
    echo out > /sys/class/gpio/gpio67/direction #We've set this I/O's data direction to an output
fi

while [ True ] 
do
    echo 1 > /sys/class/gpio/gpio67/value #Se activa (se pone a 3.3V). Se supone que el sys/class/gpio/gpio67/active_low está a 0. Ya que si estuviese a 1 querría decir que the meaning of 1 and 0 in the value-file is reversed: 0 (3.3V) and 1 (0.V) 
    sleep 2 #Time in seconds
    echo 0 > /sys/class/gpio/gpio67/value #Se desactiva (Se pone a 0V)
    sleep 2
done
#echo 67 > unexport

#The GPIO port can be set to provide events. The event can trigger on rising edge, falling edge or both.
#echo rising > /sys/class/gpio/gpio67/edge
#No lo usamos en este ejemplo, pero también se podría probar

