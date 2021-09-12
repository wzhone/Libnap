mkdir -p ./output
make -C src
make[1]: 进入目录“/home/wzh/桌面/Libnap/src”
g++ -c aes.cpp  -o aes.o -std=c++14 -W -Wall -O0  
g++ -c trans.cpp  -o trans.o -std=c++14 -W -Wall -O0  
g++ -c nap_common.cpp  -o nap_common.o -std=c++14 -W -Wall -O0  
g++ -c hash.cpp  -o hash.o -std=c++14 -W -Wall -O0  
g++ -c threadpool.cpp  -o threadpool.o -std=c++14 -W -Wall -O0  
g++ -c btring.cpp  -o btring.o -std=c++14 -W -Wall -O0  
g++ -c net.cpp  -o net.o -std=c++14 -W -Wall -O0  
g++ -c json.cpp  -o json.o -std=c++14 -W -Wall -O0  
ar rcs ../output/libnap.a  ./aes.o ./trans.o ./nap_common.o ./hash.o ./threadpool.o ./btring.o ./net.o ./json.o 
mv ./*.o ../output/
make[1]: 离开目录“/home/wzh/桌面/Libnap/src”
make -C test
make[1]: 进入目录“/home/wzh/桌面/Libnap/test”
g++ -o ../output/test -std=c++14 -W -Wall -O0   -I../src/  ./naptest.o ./test.o   -lpthread -L/usr/local/lib -L../output/ -lnap -g
make[1]: 离开目录“/home/wzh/桌面/Libnap/test”
