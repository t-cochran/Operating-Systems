#!/bin/bash

#  Create the device file
if ! [ -f /dev/simple_character_device ]
then
    printf "\033[1;33mCreating /dev/simple_character_device...\n\033[0m"
    sudo mknod /dev/simple_character_device c 200 0
	sudo chmod 777 /dev/simple_character_device
fi

#  Install the module
sudo dmesg -C
sudo insmod my_driver.ko
printf "\033[1;33m[KERNEL MODULE INSTALLED]\n\033[0m"
