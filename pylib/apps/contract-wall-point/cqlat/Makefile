SHELL=/bin/bash

CXX=mpic++ -std=c++11 -fopenmp -O2 -Wall -fno-strict-aliasing

qlat_prefix="$(shell readlink -m "$$HOME/qlat-build/default")"

QLAT_INCLUDE=$(qlat_prefix)/include
QLAT_LIB=$(qlat_prefix)/lib

QLAT_CFLAGS=
QLAT_CFLAGS+= -I$(QLAT_INCLUDE)
QLAT_CFLAGS+= -I$(QLAT_INCLUDE)/eigen3
QLAT_CFLAGS+= -I$(QLAT_INCLUDE)/qutils

QLAT_CXXFLAGS=$(QLAT_CFLAGS)

QLAT_LDFLAGS=
QLAT_LDFLAGS+= -L$(QLAT_LIB)
QLAT_LDFLAGS+= -l:libfftw3.a -l:libfftw3f.a
QLAT_LDFLAGS+= -l:libcuba.a
QLAT_LDFLAGS+= -l:libz.a
QLAT_LDFLAGS+= -lm

CXXFLAGS=
CXXFLAGS+= $(QLAT_CXXFLAGS)
CXXFLAGS+= $$(python3-config --includes)
# CXXFLAGS+= $$(python3 -c "import numpy; print(numpy.get_include())") 

LDFLAGS=
LDFLAGS+= $(QLAT_LDFLAGS)

NAME=cqlat.so

OBJS:=$(patsubst %.C, %.o, $(wildcard *.C))

all: build

build: $(OBJS) lib.o
	$(CXX) --shared -fPIC -o $(NAME) *.o base/*.o $(LDFLAGS)
	ls -lh $(NAME)
	mv $(NAME) ..

exports.h: *.C base/exports.h
	./base/update.sh base/exports.h

base/exports.h:
	rsync -av --delete $(qlat_prefix)/pylib/cqlat/ base/

lib.C: base/exports.h
	rm -v base/lib.o
	cp -v base/lib.C .

%.o : %.C exports.h *.h
	$(CXX) -fPIC -c -o $@ $< $(CXXFLAGS) -Ibase
	@echo "$@" "$<"

clean:
	-rm -rfv *.o lib.C exports.h base
