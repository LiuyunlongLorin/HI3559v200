#!/bin/sh

top_path=`pwd`
while [ ! -d $top_path/osdrv ];do
  top_path=$top_path/..;
done;
echo `cd $top_path; pwd`
