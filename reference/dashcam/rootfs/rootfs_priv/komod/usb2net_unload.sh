#!/bin/sh

cd /komod/
rmmod g_multi.ko
rmmod usb_f_rndis.ko
rmmod usb_f_acm.ko
rmmod u_serial.ko
rmmod u_ether.ko
rmmod libcomposite.ko

exit
