COMMON = -std=c++14 -W -Wall -O0 $(Mco) 
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

install: libnap uninstall
	mkdir /usr/local/include/libnap
	cp ./src/*.h /usr/local/include/libnap
	cp ./output/libnap.a /usr/local/lib/libnap.a

uninstall:
	rm -rf /usr/local/include/libnap
	rm -f /usr/local/lib/libnap.a
