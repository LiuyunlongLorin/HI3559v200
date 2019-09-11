#!/bin/sh

cd /komod/
rmmod g_mass_storage.ko
rmmod usb_f_mass_storage.ko
rmmod libcomposite.ko

exit
