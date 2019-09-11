#!/bin/bash

if [ $# -eq 0 ];then
echo "##############################################################"
echo "Example: ./make_version 1"
echo "If the input parameter is 1, the version is used for self-test."
echo "Other numbers indicate that the version is used for test."
echo "##############################################################"
exit
fi

rm -rf 3rdParty
rm -rf vendor

sed -i '/vendor\/Kconfig"/d' tools/menuconfig/extra/Configs/Config.in
sed -i '/3rdParty\/Kconfig"/d' tools/menuconfig/extra/Configs/Config.in

if [ $1 -ne 1 ];then
rm -rf test
sed -i '/test\/Kconfig"/d' tools/menuconfig/extra/Configs/Config.in
fi

find . -name *.git | xargs rm -rf

echo "make version for test done"
