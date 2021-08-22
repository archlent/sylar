# sylar 一个网络编程框架
学习并适当改造这个[项目](https://github.com/sylar-yin/sylar)
**采用C++17标准**  

## Introduction  
* 日志  
支持流式日志(`SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << "this is a log"`)和格式化日志(`SYLAR_LOG_INFO(SYLAR_LOG_ROOT, %s, "this is a log")`)，日志内容支持时间，线程ID，日志级别，文件名，行号，协程ID等内容自由配置

* 配置  
## 编译  
* 编译环境  
*ArchLinux*    
*gcc version 11.1.0   (最低 gcc 7.1)*  
*cmake version 3.21.1 (最低 cmake 3.8)*  
*boost version 1.76*

* 编译依赖  
依赖于boost库和yaml-cpp库  
1. boost库安装 :  
    **Ubuntu**    `sudo apt install libboost-all-dev`(version 1.71)  
    **ArchLinux** `sudo pacman -S boost`  
2. yaml-cpp安装(可不安装到全局) :  
    `git clone https://github.com/jbeder/yaml-cpp.git`  
    `mkdir build`  
    `cd build`  
    `cmake -DBUILD_SHARED_LIBS=ON ..`  //动态库, 默认为静态库  
    `make install`  
     **ArchLinux** `sudo pacman -S yaml-cpp`  
3. ragel安装：    
   **Ubuntu**    `sudo apt install ragel`  
   **ArchLinux** `sudo pacman -S ragel`

* 编译安装  
```
git clone https://github.com/4kangjc/sylar.git
cd sylar
mkdir include && cd include
git clone https://github.com/jbeder/yaml-cpp.git  // 将yaml-cpp clone 到 include目录, 视自己情况而定  
cd ..
mkdir build && cd build && cmake ..
make
```    

## 技术要点  
