#!/bin/bash
#this scrip will delete all directories in src_array, and modify Confin.in settings in menuconfig accordingly.
#All .git files and directories will be deleted too.
#Please run this scrip under Huawei_LiteOS dir.

LITEOSTOPDIR=`pwd`
cd ${LITEOSTOPDIR}

src_array=(
vendor/
test/
3rdPatry/
drivers/usb/
fs/fat/
fs/jffs2/
fs/yaffs2/
)

echo -e "This scrip are going to delete flowing stuff:"
echo "1), Dirs:"
for dir in ${src_array[@]}
do
    echo -e "    ${dir}"
done

echo -e "\n2), all .git files\n"
echo -e "And will modify Config.in in menuconfig accordingly.\n"
read -n1 -p "Are you sure to execute? (Y or y to continue, others to abort)" ans

if [ "$ans" != "Y" ] && [ "$ans" != "y" ]
then
    echo -e "\naborted"
    exit
fi

echo -e "Deleting dirs ..."
for dir in ${src_array[@]}
do
    rm -rf ${dir}
done

echo "Modifying Config.in for menuconfig ..."
for dir in ${src_array[@]}
do
    #从原字符中的“/"增加转义字符,给sed处理
    tmp=${dir//\//\\\/}
    tmp=${tmp}"Kconfig"
    #echo $tmp
    sed -i '/'"$tmp"'/d' tools/menuconfig/extra/Configs/Config.in
done

echo -e "Deleting .git files ..."
find  ./ -name ".git" |xargs rm -rf

cd -
