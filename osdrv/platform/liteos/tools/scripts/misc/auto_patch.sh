#!/bin/bash

src=$1
dst=$2

patch_dir=~/temp_patches

rm $patch_dir -rf
mkdir $patch_dir

cd $src

repo status | while read LINE; do
    first=`echo $LINE | awk -F " " '{print $1}'`
    second=`echo $LINE | awk -F " " '{print $2}'`
    if [[ $first == "project" ]]; then
        striped=${second%*/}
        path=${striped#*/}
        reponame=${striped##*/}
        cd $path; git add . -A && git diff --staged > $patch_dir/$reponame; cd $src
    fi
done

cd $dst

repo status | while read LINE; do
    first=`echo $LINE | awk -F " " '{print $1}'`
    second=`echo $LINE | awk -F " " '{print $2}'`
    if [[ $first == "project" ]]; then
        striped=${second%*/}
        path=${striped#*/}
        reponame=${striped##*/}
        if [ -f $patch_dir/$reponame ]; then
            cd $path; git apply $patch_dir/$reponame; cd $dst
        fi
    fi
done
