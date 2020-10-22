/*
主头文件
*/
#ifndef MAIN_H
#define MAIN_H
#define YYERROR_VERBOSE
#define YYDEBUG 1
//#define LEXDEBUG 词法分析DEBUG开关
extern void outputErrors();
extern int errorCount;
extern int errorBound;

#define CHECK_ERROR_BOUND errorCount++;\
if(errorCount>=errorBound){\
    cout << "There have been more than " << errorBound << " errors, compiler abort." << endl;\
    outputErrors();\
    exit(0);\
}

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

//重新定义属性类型(yylval实际上应由yacc定义)c
class Type{
public:
    string str;//终结符号的具体属性
    string token;//终结符号或非终结符号本身的名称
    int lineNumber;//终结符号的行号，参照语法分析指导.docx
    vector<Type*> children; //对应产生式下面的结点

    Type(){}
    Type(string typ, string name, int ln): str(typ), token(name), lineNumber(ln){}
    Type(string name, vector<Type*> cdn): token(name), children(cdn){}
    };
#define YYSTYPE Type*

#endif //!MAIN_H