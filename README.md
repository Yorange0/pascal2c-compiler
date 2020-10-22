## 项目说明
- 此项目为本人大学期间的编译原理课程设计
- 为PASCAL到C的编译器

## 项目构建方式
- 安装cmake
- 安装MinGW，将mingw32-make.exe复制改名为make.exe
- 在pascal2c-compiler/build目录下依次执行指令
    - cmake -G "MinGW Makefiles" ../src
    - make
    - make install
- 在pascal2c-compiler/build/out/目录下，执行pascal2c.exe
    - 得到CProgram.c文件，该文件是将pascal源代码PascalProgram.pas编译为C代码的结果

