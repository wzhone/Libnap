COMMON = -std=c++11 -W -Wall -O0 $(Mco) 
LINKLIB = -lpthread -L/usr/local/lib
export COMMON
export LINKLIB


all: checkdir libnap test
	./output/test

build: checkdir test libnap

test: checkdir
	make -C test

libnap: checkdir
	make -C src

clean:
	rm -rf ./output
	make -C src -s clean
	make -C test -s clean

checkdir: 
	mkdir -p ./output

fast: 
	make -j -s build

# install:
# 	make -C src install

# uninstall:
# 	make -C src install
