##get warning
#sed -i '/[0-9]: warning:/!d' tmp
##the number of leaving directory

#cd $LITEOSTOPDIR
echo "calc warnings,please wait for a moment"
make -j16 >tmp2 2>&1
sum1=`grep -o "warning" <tmp2 | wc -l`
#echo "the total $sum1(times)"
sum2=0;
res=`grep -0 "Leaving directory"  <tmp2|wc -l`
#echo "res= $res"
for((i=1;i<=$res;i++));do
str=`grep -n "Entering directory"  tmp2 | head -$i |tail -1`
str=${str#*Huawei_LiteOS/}
str=`echo ${str%%\'*}`
a=`grep -n "Entering directory"  tmp2 | head -$i |tail -1 | cut -d ":" -f 1`
b=`grep -n "Leaving directory"  tmp2 | head -$i |tail -1| cut -d ":" -f 1`
#echo "a=$a  b=$b"
c=`sed -n ''${a}','${b}'p'  tmp2| grep -o "warning:" |wc -l`
#echo "c= $c"
if [ $c -ne 0 ];then
  echo "$str $c(warnings)"
  sum2=`expr $sum2 + $c`
fi
done
rm tmp2
echo "total warnings:$sum2"

