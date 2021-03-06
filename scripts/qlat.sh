#!/bin/bash

. conf.sh

name=qlat
echo "!!!! build $name !!!!"

rm -rfv $prefix/include/qutils
rm -rfv $prefix/include/qlat
rm -rfv $prefix/pylib/qlat
rm -rfv $prefix/pylib/rbc_ukqcd_params
mkdir -pv $prefix/include/qutils
mkdir -pv $prefix/include/qlat
mkdir -pv $prefix/pylib/qlat
cp -pv qutils/*.h $prefix/include/qutils
cp -pv qlat/*.h $prefix/include/qlat
cp -pv qlat-setup.h $prefix/include
cp -pv pylib/qlat/* $prefix/pylib/qlat
cp -rpv pylib/rbc_ukqcd_params $prefix/pylib/

cp -pv pylib/*.py $prefix/pylib/

( cd ./pylib/cqlat ; ./update.sh )

rm -rfv $prefix/pylib/cqlat
mkdir -pv $prefix/pylib
cp -rpv pylib/cqlat $prefix/pylib/

time make -C $prefix/pylib/cqlat -j $num_proc qlat_prefix="$(readlink -m "$prefix")"

echo "!!!! $name build !!!!"

rm -rf $temp_dir
