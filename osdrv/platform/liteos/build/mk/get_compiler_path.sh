#!/bin/bash
declare TEMP=$1
function get_compiler_path()
{
    local SYSTEM=`uname -s`
    local user_gcc=${TEMP}gcc
    local gcc_install_path=`which ${user_gcc}`
    if [ $SYSTEM = "Linux" ] ; then
        gcc_install_path=${gcc_install_path/\/bin\/${user_gcc}/}
    else
        gcc_install_path=${gcc_install_path/\/bin\/${user_gcc}.exe/}
    fi
    echo $gcc_install_path
}
get_compiler_path

