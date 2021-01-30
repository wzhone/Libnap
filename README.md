LIBNAP
=============

A Cross-Platform **LIB**rary of **N**etwork **AP**plication tools

![ubuntu](https://github.com/wzhone/libnap/workflows/UBUNTU/badge.svg)
![Windows](https://github.com/wzhone/libnap/workflows/Windows/badge.svg)

<br/>

------

<h3 align = "center"><a href="./doc/nap.md">LIBNAP Document</a></h3>

------

<br/>

## 1. BUILD

<br/>

### Make (Linux)
------
```bash
$ git clone https://github.com/staticn/libnap.git
$ cd ./libnap
$ make
$ make install
```

<br/>


### Visual Studio (Windows)
------
```powershell
$ git clone https://github.com/staticn/libnap.git
$ cd ./libnap/win
$ ./libnap.sln
```

<br/>
<br/>

## 2. INSTALLATION

编译成功后，在output文件夹下，会有需要的静态库文件和头文件。

<br/>

## 3. Other content 

<br/>

 **在Linux中，可以使用以下make命令根据不同需求进行编译**

------
```bash
make / make all : 编译libnap 编译测试文件 并运行测试
make build : 编译libnap 编译测试文件
make test : 编译测试文件
make libnap : 编译libnap
make clean : 清除编译的文件
make fast : 启用多核快速安静编译
```

<br/>

##  4. Platform requirements 
------

##### Windows
```
Visual Studio 2019
Windows SDK Version >= 10.0
```

##### Linux
```
G++ Version >= 4.8.5
Make Version >= 3.8
```


