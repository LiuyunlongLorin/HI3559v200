#!/bin/sh

cd /komod/
insmod libcomposite.ko
insmod usb_f_mass_storage.ko
insmod g_mass_storage.ko

exit
