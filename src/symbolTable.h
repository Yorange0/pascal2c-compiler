/*
符号表头文件
*/
//通过了语法分析，则一定不存在函数的嵌套定义，而且子符号表只比主符号表少用了一些字段，因此主符号表和子符号表可以用同一个结构
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <string>
#include <vector>
#include <map>
using namespace std;

class _SymbolTable;

class _SymbolRecord {
public:
	string flag;//"value parameter"表示传值参数,"var parameter"表示传引用参数,"normal variant"表示普通变量,"constant"表示常量,
	//"array"表示数组,"procedure"表示过程,"function"表示函数
	//"(sub)program name"表示该条记录是当前符号表对应的程序名信息
	//"parameter of program"表示主程序的参数
	string id;
	int lineNumber;//定义位置的行号
	string type;//如果是变量/常量，则表示变量/常量类型；
	//如果是数组，则表示数组元素的类型；
	//如果是函数，则表示函数返回值类型，类型本身只能为基本类型,"integer","real","char","boolean"
	string value;//如果是常量，则表示常量取值
	bool isMinusShow;//常量前是否带负号
	//所以取值有"integer","real","char","boolean"五种
	int amount;//数组维数/参数个数。
	//如果是数组，则表示数组维数，如果是函数/过程，则表示参数个数
	vector< pair<int,int> > arrayRangeList;//数组各维上下界
	_SymbolTable* subSymbolTable;//指向过程/函数对应的字符号表的指针

	string subprogramType;//这是一条特殊的记录，表示当前符号表对应的程序名称等信息，该变量表示程序是函数还是过程

	void setPara(string id, int lineNumber, string type);
	void setVarPara(string id, int lineNumber, string type);
	void setVar(string id, int lineNumber, string type);
	void setConst(string id, int lineNumber, string type, bool isMinusShow, string value);
	void setArray(string id, int lineNumber, string type, int amount, vector< pair<int, int> > arrayRangeList);
	void setProcedure(string id, int lineNumber, int amount, _SymbolTable *subSymbolTable);
	void setFunction(string id, int lineNumber, string type, int amount, _SymbolTable *subSymbolTable);
	void setProgramName(string id, int lineNumber, string subprogramType, int amount, string returnType);
	void setVoidPara(string id, int lineNumber);

	string findXthFormalParaType(int X);//找到第X个形式参数的类型
	bool isXthFormalParaRefered(int X);//检查第X个形式参数是否是引用调用
	bool checkArrayXthIndexRange(int X,int index);//检查第X维下标是否越界，true表示越界，false表示未越界

	_SymbolRecord(){
		arrayRangeList.clear();
		subSymbolTable = NULL;
	}
	~_SymbolRecord(){}
};

class _SymbolTable{
public:
	string tableType;//"main"或者"sub"

	vector<_SymbolRecord*> recordList;
	map<string,int> idToLoc;//加快查询速度的map
	map<string, int> idCount;//标识符定义的次数-1，用于恢复程序参数重定义的错误
	//每加入一个符号，都要记录到这个map中
	//每查询一个符号，都可以通过这个map查询该符号在vector中的位置
	
	void addPara(string id, int lineNumber, string type);
	void addVarPara(string id, int lineNumber, string type);
	void addVar(string id, int lineNumber, string type);
	void addConst(string id, int lineNumber, string type, bool isMinusShow, string value);
	void addArray(string id, int lineNumber, string type, int amount, vector< pair<int, int> > arrayRangeList);
	void addProcedure(string id, int lineNumber, int amount, _SymbolTable *subSymbolTable=NULL);
	void addFunction(string id, int lineNumber, string type, int amount, _SymbolTable *subSymbolTable=NULL);
	void addSubSymbolTable(string id, _SymbolTable *subSymbolTable);
	void addProgramName(string id, int lineNumber, string subprogramType, int amount, string returnType);
	void addVoidPara(string id, int lineNumber);

	_SymbolTable(string type = "sub");
	~_SymbolTable(){}
};

#endif //!SYMBOLTABLE_H
