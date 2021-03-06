#!/bin/bash

. conf.sh

name=fftw

echo "!!!! build $name !!!!"

rm -rf $src_dir || true
mkdir -p $src_dir || true
cd $src_dir
tar xzf $distfiles/$name-*.tar.gz

export CC="gcc -fPIC"

cd $name-*
./configure \
    --prefix=$prefix \
    --enable-float \
#     --enable-openmp

make -j$num_proc
make install

cd $wd
echo "!!!! $name build !!!!"

rm -rf $temp_dir
