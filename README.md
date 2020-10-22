## 项目说明
- 此项目为本人大学期间的编译原理课程设计
- 为PASCAL到C的编译器
- 词法分析使用lex
- 语法分析使用yacc
- 整体工程使用C++，采用cmake构建

## 项目构建方式
- 安装cmake
- 安装MinGW，将mingw32-make.exe复制改名为make.exe
- 在pascal2c-compiler/build目录下依次执行指令
    - cmake -G "MinGW Makefiles" ../src
    - make
    - make install
- 在pascal2c-compiler/build/out/目录下，执行pascal2c.exe
    - 得到CProgram.c文件，该文件是将pascal源代码PascalProgram.pas编译为C代码的结果

## 目录结构
- lex-yacc-src/
    - 本项目的词法分析和语法分析使用lex和yacc
    - 该目录为lex和yacc的源代码目录
    - lex和yacc编译后得到c代码，加入src目录
- src/
    - pascal2c编译器的源代码
- test-cases/
    - 各种测试用例

## pascal2c.exe的使用
| 参数接口 | 参数 | 参数功能 | 参数默认值
| --- | --- | --- | ---
| -inname | [file name] | 指定输入文件名 | PascalProgram.pas
| -outname | [file name] | 指定输出文件名 | CProgram.c
| -compiler | [compiler name] | 指定c编译器名,并将c程序编译成可执行文件 | gcc
| -exename | [exe name] | 指定可执行文件名，自动编译 | CProcess.exe
| -execute | 无 | 自动执行生成的可执行文件，如果未出现-e、-exename参数，则均按照默认方式进行操作 | 
| -errorbound | [n] | 指定错误上限，即编译器发现了指定个数的错误后，立即停止运行 | INF
| -developer | 无 | 输出开发者信息 | 
| -version | 无 | 输出版本信息 | 
| -help | 无 | 输出所有命令行参数的帮助信息 | 

## 编译器功能模块图
![](imgs/模块图.png)

## 主函数流程图
![](imgs/主函数流程图.png)

