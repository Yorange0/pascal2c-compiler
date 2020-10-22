/*
抽象语法树各节点类的定义
*/

#ifndef ASTNODES_H
#define ASTNODES_H

#include <vector>
#include <string>
using namespace std;

class _Expression;
class _FunctionCall;
class _VariantReference;
class _Statement;
class _Compound;
class _RepeatStatement;
class _WhileStatement;
class _ForStatement;
class _IfStatement;
class _AssignStatement;
class _ProcedureCall;
class _Type;
class _FormalParameter;
class _FunctionDefinition;
class _Variant;
class _Constant;
class _SubProgram;
class _Program;

class _Expression
{
    public:
        string type;//表达式类型,"var"表示变量,"integer"表示整数,"real"表示浮点数,"char"表示常量字符
        //"function"表示函数调用,"compound"表示复合表达式,
        //compound有普通的二目运算符，还有"minus"、"not"、"bracket"等单目运算符
        
        _VariantReference* variantReference;//变量或常量或数组

        int intNum;//整数
        
        float realNum;//浮点数

        string strOfNum;//整数和浮点数的string表示（考虑从PASCAL-S源程序将字符串转为浮点数，再将浮点数转为字符串会带来精度问题，所以需要存下初始字符串值）

        char charVal;//常量字符

        _FunctionCall *functionCall;//函数调用
        
        string operation;//具体操作符
        string operationType;//操作符类型,"relop","mulop","addop","single"
        _Expression *operand1,*operand2;

        int lineNumber;//行号, 用表达式中最先出现的操作数的行号表示
    public:
        _Expression();
		~_Expression();
    //语义分析相关
    public:
        int totalIntValue;
        bool totalIntValueValid;
        string expressionType;//区别于type，这个值表示表达式的具体类型，即"integer"、"real"、"char"、"boolean"、"error"，其中error表示表达式中包含类型不一致的操作数
};

class _FunctionCall
{
    public:
        pair<string,int> functionId;//函数标识符
        vector<_Expression*> actualParaList;//实际参数列表，由表达式组成
    public:
        _FunctionCall();
        ~_FunctionCall();
    public:
        string returnType;//"integer"、"real"、"char"、"boolean"、"error"，其中error表示函数标识符不存在
};

class _VariantReference
{
    public:
        pair<string,int> variantId;//变量或常量标识符和行号
        int flag;//0表示非数组,1表示数组
        vector<_Expression*> expressionList;//各维引用表达式列表
    public:
        _VariantReference();
        ~_VariantReference();
    public:
		int locFlag;//-1表示左值，1表示右值，0表示什么都不是 左值特判
		string kind;//"array","var","constant","function call","function return reference"
        string variantType;//"integer"、"real"、"char"、"boolean"、"error"，其中"error"表示数组某一维下标表达式的类型不为"integer"或标识符不存在
};

class _Statement
{
    public:
        string type;//"compound","repeat","while","for","if","assign","procedure"
		string statementType;//区别于type，取值为"void"或"error"
		int lineNumber;//行号
		bool isReturnStatement;//是否是返回值语句
    public:
        _Statement(){}
        ~_Statement(){}
};

class _Compound:public _Statement
{
    public:
        vector<_Statement*> statementList;//语句列表
		//行号由begin的位置决定
    public:
        _Compound();
        ~_Compound();
};

class _RepeatStatement:public _Statement
{
    public:
        _Expression *condition;//条件表达式
        _Statement *_do;//循环体语句
		//行号由repeat的位置决定
    public:
        _RepeatStatement();
        ~_RepeatStatement();
};

class _WhileStatement:public _Statement
{
    public:
        _Expression *condition;//条件表达式
        _Statement *_do;//循环体语句
		//行号由while的位置决定
    public:
        _WhileStatement();
        ~_WhileStatement();
};

class _ForStatement:public _Statement
{
    public:
        pair<string,int> id;//循环变量
        _Expression *start;//起始值
        _Expression *end;//终止值
        _Statement *_do;//循环体语句
		//行号由for的位置决定
    public:
        _ForStatement();
        ~_ForStatement();
};

class _IfStatement:public _Statement
{
    public:
        _Expression *condition;//条件表达式
        _Statement *then;//满足条件时执行的语句
        _Statement *els;//不满足条件时执行的语句，如果为NULL，则没有else部分
		//行号由if的位置决定
    public:
        _IfStatement();
        ~_IfStatement();
};

class _AssignStatement:public _Statement
{
    public:
        _VariantReference* variantReference;//左值变量
        _Expression* expression;//右值表达式
		//行号由赋值符号的位置决定
    public:
        _AssignStatement();
        ~_AssignStatement();
};

class _ProcedureCall:public _Statement
{
    public:
        pair<string,int> procedureId;//过程标识符
        vector<_Expression*> actualParaList;//实际参数列表，由表达式组成
		//行号由procedure名的位置决定
    public:
        _ProcedureCall();
        ~_ProcedureCall();
};

class _Type//类型
{
    public:
        pair<string,int> type;//基本类型及行号 "integer"、"char"、"real"、"boolean" 
        int flag;//0表示非数组，1表示数组
        vector< pair<int,int> > arrayRangeList;//flag=1时，表示数组各维上下界
    public:
        _Type();
        _Type(pair<string,int> _type,int _flag,vector< pair<int,int> > _arrayRangeList);
        ~_Type(){}
};

class _FormalParameter//形式参数
{
    public:
        pair<string,int> paraId;//形式参数标识符和行号
        string type;//形式参数类型，形式参数一定是基本类型，所以改为string
        int flag;//flag=0表示传值调用，flag=1表示引用调用
    public:
        _FormalParameter();
        _FormalParameter(pair<string,int> _paraId,string _type,int _flag);
        ~_FormalParameter(){}
};

class _FunctionDefinition
{//函数/过程定义
    public:
        pair<string,int> functionID;//函数/过程标识符及行号
        vector<_FormalParameter*> formalParaList;//形式参数列表
        pair<string,int> type;//如果type.first是空串，则为过程，否则为函数,取值为"integer","real","boolean","char"四种
        vector<_Constant*> constList;//常数定义列表
        vector<_Variant*> variantList;//变量定义列表
        _Compound* compound;
    public:
        _FunctionDefinition();
        ~_FunctionDefinition();
};

class _Variant//变量定义
{
    public:
        pair<string,int> variantId;//变量标识符ID及行号
        _Type *type;//变量类型
    public:
        _Variant();
		_Variant(pair<string,int> _variantId,_Type *_type);
        ~_Variant();
};

class _Constant//常量定义
{//常量定义的时候，右值可以是已经定义好的常量标识符
//在代码生成的时候，可以根据常量的范围对类型进行进一步的细化
    public:
        pair<string,int> constId;
        string type;//常数类型,分为"id","integer","real","char"

        pair<string,int> valueId;
        char charValue;
		int intValue;
        float realValue;

		string strOfVal;//所有常量取值的字符串表示
		bool isMinusShow;
    public:
        _Constant(){}
        ~_Constant(){}
};

class _SubProgram//分程序
{
    public:
        vector<_Constant*> constList;//常数定义列表
        vector<_Variant*> variantList;//变量定义列表
        vector<_FunctionDefinition*> subprogramDefinitionList;//子程序和子函数定义列表
        _Compound *compound;//主程序体
    public:
        _SubProgram();
        ~_SubProgram();
};

class _Program//程序
{
    public:
        pair<string,int> programId;//PASCAL程序名称标识符及行号
        vector< pair<string,int> > paraList;//PASCAL程序参数列表及行号
        _SubProgram* subProgram;//分程序
	public:
		_Program();
		~_Program();
};

#endif // !ASTNODES_H


