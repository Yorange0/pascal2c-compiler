%{
#include "main.h"
#include "yacc.tab.h"
//处理位置信息
int yycolumn = 1;
extern YYLTYPE yylloc;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno - 1; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng;

extern string itos(int num);

string charRec; //保存字符常量
char lineBuffer[10005]; //保存当前行的所有内容
vector<string> lexicalErrorInformation;
void addLexicalErrorInformation(char *word, string info, int l, int r);
bool CheckAndAddLengthTooLargeErrorInformation(char *text, string type, int l, int r);

//函数声明
extern "C"
{
	int yywrap();
	int yylex();
}
%}

%option yylineno

%x SCOM
%X MCOM
%x CH

line \n.*
letter [a-z]
digit [0-9]
blank_chars [ \f\r\t\v]+
identifier {letter}({letter}|{digit})*
_integer {digit}+
floater {digit}+\.{digit}+
_type (integer|real|boolean|char)
relop (>=|>|<=|<>|<)
addop (\+|or)
mulop (\*|\/|div|mod|and)
delimiter (\(|\)|\[|\]|:|,|;|\.)

%%
{blank_chars} {}

{line} {
    if(CheckAndAddLengthTooLargeErrorInformation(yytext, "line", 1, yyleng))
        return 0;
    strcpy(lineBuffer,yytext+1);
    yycolumn=1;
    yyless(1); //yyleng是当前匹配的单词的长度,yyless(n)表示退回yyleng-n个字符，只保留n个字符
}

"program" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="PROGRAM";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return PROGRAM;
}

"const" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="CONST";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return CONST;
}

"var" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="VAR";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return VAR;
}

"array" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="ARRAY";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return ARRAY;
}

"of" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="OF";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return OF;
}

"procedure" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="PROCEDURE";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return PROCEDURE;
}

"function" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="FUNCTION";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return FUNCTION;
}

"begin" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="BEGIN";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return _BEGIN;
}

"end" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="END";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return END;
}

"if" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="IF";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return IF;
}

"then" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="THEN";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return THEN;
}

"for" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="FOR";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return FOR;
}

"to" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="TO";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return TO;
}

"do" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="DO";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return DO;
}

"else" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="ELSE";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return ELSE;
}

"repeat" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="REPEAT";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return REPEAT;
}

"until" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="UNTIL";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return UNTIL;
}

"while" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="WHILE";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "keyword: " << yylval->str << endl;
#endif
	return WHILE;
}

{_type} {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token = "TYPE";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "type: " << yylval->str << endl;
#endif
    return TYPE;
}

"not" {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="NOT";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "NOT: " << yylval->str << endl;
#endif
	return NOT;
}

{relop} {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="RELOP";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "RELOP: " << yylval->str << endl;
#endif
	return RELOP;
}

{addop} {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="ADDOP";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "ADDOP: " << yylval->str << endl;
#endif
	return ADDOP;
}

{mulop} {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="MULOP";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "MULOP: " << yylval->str << endl;
#endif
	return MULOP;
}

"-" {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token = yytext;
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "UMINUS OR SUB: " << yylval->str << endl;
#endif
	return yytext[0];
}

"=" {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token = yytext;
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "EQUAL: " << yylval->str << endl;
#endif
	return yytext[0];
}

{identifier} {
    if(CheckAndAddLengthTooLargeErrorInformation(yytext, "identifier", yycolumn-yyleng, yycolumn-1)){
        yytext[100]=0; //截断处理
        //yycolumn=yycolumn-yyleng;
        //yylloc.first_column=yycolumn;
        //yylloc.last_column=yycolumn+100-1;
        //yycolumn+=100;
        yyleng=100;
    }
    yylval=new Type;
	yylval->str=yytext;
	yylval->token = "IDENTIFIER";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "identifier: " << yylval->str << endl;
#endif
	return IDENTIFIER;
}

{floater} {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="UFNUM";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "floater:" << yylval->str << endl;
#endif
    return UFNUM;
}

{_integer} {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="UINUM";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "integer:" << yylval->str << endl;
#endif
	return UINUM;
}

":=" {
    yylval=new Type;
    yylval->str=yytext;
    yylval->token="ASSIGNOP";
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "ASSIGNOP: " << yylval->str << endl;
#endif
    return ASSIGNOP;
}

"\.\." {
    yylval=new Type;
	yylval->str=yytext;
	yylval->token="RANGEDOT";
	yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
	cout << "RANGEDOT: " << yylval->str << endl;
#endif
	return RANGEDOT;
}

{delimiter} {
    yylval=new Type;
    yylval->str = yytext;
    yylval->token = yytext;
    yylval->lineNumber = yylineno - 1;
#ifdef LEXDEBUG
    cout << "delimiter: " << yylval->str << endl;
#endif
    return yytext[0];
}

"'" {//进入字符常量识别
    BEGIN CH;
	charRec="";
}

"\/\/" {//进入单行注释
    BEGIN SCOM;
}

"\{" {//进入多行注释
	BEGIN MCOM;
}

. {//非法字符 错误3
    addLexicalErrorInformation(yytext, "Invalid character!", yycolumn-yyleng, yycolumn-1);
	//cout << "error: invalid char" << endl;
}

<CH><<EOF>> { //读取字符常量时遇到文件尾 错误4
    addLexicalErrorInformation(yytext, "Unexpected end of file when reading a char constant", yycolumn-yyleng, yycolumn-1);
    return 0;
}

<CH>("'"|"\n") {//字符常量限定在一行中
	int len=charRec.length();
	if(yytext[0]=='\'' && len==0){
        addLexicalErrorInformation(yytext, "Char constant missing!", yycolumn-yyleng-1, yycolumn-1); //错误5
		//cout << "error: missing char" << endl;
        BEGIN INITIAL;
        yylval = new Type;
        yylval->str="\0";
        yylval->token="CHAR";
        yylval->lineNumber=yylineno - 1;
#ifdef LEXDEBUG
		cout << "char: " << yylval->str << endl;
#endif
        return CHAR;
    }
	else if(yytext[0]=='\'' && len==1){
        yylval=new Type;
		yylval->str=charRec[0];//PASCAL似乎不支持转义字符
        yylval->token="CHAR";
        yylval->lineNumber=yylineno - 1;
#ifdef LEXDEBUG
		cout << "char: " << yylval->str << endl;
#endif
        BEGIN INITIAL;
		return CHAR;
	}
	else if(yytext[0]=='\''){
        addLexicalErrorInformation(yytext, "Too many characters in a char constant!", yycolumn-yyleng-len, yycolumn-yyleng-1); //错误6
		//cout << "error: too many char" << endl;
        yylval = new Type;
        yylval->str=charRec[0];
        yylval->token="CHAR";
        yylval->lineNumber=yylineno - 1;
#ifdef LEXDEBUG
		cout << "char: " << yylval->str << endl;
#endif
        BEGIN INITIAL;
        return CHAR;
    }
	else{
        addLexicalErrorInformation(yytext, "Right quote missing!", yycolumn-yyleng-len+1, yycolumn-yyleng-len+1); //错误7
        yyless(0);//将换行符退回
		yylineno--;//行号减一
        //cout << "quote miss match" << endl;
        yylval = new Type;
        if(len==0)
            yylval->str="\0";
        else
            yylval->str=charRec[0];
        yylval->token="CHAR";
        yylval->lineNumber=yylineno - 1;
#ifdef LEXDEBUG
		cout << "char: " << yylval->str << endl;
#endif
        BEGIN INITIAL;
        return CHAR;
    }
}

<CH>. {
	charRec+=yytext[0];
}

<SCOM><<EOF>> { //单行注释遇到文件尾
    return 0;
}

<SCOM>"\n" {
    BEGIN INITIAL;
    yyless(0);//将换行符退回
	yylineno--;//行号减一
}

<SCOM>. {}

<MCOM><<EOF>> { //多行注释遇到文件尾 错误8
    addLexicalErrorInformation(yytext, "Unexpected end of file when reading a multiple line comment, lacking of a right brace", yycolumn-yyleng, yycolumn-1);
    return 0;
}

<MCOM>{line} {
    if(CheckAndAddLengthTooLargeErrorInformation(yytext, "line", 1, yyleng)) //行长度超过限制
        return 0;
    yycolumn=1;
    strcpy(lineBuffer, yytext+1);
    yyless(1);
}

<MCOM>"\}" {
	BEGIN INITIAL;
}

<MCOM>. {}

%%

//这个函数是必须的，不用动
int yywrap()
{
	return 1;
}

void addLexicalErrorInformation(char *word, string info, int l, int r){
    string errorInformation = "[" + info + "] " + itos(yylineno-1) + "." + itos(l) + "-" + itos(yylineno-1) + "." + itos(r) + "\n";
    errorInformation += string(lineBuffer) + "\n";
    for(int i=1;i<=l-1;i++)
        errorInformation+=" ";
    for(int i=l;i<=r;i++)
        errorInformation+="^";
    //cout << errorInformation << endl;
    lexicalErrorInformation.push_back(errorInformation);
}

bool CheckAndAddLengthTooLargeErrorInformation(char *text, string type, int l, int r){
    string errorInformation;
    int len=strlen(text);
    if(type=="line"){
        if(len>10000){ //错误1
            errorInformation = "[Line length too large, exceed 10000] " + itos(yylineno-1) + "." + itos(l) + "-" + itos(yylineno-1) + "." +itos(r); 
            errorInformation += "\nLex analyse abort!";
            //cout << errorInformation << endl;
            lexicalErrorInformation.push_back(errorInformation);
            return true;
        }
        return false;
    }
    else if(type=="identifier"){
        if(len>100){ //错误2
            string id = string(text);
            errorInformation = "[Identifier length too large, exceed 100] " + itos(yylineno-1) + "." + itos(l) + "-" + itos(yylineno-1) + "." + itos(r);
            //cout << errorInformation << endl;
            lexicalErrorInformation.push_back(errorInformation);
            return true;
        }
        return false;
    }
    else{
        cout << "[CheckAndAddLengthTooLargeErrorInformation] type not found" << endl;
        return false;
    }
}