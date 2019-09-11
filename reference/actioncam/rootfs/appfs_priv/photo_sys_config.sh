#!/bin/sh
echo 10 > /proc/sys/vm/dirty_background_ratio
echo 20 > /proc/sys/vm/dirty_ratio 
echo 100 > /proc/sys/vm/vfs_cache_pressure
echo "noop" > /sys/block/mmcblk0/queue/scheduler
