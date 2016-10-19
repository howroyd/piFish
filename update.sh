#!/bin/bash
clear

echo "Updating..."

sudo apt-get update
sudo apt-get -f -y dist-upgrade
sudo dpkg --configure -a
sudo apt-get -y autoremove
sudo PRUNE_MODULES=1 rpi-update

echo "Done!"

sudo shutdown -r now

# add the following line to sudo crontab -e
# 00 21 * * * /home/pi/update.sh
