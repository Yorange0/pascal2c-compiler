/*
代码生成实现
*/
//函数和过程中需要特判进行转换
#include "symbolTable.h"
#include "ASTnodes.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
using namespace std;

extern string itos(int num);

extern _SymbolTable *mainSymbolTable;
_SymbolTable* codeGenerateCurrentSymbolTable;
extern _SymbolRecord* findSymbolRecord(_SymbolTable* currentSymbolTable, string id, int mode = 0);//从符号表中找出id对应的记录
extern bool checkIsReferedPara(_SymbolTable* currentSymbolTable, string id);

map<string,string> mp_subprogramToHeadFile;//过程、函数到C程序头文件的映射  
map<string,bool> mp_headFileShow;//头文件是否出现的映射
//全局常量
vector<string> v_s_constIdList;//常量标识符  
vector<string> v_s_constTypeList;//常量类型,"integer","real","char","boolean"  
vector<string> v_s_constValueList;//常量值,不论是哪种类型，均用string表示
//全局变量
vector<string> v_s_variantIdList;//变量标识符  
vector<string> v_s_variantTypeList;//变量类型,"integer","real","char","boolean"  
vector< vector<int> > v_ArraySizeList;//各数组变量各维的大小

//子程序接口声明
struct subproDec{
	string returnType;//子程序返回值类型,"integer","real","char","boolean"
	string id;//子程序名称标识符
	vector<string> v_paraIdList;//参数标识符
	vector<bool> v_isParaRef;//参数是否为引用
	vector<string> v_paraTypeList;//参数类型,"integer","real","char","boolean"
	subproDec(){
		v_paraIdList.clear();
		v_isParaRef.clear();
		v_paraTypeList.clear();
	}
	void clear() {
		v_paraIdList.clear();
		v_isParaRef.clear();
		v_paraTypeList.clear();
	}
};
vector< struct subproDec > v_subproDecList;//子程序接口列表


//子程序定义
struct subproDef{
	vector<string> constIdList;//常量标识符  
	vector<string> constTypeList;//常量类型,"integer","real","char","boolean"  
	vector<string> constValueList;//常量值,不论是哪种类型，均用string表示

	vector<string> variantIdList;//变量标识符  
	vector<string> variantTypeList;//变量类型,"integer","real","char","boolean"  
	vector< vector<int> > arraySizeList;//各数组变量各维的大小
	
	vector< pair<string,int> > statementList;//pair的first是语句本身，pair的second控制缩进，即语句前的制表符个数
};
//子程序定义列表
vector< struct subproDef > subproDefList;

string subMainFunctionDeclaration;//原PASCAL主程序对应到C程序的声明
vector< pair<string, int> > v_statementList;//主程序语句列表

void codeGenerate(_Program *ASTRoot, string outName);//代码生成
void beforeCodeGenerate();//在代码生成之前需要进行的初始化工作
bool isEqual(int num,...);//判断num个整数是否相等
string transformType(string pascalType);//将pascal的type转化为c的type
string transformOpertion(string operation, int mode=0);//将pascal的operation转化为c的operation

void inputHeadFileList(string subProgramId);//获取头文件
void inputConstList(vector<_Constant *> &constList, vector<string> &constIdList, vector<string> &constTypeList, vector<string> &constValueList, _SymbolTable* symbolTable);//获取常数列表
void inputVariantList(vector<_Variant *> variantList, vector<string> &variantIdList, vector<string> &variantTypeList, vector< vector<int> > &arraySizeList, _SymbolTable* symbolTable);//获取变量列表
void inputSubproDecList();//获取子程序声明列表
void inputFunctionCall(_FunctionCall *functionCallNode, string &functionCall, int mode=0);//获取函数调用
int inputExpression(_Expression *expressionNode, string &expression, int mode=0, bool isReferedActualPara=false);//获取表达式
void inputVariantRef(_VariantReference *variantRefNode, string &variantRef, int mode=0, bool isReferedActualPara=false);//获取变量引用
void inputStatementList(_Statement *statementNode, vector< pair<string,int> > &v_statementList, int retract, int flag=0);//获取语句列表
void inputSubproDef(_FunctionDefinition* functionDefinitionNode);//获取子程序定义
void inputSubproDefList(_SubProgram* subProgramNode);//获取子程序定义列表
void inputSubMainFunction(_Program* ASTRoot);//获取原PASCAL主程序头对应到的C程序头，以及主程序体
string getOutputFormat(string type);//根据类型获取输入输出格式控制符
bool checkAndInputLibrarySubprogram(_ProcedureCall* procedureCall, vector< pair<string, int> > &v_statementList, int retract); //检查并获取库程序

void outputHeadFileList();//输出头文件
void outputConstList(vector<string> &constIdList, vector<string> &constTypeList, vector<string> &constValueList, int retract = 0);//输出常数列表
void outputVariantList(vector<string> &variantIdList, vector<string> &variantTypeList, vector< vector<int> > &arraySizeList, int retract = 0);//输出变量列表
void outputSubproDec(subproDec &tmp, int flag = 0);//输出子程序声明
void outputSubproDecList();//输出子程序声明列表
void outputStatement(pair<string, int> &tmp);//输出语句
void outputStatementList(vector< pair<string, int> > &vec);//输出语句列表
void outputSubproDefList();//输出子程序定义列表
void outputMain(_Program* ASTRoot);//输出主函数

ofstream fout;

void codeGenerate(_Program *ASTRoot, string outName) {
	fout.open(outName.c_str());
	codeGenerateCurrentSymbolTable = mainSymbolTable;//定位到主符号表
	beforeCodeGenerate();
	inputSubproDecList();//子程序声明
	inputConstList(ASTRoot->subProgram->constList, v_s_constIdList, v_s_constTypeList, v_s_constValueList, codeGenerateCurrentSymbolTable);//全局常量/主程序常量
	inputVariantList(ASTRoot->subProgram->variantList, v_s_variantIdList, v_s_variantTypeList, v_ArraySizeList, codeGenerateCurrentSymbolTable);//全局变量/主程序变量
	inputSubproDefList(ASTRoot->subProgram);//子程序定义列表
	codeGenerateCurrentSymbolTable = mainSymbolTable;//定位到主符号表
	inputSubMainFunction(ASTRoot);//原PASCAL主程序对应C的程序头和语句体

	fout << "//Head files" << endl;
	outputHeadFileList();//输出头文件
	fout << endl;
	
	fout << "//Overall constant definiton" << endl;
	outputConstList(v_s_constIdList, v_s_constTypeList, v_s_constValueList);//输出全局常量
	fout << endl;

	fout << "//Overall variable definition" << endl;
	outputVariantList(v_s_variantIdList, v_s_variantTypeList, v_ArraySizeList);//输出全局变量
	fout << endl;
	
	fout << "//Subprogram declaration" << endl;
	outputSubproDecList();//输出子程序声明
	fout << endl;

	fout << "//Main function" << endl;
	outputMain(ASTRoot);//输出主函数
	fout << endl;

	fout << "//Subprogram definition" << endl;
	outputSubproDefList();
	
	fout.close();
}

void beforeCodeGenerate(){
	codeGenerateCurrentSymbolTable = mainSymbolTable;
	mp_subprogramToHeadFile["read"]="stdio.h";
	mp_subprogramToHeadFile["write"]="stdio.h";
	mp_subprogramToHeadFile["writeln"]="stdio.h";
	mp_headFileShow["stdio.h"]=false;
}

bool isEqual(int num,...){
    va_list argp;
    int para;
    va_start(argp, num);
    int pre=va_arg(argp, int);
    for(int argno=1; argno<num; argno++){
        para = va_arg(argp, int);
        if(para!=pre)
            return false;
    }
    va_end(argp);
    return true;
}

string transformType(string pascalType) {
	if (pascalType == "integer")
		return "int";
	if (pascalType == "real")
		return "float";
	if (pascalType == "boolean")
		return "bool";
	if (pascalType == "char")
		return "char";
	if (pascalType == "")
		return "";
	cout << "[transformType] pascalType error" << endl;
	return "";
}

string transformOpertion(string operation,int mode) {
	if (mode != 0)
		return operation;
	if (operation == "not")
		return "!";
	if (operation == "=")
		return "==";
	if (operation == "<>")
		return "!=";
	if (operation == "or")
		return "||";
	if (operation == "div")
		return "/";
	if (operation == "mod")
		return "%";
	if (operation == "and")
		return "&&";
	return operation;
}

void inputHeadFileList(string subProgramId){
	if (mp_subprogramToHeadFile.count(subProgramId)) 
		mp_headFileShow[mp_subprogramToHeadFile[subProgramId]] = true;
}

void inputConstList(vector<_Constant *> &constList, vector<string> &constIdList, vector<string> &constTypeList, vector<string> &constValueList, _SymbolTable* symbolTable){
	//初始化
	constIdList.clear();
	constTypeList.clear();
	constValueList.clear();
	for(int i=0;i<constList.size();i++){
		if(constList[i]==NULL){
			cout << "[inputConstList] ERROR: constant pointer is null" << endl;
			return;
		}
		constIdList.push_back(constList[i]->constId.first);
		_SymbolRecord* record = findSymbolRecord(symbolTable, constList[i]->constId.first, 1);
		//if (record == NULL || record->flag!="constant") {
			//cout << "[inputConstList] constant record not found" << endl;
			//return;
		//}
		string constVal;
		if (record->type != "char" && record->isMinusShow)
			constVal += "-";
		constVal += record->value;
		constTypeList.push_back(transformType(record->type));//从符号表获取常量类型
		constValueList.push_back(constVal);//从符号表获取常量值，要求是string形式
	}
}

void inputVariantList(vector<_Variant *> variantList, vector<string> &variantIdList, vector<string> &variantTypeList, vector< vector<int> > &arraySizeList, _SymbolTable* symbolTable){
	//初始化
	variantIdList.clear();
	variantTypeList.clear();
	arraySizeList.clear();
	for(int i=0;i<variantList.size();i++){
		if(variantList[i]==NULL){
			cout << "[inputVariantList] ERROR: variant pointer is null" << endl;
			return;
		}
		variantIdList.push_back(variantList[i]->variantId.first);
		_SymbolRecord* record = findSymbolRecord(symbolTable, variantList[i]->variantId.first, 1);
		//if (record == NULL || (record->flag != "array" && record->flag != "normal variant")) {
			//cout << "[inputVariantList] variant record not found" << endl;
			//return;
		//}
		variantTypeList.push_back(transformType(record->type));//从符号表获取类型
		vector< pair<int, int> > &tmpRangeList = record->arrayRangeList;
		vector<int> tmpRange;
		for (int j = 0; j < tmpRangeList.size(); j++)
			tmpRange.push_back(tmpRangeList[j].second - tmpRangeList[j].first + 1);
		arraySizeList.push_back(tmpRange);
	}
}

void inputSubproDecList(){
	subproDec dec;
	for(int i=0;i<mainSymbolTable->recordList.size();i++){
		_SymbolRecord* record=mainSymbolTable->recordList[i];
		if((record->flag=="procedure" || record->flag=="function") && record->subSymbolTable != NULL){
			dec.clear();
			dec.returnType=transformType(record->type);
			dec.id=record->id;
			_SymbolTable *subTable = record->subSymbolTable;
			for(int i=1;i<=record->amount;i++){
				dec.v_paraIdList.push_back(subTable->recordList[i]->id);
				if(subTable->recordList[i]->flag=="var parameter")
					dec.v_isParaRef.push_back(true);
				else
					dec.v_isParaRef.push_back(false);
				dec.v_paraTypeList.push_back(transformType(subTable->recordList[i]->type));
			}
			v_subproDecList.push_back(dec);
		}
	}
}

//mode表示是否需要将PASCAL运算符转化为C运算符
void inputFunctionCall(_FunctionCall *functionCallNode, string &functionCall, int mode){
	if(functionCallNode==NULL){
		cout << "[inputFunctionCall] ERROR: functionCall pointer is null" << endl;
		return;
	}
	inputHeadFileList(functionCallNode->functionId.first);
	if(functionCallNode->actualParaList.size()) //hhh 这里有点奇怪 好像不需要这个判断?
		functionCall=functionCallNode->functionId.first+"(";
	_SymbolRecord *record = findSymbolRecord(mainSymbolTable, functionCallNode->functionId.first);
	//if (record == NULL || record->flag!="function") {
		//cout << "[inputFunctionCall] function not found" << endl;
		//return;
	//}
	for(int i=0;i<functionCallNode->actualParaList.size();i++){
		if(i!=0)
			functionCall+=", ";
		string expression; //需要检查对应的形参是否是引用参数
		inputExpression(functionCallNode->actualParaList[i],expression, mode, record->isXthFormalParaRefered(i + 1));
		functionCall+=expression;
	}
	if (functionCallNode->actualParaList.size())
		functionCall+=")";
}

//mode表示是否需要将PASCAL运算符转化为C运算符
int inputExpression(_Expression *expressionNode, string &expression, int mode, bool isReferedActualPara){
	//返回值用于表示是否需要加括号
	int flag;
	if(expressionNode==NULL){
		cout << "[inputExpression] ERROR: expression pointer is null" << endl;
		return -1; 
	}
	if(expressionNode->type=="var"){//如果是变量标识符
		string variantRef;//这里应该一定是右值，所以无需特判
		inputVariantRef(expressionNode->variantReference, variantRef, mode, isReferedActualPara);
		expression+=variantRef;
		return 0;
	}
	else if(expressionNode->type=="integer"){//如果是整数
		expression+=expressionNode->strOfNum;
		return 0;
	}
	else if(expressionNode->type=="real"){//如果是浮点数
		expression+=expressionNode->strOfNum;
		return 0;
	}
	else if (expressionNode->type == "char") {//如果是字符常量
		expression += string("'") + expressionNode->charVal + string("'");
		return 0;
	}
	else if(expressionNode->type=="function"){//如果是函数调用
		string functionCall;
		inputFunctionCall(expressionNode->functionCall, functionCall, mode);
		expression+=functionCall;
		return 0;
	}
	else if(expressionNode->type=="compound"){//如果是复合表达式
		if(expressionNode->operationType=="single"){//如果是单目运算符
			string tmp;
			flag=inputExpression(expressionNode->operand1, tmp, mode);
			if(expressionNode->operation=="bracket"){//如果是括号
				expression+="("+tmp+")";
				return 0;
			}
			if(expressionNode->operation=="minus")//如果是取相反数
				expression+=" - ";
			else if(expressionNode->operation=="not")//如果是取非
				expression+=" " + transformOpertion(expressionNode->operation ,mode) + " ";
			if (mode == 0 && flag > 0)
				expression += "(" + tmp + ")";
			else
				expression+=tmp;
			return 4;
		}
		else if(expressionNode->operationType=="mulop"){//如果是"mulop"
			string tmp;
			flag=inputExpression(expressionNode->operand1, tmp, mode);
			if(flag!=0&&flag<3)
				expression+="(" + tmp + ") " + transformOpertion(expressionNode->operation, mode) + " ";
			else
				expression+=tmp + " " + transformOpertion(expressionNode->operation, mode) + " ";
			tmp="";
			flag=inputExpression(expressionNode->operand2, tmp, mode);
			if(mode == 0 && flag != 0 && flag < 3)
				expression+="("+tmp+")";
			else
				expression+=tmp;
			return 3;
		}
		else if(expressionNode->operationType=="addop"){//如果是"addop"
			string tmp;
			flag=inputExpression(expressionNode->operand1, tmp, mode);
			if(flag==1)
				expression+="(" + tmp + ") " + transformOpertion(expressionNode->operation, mode) + " ";
			else
				expression+= tmp+ " " + transformOpertion(expressionNode->operation, mode) + " ";
			tmp="";
			flag=inputExpression(expressionNode->operand2, tmp, mode);
			if(mode == 0 && flag == 1)
				expression+="("+tmp+")";
			else
				expression+=tmp;
			return 2;
		}
		else if(expressionNode->operationType=="relop"){//如果是"relop"
			string tmp;
			inputExpression(expressionNode->operand1, tmp, mode);
			expression += tmp + " " + transformOpertion(expressionNode->operation, mode) + " ";
			tmp="";
			inputExpression(expressionNode->operand2, tmp, mode);
			expression += tmp;
			return 1;
		}
		else{
			cout << "[inputExpression] ERROR: operationType missing" << endl;
			return -1;
		}
	}
	else {
		//cout << "[inputExpression] ERROR: expression type error" << endl;
		return -1;
	}
}

//mode表示是否需要将PASCAL运算符转化为C运算符
//有五种kind,"array","var","constant","function call","function return reference"
//左值特判
void inputVariantRef(_VariantReference *variantRefNode, string &variantRef,int mode, bool isReferedActualPara){
	if(variantRefNode==NULL){
		cout << "[inputVariantRef] ERROR: variantRef pointer is null" << endl;
		return;
	}
	variantRef=variantRefNode->variantId.first;
	//如果是函数，一定是右值，如果是左值，代码逻辑保证了不会调用该函数
	//通过了语义分析，保证了该函数形参个数一定为0
	if (mode == 0 && (variantRefNode->kind == "var" || variantRefNode->kind == "array")) {
		bool isRefered = checkIsReferedPara(codeGenerateCurrentSymbolTable, variantRefNode->variantId.first);
		if (isRefered) {
			if (!isReferedActualPara)
				variantRef = "*" + variantRef;
		}
		else {
			if (isReferedActualPara)
				variantRef = "&" + variantRef;
		}
	}
	if (mode == 0 && variantRefNode->kind == "function") 
		variantRef += "()";
	if (variantRefNode->kind != "array")
		return;
	//这里得查符号表，找出数组定义时每一维的下界
	_SymbolRecord *record = findSymbolRecord(codeGenerateCurrentSymbolTable, variantRefNode->variantId.first);
	//if (record == NULL || record->flag != "array" || record->amount != variantRefNode->expressionList.size()) {
		//cout << "[inputVariantRef] get array error" << endl;
		//return;
	//}
	if (mode == 0) { //如果生成C代码
		for (int i = 0; i<variantRefNode->expressionList.size(); i++) {
			string expression;
			inputExpression(variantRefNode->expressionList[i], expression, mode);
			int lowBound = record->arrayRangeList[i].first;
			string delta;
			if (lowBound > 0)
				delta = " - " + itos(lowBound);
			else if (lowBound < 0)
				delta = " + " + itos(abs(lowBound));
			else
				delta = "";
			variantRef += "[" + expression + delta + "]";
		}
	}
	else {//如果按照原样输出
		variantRef += "[";
		for (int i = 0; i<variantRefNode->expressionList.size(); i++) {
			string expression;
			inputExpression(variantRefNode->expressionList[i], expression, mode);
			if (i != 0)
				variantRef += ", ";
			variantRef += expression;
		}
		variantRef += "]";
	}
}

void inputStatementList(_Statement *statementNode, vector< pair<string,int> > &v_statementList, int retract, int flag){
	if(statementNode==NULL){
		cout << "[inputStatementList] ERROR: statement pointer is null" << endl;
		return;
	}
	if(statementNode->type=="compound"){
		_Compound* compoundNode = reinterpret_cast<_Compound*>(statementNode);
		if(flag==0)
			v_statementList.push_back(make_pair("{",retract-1));
		for(int i=0;i<compoundNode->statementList.size();i++)
			inputStatementList(compoundNode->statementList[i], v_statementList, retract);
		if(flag==0)
			v_statementList.push_back(make_pair("}",retract-1));
	}
	else if(statementNode->type=="repeat"){
		_RepeatStatement* repeatStatementNode = reinterpret_cast<_RepeatStatement*>(statementNode);
		string condition;
		v_statementList.push_back(make_pair("do",retract));
		inputStatementList(repeatStatementNode->_do, v_statementList, retract+1);
		inputExpression(repeatStatementNode->condition,condition);
		condition="while(!("+condition+"));";
		v_statementList.push_back(make_pair(condition,retract));
	}
	else if(statementNode->type=="while"){
		string condition;
		_WhileStatement* whileStatementNode = reinterpret_cast<_WhileStatement*>(statementNode);
		inputExpression(whileStatementNode->condition,condition);
		condition="while("+condition+")";
		v_statementList.push_back(make_pair(condition,retract));
		inputStatementList(whileStatementNode->_do, v_statementList, retract+1);
	}
	else if(statementNode->type=="for"){
		string start,end;
		_ForStatement* forStatementNode = reinterpret_cast<_ForStatement*>(statementNode);
		inputExpression(forStatementNode->start,start);
		inputExpression(forStatementNode->end,end);
		v_statementList.push_back(make_pair("for("+ forStatementNode->id.first+" = "+start+"; "+ forStatementNode->id.first+" <= "+end+"; "+ forStatementNode->id.first+"++)",retract));
		inputStatementList(forStatementNode->_do, v_statementList, retract+1);
	}
	else if(statementNode->type=="if"){
		string condition;
		_IfStatement* ifStatementNode = reinterpret_cast<_IfStatement*>(statementNode);
		inputExpression(ifStatementNode->condition,condition);
		v_statementList.push_back(make_pair("if("+condition+")",retract));
		inputStatementList(ifStatementNode->then, v_statementList, retract+1);
		if(ifStatementNode->els!=NULL){
			v_statementList.push_back(make_pair("else",retract));
			inputStatementList(ifStatementNode->els, v_statementList, retract+1);
		}
	}
	else if(statementNode->type=="assign"){//左值特判
		string variantRef;
		_AssignStatement* assignStatementNode = reinterpret_cast<_AssignStatement*>(statementNode);
		string expression;
		inputExpression(assignStatementNode->expression, expression);
		if (assignStatementNode->variantReference->kind == "function return reference") {//如果是返回值语句
			variantRef = "return (" + expression + ");";
			v_statementList.push_back(make_pair(variantRef, retract));
		}
		else {
			inputVariantRef(assignStatementNode->variantReference, variantRef);
			v_statementList.push_back(make_pair(variantRef + " = " + expression + ";", retract));
		}
	}
	else if(statementNode->type=="procedure"){
		_ProcedureCall* procedureCallNode = reinterpret_cast<_ProcedureCall*>(statementNode);
		inputHeadFileList(procedureCallNode->procedureId.first);
		if (checkAndInputLibrarySubprogram(procedureCallNode, v_statementList, retract))
			return;
		_SymbolRecord *record = findSymbolRecord(mainSymbolTable, procedureCallNode->procedureId.first);
		if (record == NULL || record->flag != "procedure") {
			cout << "[inputStatementList] procedure not found" << endl;
			return;
		}
		string procedureCall = procedureCallNode->procedureId.first+"(";
		for(int i=0;i<procedureCallNode->actualParaList.size();i++){
			string expression;
			inputExpression(procedureCallNode->actualParaList[i], expression, 0, record->isXthFormalParaRefered(i + 1));
			if(i!=0)
				procedureCall+=", ";
			procedureCall+=expression;
		}
		procedureCall += ");";
		v_statementList.push_back(make_pair(procedureCall,retract));
	}
}

void inputSubproDef(_FunctionDefinition* functionDefinitionNode){
	if(functionDefinitionNode==NULL){
		cout << "[inputSubproDef] ERROR: functionDefinitionNode is null" << endl;
		return;
	}
	_SymbolRecord *record = findSymbolRecord(mainSymbolTable, functionDefinitionNode->functionID.first);
	//if (record == NULL || (record->flag!="function" && record->flag!="procedure")) {
		//cout << "[inputSubproDef] function or procedure id not found!" << endl;
		//return;
	//}
	struct subproDef tmp;
	inputConstList(functionDefinitionNode->constList, tmp.constIdList, tmp.constTypeList, tmp.constValueList, record->subSymbolTable);
	inputVariantList(functionDefinitionNode->variantList, tmp.variantIdList, tmp.variantTypeList, tmp.arraySizeList, record->subSymbolTable);
	inputStatementList(functionDefinitionNode->compound, tmp.statementList, 1, 1);
	subproDefList.push_back(tmp);
}

void inputSubproDefList(_SubProgram* subProgramNode){
	if(subProgramNode==NULL){
		cout << "[inputSubproDefList] ERROR: subProgramNode is null" << endl;
		return;
	}
	//初始化
	subproDefList.clear();
	for (int i = 0; i < subProgramNode->subprogramDefinitionList.size(); i++) {
		_SymbolRecord *record = findSymbolRecord(mainSymbolTable, subProgramNode->subprogramDefinitionList[i]->functionID.first);
		//if (record == NULL || (record->flag != "function" && record->flag != "procedure")) {
			//cout << "subProgramNode definition not found" << endl;
			//return;
		//}
		codeGenerateCurrentSymbolTable = record->subSymbolTable; //定位到子符号表
		inputSubproDef(subProgramNode->subprogramDefinitionList[i]);
	}
}

void inputSubMainFunction(_Program* ASTRoot) {
	if (ASTRoot == NULL) {
		cout << "[inputSubMainFunction] pointer of _Program is null" << endl;
		return;
	}
	subMainFunctionDeclaration = "void " + ASTRoot->programId.first + "()";
	inputStatementList(ASTRoot->subProgram->compound, v_statementList, 1, 1);
}

string getOutputFormat(string type) {
	if (type == "integer")
		return "%d";
	if (type == "real")
		return "%f";
	if (type == "char")
		return "%c";
	if (type == "boolean")
		return "bool";
	cout << "[getOutputFormat] type error" << endl;
	return "";
}

bool checkAndInputLibrarySubprogram(_ProcedureCall* procedureCall, vector< pair<string, int> > &v_statementList, int retract) {
	string proCall;
	string &id = procedureCall->procedureId.first;
	if (id == "exit") {
		proCall += "return";
		if (procedureCall->actualParaList.size() > 1) {
			cout << "[checkAndInputLibrarySubprogram] size of actualParaList is too large" << endl;
			return false;
		}
		if (procedureCall->actualParaList.size()) {
			string expression;
			inputExpression(procedureCall->actualParaList[0], expression);
			proCall += " (" + expression + ")";
		}
		proCall += ";";
		v_statementList.push_back(make_pair(proCall, retract));
	}
	else if (id == "write" || id == "writeln") {
		if (id == "writeln"&&procedureCall->actualParaList.size() == 0) {
			v_statementList.push_back(make_pair("printf(\"\\n\");", retract));
			return true;
		}
		proCall += "printf(\"";
		string exp="";//exp按顺序保存了表达式
		for (int i = 0; i < procedureCall->actualParaList.size(); i++) {
			string expression, typeFormat;
			inputExpression(procedureCall->actualParaList[i], expression);
			typeFormat = getOutputFormat(procedureCall->actualParaList[i]->expressionType);
			if (typeFormat == "bool") {
				if (exp != "") {
					proCall += "\"" + exp + ");";
					v_statementList.push_back(make_pair(proCall, retract));
					proCall = "printf(\"";
					exp = "";
				}
				v_statementList.push_back(make_pair("{", retract));
				v_statementList.push_back(make_pair("if(" + expression + ")", retract + 1));
				v_statementList.push_back(make_pair("printf(\"true\");", retract + 2));
				v_statementList.push_back(make_pair("else", retract + 1));
				v_statementList.push_back(make_pair("printf(\"false\");", retract + 2));
				v_statementList.push_back(make_pair("}", retract));
				if (i == procedureCall->actualParaList.size() - 1 && id == "writeln")
					v_statementList.push_back(make_pair("printf(\"\\n\");", retract));
			}
			else {
				proCall += typeFormat;
				exp += ", " + expression;
			}
		}
		if (exp != "") {
			if (id == "writeln")
				proCall += "\\n";
			proCall += "\"" + exp + ");";
			v_statementList.push_back(make_pair(proCall, retract));
		}
	}
	else if (id == "read") {
		proCall += "scanf(\"";
		string exp = "";
		for (int i = 0; i < procedureCall->actualParaList.size(); i++) {
			string expression, typeFormat;
			inputExpression(procedureCall->actualParaList[i], expression, 0, true); //引用参数
			typeFormat = getOutputFormat(procedureCall->actualParaList[i]->expressionType);
			if (typeFormat == "bool")
				typeFormat = "%d";
			proCall += typeFormat;
			//exp += ", &" + expression;
			exp += ", " + expression;
		}
		proCall += "\"" + exp + ");";
		v_statementList.push_back(make_pair(proCall, retract));
	}
	else 
		return false;
	return true;
}

void outputHeadFileList() {
	for (auto it = mp_headFileShow.begin(); it != mp_headFileShow.end(); it++) {//遍历
		if (it->second) {
			fout << "#include<" << it->first << ">" << endl;
			it->second = false;
		}
	}
	fout << "#include<stdbool.h>" << endl;
}

void outputConstList(vector<string> &constIdList, vector<string> &constTypeList, vector<string> &constValueList, int retract) {
	if (!isEqual(3, constIdList.size(), constTypeList.size(), constValueList.size())) {
		cout << "[outputConstList] ERROR: constant list size miss match" << endl;
		return;
	}
	int n = int(constIdList.size());
	for (int i = 0; i<n; i++) {
		for (int j = 0; j<retract; j++)
			fout << "\t";
		fout << "const " << constTypeList[i] << " " << constIdList[i] << " = " << constValueList[i] << ";" << endl;
	}
}

void outputVariantList(vector<string> &variantIdList, vector<string> &variantTypeList, vector< vector<int> > &arraySizeList, int retract) {
	if (!isEqual(3, variantIdList.size(), variantTypeList.size(), arraySizeList.size())) {
		cout << "[outputConstList] ERROR: variant list size miss match" << endl;
		return;
	}
	int n = int(variantIdList.size());
	for (int i = 0; i<n; i++) {
		for (int j = 0; j<retract; j++)
			fout << "\t";
		fout << variantTypeList[i] << " " << variantIdList[i];
		int m = int(arraySizeList[i].size());
		for (int j = 0; j<m; j++)
			fout << "[" << arraySizeList[i][j] << "]";
		fout << ";" << endl;
	}
}

void outputSubproDec(subproDec &tmp, int flag) {//flag==0时表示接口声明,flag==1时表示定义时的子程序头
	string str = tmp.returnType == "" ? "void" : tmp.returnType;
	fout << str << " " << tmp.id << "(";
	int m = int(tmp.v_paraIdList.size());
	for (int j = 0; j<m; j++) {
		if (j != 0)
			fout << ", ";
		fout << tmp.v_paraTypeList[j] << " ";
		if (tmp.v_isParaRef[j])
			fout << "*";
		fout << tmp.v_paraIdList[j];
	}
	fout << ")";
	if (flag == 0)
		fout << ";";
	fout << endl;
}

void outputSubproDecList() {
	fout << subMainFunctionDeclaration << ";" << endl;//原PASCAL主程序对应C的程序头
	vector< struct subproDec > &vec = v_subproDecList;
	int n = int(vec.size());
	for (int i = 0; i<n; i++) {
		if (vec[i].id == "") {
			fout << "[outputSubproDecList] ERROR: " << i << "th" << " subprogram's id missing" << endl;
			return;
		}
		if (!isEqual(3, vec[i].v_paraIdList.size(), vec[i].v_isParaRef.size(), vec[i].v_paraTypeList.size())) {
			fout << "[outputSubproDecList] ERROR: " << i << "th" << " subprogram's vector size mismatch" << endl;
			return;
		}
	}
	for (int i = 0; i<n; i++)
		outputSubproDec(vec[i], 0);
}

void outputStatement(pair<string, int> &tmp) {
	int n = tmp.second;
	for (int i = 0; i<n; i++)
		fout << "\t";
	fout << tmp.first << endl;
}

void outputStatementList(vector< pair<string, int> > &vec) {
	int n = int(vec.size());
	for (int i = 0; i<n; i++)
		outputStatement(vec[i]);
	vec.clear();//清空
}

void outputSubproDefList() {
	int n = int(subproDefList.size());
	vector<subproDef> &vec = subproDefList;
	for (int i = 0; i<n; i++) {
		if (!isEqual(3, vec[i].constIdList.size(), vec[i].constTypeList.size(), vec[i].constValueList.size())) {
			fout << "[outputSubproDefList] ERROR: " << i << "th" << "subproDef's " << "constant size mismatch" << endl;
			return;
		}
		if (!isEqual(3, vec[i].variantIdList.size(), vec[i].variantTypeList.size(), vec[i].arraySizeList.size())) {
			fout << "[outputSubproDefList] ERROR: " << i << "th" << "subproDef's " << "variant size mismatch" << endl;
			return;
		}
	}
	fout << subMainFunctionDeclaration << endl;
	fout << "{" << endl;
	outputStatementList(v_statementList);
	fout << "}" << endl << endl;
	for (int i = 0; i<n; i++) {
		outputSubproDec(v_subproDecList[i], 1);
		fout << "{" << endl;
		outputConstList(vec[i].constIdList, vec[i].constTypeList, vec[i].constValueList, 1);
		outputVariantList(vec[i].variantIdList, vec[i].variantTypeList, vec[i].arraySizeList, 1);
		outputStatementList(vec[i].statementList);
		fout << "}" << endl << endl;
	}
}

void outputMain(_Program* ASTRoot) {
	fout << "int main()\n{\n\t";
	fout << ASTRoot->programId.first + "()";
	fout << "; \n\treturn 0; \n}\n";
}