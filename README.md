**⚠️ THIS REPOSITORY HAS BEEN ARCHIVED AND IS NO LONGER MAINTAINED ⚠️**
​    
<pre>
 ___       ___  ________  ________   ________  ________   
|\  \     |\  \|\   __  \|\   ___  \|\   __  \|\   __  \  
\ \  \    \ \  \ \  \|\ /\ \  \\ \  \ \  \|\  \ \  \|\  \ 
 \ \  \    \ \  \ \   __  \ \  \\ \  \ \   __  \ \   ____\
  \ \  \____\ \  \ \  \|\  \ \  \\ \  \ \  \ \  \ \  \___|
   \ \_______\ \__\ \_______\ \__\\ \__\ \__\ \__\ \__\   
    \|_______|\|__|\|_______|\|__| \|__|\|__|\|__|\|__|   

</pre>

LIBNAP
=============

A Cross-Platform **LIB**rary of **N**etwork **AP**plication tools

[![Publish Package](https://github.com/wzhone/Libnap/actions/workflows/publish.yml/badge.svg)](https://github.com/wzhone/Libnap/actions/workflows/publish.yml)

[![Ubuntu](https://github.com/wzhone/Libnap/actions/workflows/ubuntu_build.yml/badge.svg)](https://github.com/wzhone/Libnap/actions/workflows/ubuntu_build.yml)

[![Windows](https://github.com/wzhone/Libnap/actions/workflows/msbuild_build.yml/badge.svg)](https://github.com/wzhone/Libnap/actions/workflows/msbuild_build.yml)

<br/>

------

<h3 align = "center"><a href="./doc/nap.md">LIBNAP Document</a></h3>

------

<br/>

## 通过CMake进行构建

<br/>

### Linux
------
```bash
$ git clone https://github.com/wzhone/libnap.git
$ cd ./libnap
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j `nproc`
$ ./test/test
```

<br/>


### Windows
------
```powershell
$ git clone https://github.com/wzhone/libnap.git
$ mkdir build
$ cd build
$ cmake .. -A x64
$ cmake --build . --config Release
$ .\test\Release\test.exe
```

<br/>

##  平台需求
------

##### Windows
```
Visual Studio 2019
Windows SDK Version >= 10.0
```

##### Linux
```
G++ Version >= 9.3.0
Make Version >= 4.3
CMake version >= 3.1
```

