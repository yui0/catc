#!/bin/sh

./catc test/hello.c
./test/hello
echo -----

./catc test/test01.c
./test/test01
echo -----

./catc test/test02.c
./test/test02
echo -----

rm ./test/*.s
rm ./test/hello
rm ./test/test01
rm ./test/test02
#rm ./test/*.o

