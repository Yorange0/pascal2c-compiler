/*
语义分析实现代码
*/
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>
#include "symbolTable.h"
#include "ASTnodes.h"
#include "main.h"

using namespace std;

extern _SymbolTable *mainSymbolTable;//主符号表
extern _SymbolTable *currentSymbolTable;//当前符号表
extern _SymbolRecord* findSymbolRecord(_SymbolTable* currentSymbolTable, string id, int mode=0);//从符号表中找出id对应的记录
extern void inputFunctionCall(_FunctionCall *functionCallNode, string &functionCall, int mode=0);//获取函数调用
extern int inputExpression(_Expression *expressionNode, string &expression, int mode=0, bool isReferedActualPara=false);//获取表达式
extern void inputVariantRef(_VariantReference *variantRefNode, string &variantRef, int mode=0, bool isReferedActualPara=false);//获取变量引用
extern int str2int(string str);
extern void returnExistedCheckFunctionDefinition(_FunctionDefinition* functionDefinitionNode);

vector<string> semanticErrorInformation;//存储错误信息的列表
vector<string> semanticWarningInformation;//存储警告信息的列表

void SemanticAnalyse(_Program *ASTRoot);

void createSymbolTableAndInit();//创建主符号表并初始化
void createSubSymbolTableAndInit();//创建子符号表并初始化

string SemanticAnalyseVariantReference(_VariantReference* variantReference);//对变量引用进行语义分析
void SemanticAnalyseStatement(_Statement *statement);//对语句进行语义分析
void SemanticAnalyseSubprogramDefinition(_FunctionDefinition* functionDefinition);//对子程序定义进行语义分析
void SemanticAnalyseVariant(_Variant* variant);//对变量定义进行语义分析
void SemanticAnalyseConst(_Constant* constant);//对常量定义进行语义分析
void SemanticAnalyseSubprogram(_SubProgram* subprogram);//对分程序进行语义分析
void SemanticAnalyseProgram(_Program *program);//对程序进行语义分析
void SemanticAnalyseFormalParameter(_FormalParameter *formalParameter);//对形式参数进行语义分析
string SemanticAnalyseFunctionCall(_FunctionCall *functionCall);//对函数调用进行语义分析
string SemanticAnalyseExpression(_Expression* expression);//对表达式进行语义分析

string itos(int num);//将int转化为string

//添加重定义错误信息
void addDuplicateDefinitionErrorInformation(string preId, int preLineNumber, string preFlag, string preType,int curLineNumber);//获得重复定义的语义错误信息
//添加未定义错误信息
void addUndefinedErrorInformation(string id, int curLineNumber);
//添加标识符类型错误信息
void addUsageTypeErrorInformation(string curId, int curLineNumber, string curType, string usage, string correctType);
//添加个数不匹配错误信息
void addNumberErrorInformation(string curId, int curLineNumber, int curNumber, int correctNumber, string usage);
//添加标识符种类错误信息
void addPreFlagErrorInformation(string curId, int curLineNumber, string curFlag, int preLineNumber, string preFlag);
//添加表达式类型错误信息
void addExpressionTypeErrorInformation(_Expression *exp, string curType, string correctType, string description);
//添加赋值语句左值和右值类型不匹配错误信息
void addAssignTypeMismatchErrorInformation(_VariantReference *leftVariantReference, _Expression *rightExpression);
//添加数组下标越界错误信息
void addArrayRangeOutOfBoundErrorInformation(_Expression *expression, string arrayId, int X, pair<int,int> range);
//添加数组下界比上界大的错误信息
void addArrayRangeUpSideDownErrorInformation(string curId, int curLineNumber, int X, int lowBound, int highBound);
//添加运算符两边的操作数类型不一致的错误信息
void addOperandExpressionsTypeMismatchErrorInformation(_Expression *exp1, _Expression *exp2);
//添加某个操作数类型错误的信息
void addSingleOperandExpressionTypeMismatchErrorInformation(_Expression *exp, string correctType);
//添加read的实参错误信息
void addactualParameterOfReadErrorInformation(int curLineNumber, string procedureId, int X, _Expression *exp);
//添加除0错误信息
void addDivideZeroErrorInformation(string operation, _Expression *exp);
//添加read读取boolean类型变量错误的信息
void addReadBooleanErrorInformation(_Expression *exp, int X);
//将错误信息直接添加到错误信息的列表中
void addGeneralErrorInformation(string errorInformation);

//检查id是否与库程序名、主程序名、主程序参数同名 这个也可以算作符号表接口
bool checkIsTheSameAsKey(string id, int lineNumber); //这里的Key指的是库程序名、主程序名、主程序参数

void SemanticAnalyse(_Program *ASTRoot) {
	createSymbolTableAndInit();
	SemanticAnalyseProgram(ASTRoot);
}

void createSymbolTableAndInit() {//创建主符号表
	currentSymbolTable = mainSymbolTable = new _SymbolTable("main");//指定为主符号表后，会自动加入read, write等库函数
}

void createSubSymbolTableAndInit(){//创建子符号表 符号表定位
	currentSymbolTable = new _SymbolTable("sub");//创建并定位到子符号表
}

//对变量引用进行语义分析
//变量引用作为左值，可能是传值参数、引用参数、普通变量、数组元素、函数名（函数返回值）
//变量引用作为右值，可能是传值参数、引用参数、普通变量、数组元素、函数名（不带参数的函数）
string SemanticAnalyseVariantReference(_VariantReference* variantReference){
	if(variantReference==NULL){
		cout << "[SemanticAnalyseVariantReference] pointer of _VariantReference is null" << endl;
		return "";
	}
	_SymbolRecord* record = findSymbolRecord(currentSymbolTable, variantReference->variantId.first);
	if (record == NULL) {//未定义 //checked
		addUndefinedErrorInformation(variantReference->variantId.first, variantReference->variantId.second);
		return variantReference->variantType = "error";//无法找到变量引用的类型
	}
	//首先必须是函数
	//作为左值必须是当前函数名
	//作为右值则可以是任意函数名（递归调用也可以）
	//是左值还是右值、是过程还是函数、是当前符号表对应的子程序名称、还是别的子程序名称

	if (variantReference->flag == 0) {//如果是非数组
		if (record->flag == "(sub)program name") {
			if (record->subprogramType == "procedure") {//这个包含了查到主程序名的情况
				//这里有空的话，最好把查到主程序名的报错单独拿出来 hhh checked
				addGeneralErrorInformation("[Invalid reference] <Line " + itos(variantReference->variantId.second) + "> Procedure name \"" + record->id + "\" can't be referenced");
				return variantReference->variantType = "error";
			}
			//如果是函数，那么一定是当前符号表
			//如果是左值，那么是返回语句，不需要关注参数
			//如果是右值，那么是递归调用，需要关注参数，即检查形式参数个数是否是0个
			if (variantReference->locFlag == -1) { //如果是左值
				variantReference->kind = "function return reference";
				return variantReference->variantType = record->type;
			}
			//如果是右值
			if (record->amount != 0) {//如果形参个数不为0 checked 递归调用
				addNumberErrorInformation(variantReference->variantId.first, variantReference->variantId.second, 0, record->amount, "function");
				return variantReference->variantType = record->type;
			}
			//如果形参个数为0
			//这样对应到具体程序中，实际上是无参函数的递归调用
			variantReference->kind = "function call";
			return variantReference->variantType = record->type;
		}
		if (record->flag == "function") {//如果是函数 则一定是在主符号表中查到的
			variantReference->kind = "function";
			//不能作为左值，必须作为右值，且形参个数必须为0
			//被识别为variantReference的函数调用一定不含实参，所以需要检查形参个数
			if (variantReference->locFlag == -1) {//如果是左值 checked
				addGeneralErrorInformation("[Invalid reference!] <Line " + itos(variantReference->variantId.second) + "> function name \"" + record->id + "\" can't be referenced as l-value.");
				return variantReference->variantType = "error";
			}
			//如果是右值
			if (record->amount != 0) {//如果形参个数不为0 checked
				addNumberErrorInformation(variantReference->variantId.first, variantReference->variantId.second, 0, record->amount, "function");
				return variantReference->variantType = record->type;
			}
			return variantReference->variantType = record->type;
		}
		//checked
		if (!(record->flag == "value parameter" || record->flag == "var parameter" || record->flag == "normal variant" || record->flag == "constant")) {
			addGeneralErrorInformation("[Invalid reference!] <Line " + itos(variantReference->variantId.second) + "> \"" + variantReference->variantId.first + "\" is a " + record->flag + ", it can't be referenced.");
			return variantReference->variantType = "error";
		}
		variantReference->kind = "var";
		if (record->flag == "constant")
			variantReference->kind = "constant";
		return variantReference->variantType = record->type;
	}
	else if (variantReference->flag == 1) {//如果是数组
		if (record->flag != "array") {//如果符号表中记录的不是数组 checked
			addPreFlagErrorInformation(variantReference->variantId.first, variantReference->variantId.second, "array", record->lineNumber, record->flag);
			return variantReference->variantType = "error";
		}
		variantReference->kind = "array";
		//checked
		if (variantReference->expressionList.size() != record->amount) {//如果引用时的下标维数和符号表所存不一致
			addNumberErrorInformation(variantReference->variantId.first, variantReference->variantId.second, int(variantReference->expressionList.size()), record->amount, "array");
			variantReference->variantType = "error";
			return record->type;
		}
		variantReference->variantType = record->type;
		for (int i = 0; i < variantReference->expressionList.size(); i++) {
			string type = SemanticAnalyseExpression(variantReference->expressionList[i]);
			//检查下标表达式的类型是否是整型 checked
			if (type != "integer") {
				addExpressionTypeErrorInformation(variantReference->expressionList[i], type, "integer", itos(i + 1) + "th index of array \"" + variantReference->variantId.first + "\"");
				variantReference->variantType = "error";
			}
			//检查越界 checked
			if(variantReference->expressionList[i]->totalIntValueValid){
				if(!record->checkArrayXthIndexRange(i, variantReference->expressionList[i]->totalIntValue)){
					addArrayRangeOutOfBoundErrorInformation(variantReference->expressionList[i], variantReference->variantId.first, i, record->arrayRangeList[i]);
					variantReference->variantType = "error";
				}
			}
		}
		return record->type;
	}
	else {
		cout << "[SemanticAnalyseVariantReference] flag of variantReference is not 0 or 1" << endl;
		return variantReference->variantType = "error";
	}
}

//对语句进行语义分析
void SemanticAnalyseStatement(_Statement *statement){
	if(statement==NULL){
		cout << "[SemanticAnalyseStatement] pointer of _Statement is null" << endl;
		return;
	}
    if(statement->type=="compound"){
        _Compound *compound = reinterpret_cast<_Compound*>(statement);//对复合语句块中的每一条语句进行语义分析
        for(int i=0;i<compound->statementList.size();i++)
            SemanticAnalyseStatement(compound->statementList[i]);
    }
    else if(statement->type=="repeat"){
        _RepeatStatement *repeatStatement = reinterpret_cast<_RepeatStatement*>(statement);
		string type = SemanticAnalyseExpression(repeatStatement->condition);
		if (type != "boolean") {//repeat语句类型检查,condition表达式类型检查 checked
			addExpressionTypeErrorInformation(repeatStatement->condition, type, "boolean", "condition of repeat-until statement");
			repeatStatement->statementType = "error";
		}
		else
            repeatStatement->statementType="void";
        SemanticAnalyseStatement(repeatStatement->_do);//对循环体语句进行语义分析
    }
    else if(statement->type=="while"){
        _WhileStatement *whileStatement = reinterpret_cast<_WhileStatement*>(statement);
		string type = SemanticAnalyseExpression(whileStatement->condition);
		if (type != "boolean") {//while语句类型检查,condition表达式类型检查 checked
			addExpressionTypeErrorInformation(whileStatement->condition, type, "boolean", "condition of while statement");
			whileStatement->statementType = "error";
		}
		else
            whileStatement->statementType="void";
        SemanticAnalyseStatement(whileStatement->_do);//对循环体语句进行语义分析
    }
    else if(statement->type=="for"){
        _ForStatement *forStatement = reinterpret_cast<_ForStatement*>(statement);
        //检查循环变量是否已经定义，如已经定义，是否为integer型变量
		_SymbolRecord *record = findSymbolRecord(currentSymbolTable, forStatement->id.first);
		if(record==NULL){//循环变量未定义，错误信息 checked
			addUndefinedErrorInformation(forStatement->id.first, forStatement->id.second);
			return;
		}
		//如果无法作为循环变量 checked
		if (!(record->flag == "value parameter" || record->flag == "var parameter" || record->flag == "normal variant")) {//如果当前符号种类不可能作为循环变量
			addPreFlagErrorInformation(forStatement->id.first, forStatement->id.second, "value parameter, var parameter or normal variant", record->lineNumber, record->flag);
			return;
		}
		//如果类型不是整型 checked
		if (record->type != "integer") {
			addUsageTypeErrorInformation(forStatement->id.first, forStatement->id.second, record->type, "cyclic variable of for statement", "integer");
			return;
		}
        //for语句类型检查,start和end表达式类型检查
		forStatement->statementType = "void";
		string type = SemanticAnalyseExpression(forStatement->start);
		if (type != "integer") { //checked
			addExpressionTypeErrorInformation(forStatement->start, type, "integer", "start value of for statement");
			forStatement->statementType = "error";
		}
		type = SemanticAnalyseExpression(forStatement->end);
		if (type != "integer") { //checked
			addExpressionTypeErrorInformation(forStatement->end, type, "integer", "end value of for statement");
			forStatement->statementType = "error";
		}
        //对循环体语句进行语义分析
        SemanticAnalyseStatement(forStatement->_do);
    }
    else if(statement->type=="if"){
        _IfStatement *ifStatement = reinterpret_cast<_IfStatement*>(statement);
		string type = SemanticAnalyseExpression(ifStatement->condition);
		if (type != "boolean") {//if语句类型检查,condition表达式类型检查 checked
			addExpressionTypeErrorInformation(ifStatement->condition, type, "boolean", "condition of if statement");
			ifStatement->statementType = "error";
		}
		else
            ifStatement->statementType="void";
        SemanticAnalyseStatement(ifStatement->then);//对then语句进行语义分析
        if(ifStatement->els!=NULL)//对else语句进行语句分析
            SemanticAnalyseStatement(ifStatement->els);
    }
    else if(statement->type=="assign"){//左值特判
        _AssignStatement *assignStatement = reinterpret_cast<_AssignStatement*>(statement);
        //对左值变量引用进行语义分析,获得leftType
		assignStatement->statementType = "void";
		assignStatement->variantReference->locFlag = -1;//标记为左值
        string leftType = SemanticAnalyseVariantReference(assignStatement->variantReference);
		if (assignStatement->variantReference->kind == "constant") {
			//左值不能为常量 checked
			addGeneralErrorInformation("[Constant as l-value error!] <Line" + itos(assignStatement->variantReference->variantId.second) + "> Costant \"" + assignStatement->variantReference->variantId.first + "\" can't be referenced as l-value.");
			return;
		}
		//对右值表达式进行类型检查,获得rightType
        string rightType = SemanticAnalyseExpression(assignStatement->expression);
		if (assignStatement->variantReference->kind == "function return reference") {//如果是返回值语句
			//需检查返回值表达式是否和函数返回值类型一致
			if (assignStatement->variantReference->variantType != rightType && !(assignStatement->variantReference->variantType=="real" and rightType=="integer")) {
				//checked
				addGeneralErrorInformation("[Return type of funciton mismatch!] <Line " + itos(assignStatement->expression->lineNumber) + "> The type of return expression is " + rightType + " ,but not " + assignStatement->variantReference->variantType + " as function \"" + assignStatement->variantReference->variantId.first + "\" defined.");
				assignStatement->statementType = "error";
			}
			assignStatement->isReturnStatement = true;
			return;
		}
        //比较左值和右值类型,获得赋值语句的类型；类型不同时，只支持整型到实型的隐式转换
		if (leftType != rightType && !(leftType=="real" && rightType=="integer")) {
			//checked
			addAssignTypeMismatchErrorInformation(assignStatement->variantReference, assignStatement->expression);
			assignStatement->statementType = "error";
		}
		else
			assignStatement->statementType = "void";
    }
    else if(statement->type=="procedure"){//read的参数只能是变量或数组元素;
        _ProcedureCall *procedureCall = reinterpret_cast<_ProcedureCall*>(statement);
        //通过procedureId查表，获得参数个数、参数类型等信息
		//www
		//_SymbolRecord *record = findSymbolRecord(currentSymbolTable, procedureCall->procedureId.first);
		_SymbolRecord *record = findSymbolRecord(mainSymbolTable, procedureCall->procedureId.first);
		if (record == NULL)
			record = findSymbolRecord(currentSymbolTable, procedureCall->procedureId.first, 1);
		procedureCall->statementType = "void";
		if (record == NULL) {//未定义 checked
			addUndefinedErrorInformation(procedureCall->procedureId.first, procedureCall->procedureId.second);
			procedureCall->statementType = "error";
			return;
		}
		if (record->flag != "procedure") {//如果不是过程 checked
			addPreFlagErrorInformation(procedureCall->procedureId.first, procedureCall->procedureId.second, "procedure", record->lineNumber, record->flag);
			procedureCall->statementType = "error";
			return;
		}
		if (record->id == "exit") {
			/*exit既可以出现在过程中，也可以出现在函数中，出现在过程中时，
			exit不能带参数，出现在函数中时，exit只能带一个参数，
			且该参数表达式的类型必须和函数的返回值类型一致*/
			//所以需判断当前程序是过程还是函数
			if (currentSymbolTable->recordList[0]->subprogramType == "procedure") {//如果是过程
				//exit不能带参数表达式
				if (procedureCall->actualParaList.size() != 0) {//如果实参个数不为0 checked
					addGeneralErrorInformation("[Return value redundancy!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of procedure must be 0, that is, exit must have no actual parameters.");
					procedureCall->statementType = "error";
				}
				return;
			}
			//如果是函数
			if (procedureCall->actualParaList.size() != 1) {//如果实参个数不为1
				if (procedureCall->actualParaList.size() == 0) //checked
					addGeneralErrorInformation("[Return value missing!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of function must be 1, that is, exit must have 1 actual parameters.");
				else //checked
					addGeneralErrorInformation("[Return value redundancy!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of function must be 1, that is, exit must have 1 actual parameters.");
				return;
			}
			//如果实参个数为1，检查实参表达式的类型，检查是否与函数返回值类型一致
			string returnType=SemanticAnalyseExpression(procedureCall->actualParaList[0]);
			if (currentSymbolTable->recordList[0]->type != returnType && !(currentSymbolTable->recordList[0]->type == "real" && returnType=="integer")) { 
				//checked
				addGeneralErrorInformation("[Return type of funciton mismatch!] <Line " + itos(procedureCall->actualParaList[0]->lineNumber) + "> The type of return expression is " + returnType + " ,but not " + currentSymbolTable->recordList[0]->type + " as function \"" + currentSymbolTable->recordList[0]->id + "\" defined.");
				procedureCall->statementType = "error";
			}
			procedureCall->isReturnStatement = true;
			return;
		}
		if (record->id == "read" || record->id == "write") {
			if (procedureCall->actualParaList.size() == 0) { //read、write的参数个数不能为0 checked
				string tmp = record->id;
				tmp[0] -= 'a' - 'A';
				addGeneralErrorInformation("[" + tmp + " actual parameter missing!] <Line " + itos(procedureCall->procedureId.second) + "> procedure \"" + record->id + "\" must have at least one actual parameter.");
				procedureCall->statementType = "error";
			}
		}
		if (record->id == "read") {//参数只能是变量或数组元素，不能是常量、表达式等
			for (int i = 0; i < procedureCall->actualParaList.size(); i++) {
				string actualType = SemanticAnalyseExpression(procedureCall->actualParaList[i]);
				//checked
				if (!(procedureCall->actualParaList[i]->type == "var" && (procedureCall->actualParaList[i]->variantReference->kind == "var" || procedureCall->actualParaList[i]->variantReference->kind == "array"))) 
					addactualParameterOfReadErrorInformation(procedureCall->actualParaList[i]->lineNumber, record->id, i + 1, procedureCall->actualParaList[i]);
				if (procedureCall->actualParaList[i]->expressionType == "boolean")
					addReadBooleanErrorInformation(procedureCall->actualParaList[i], i + 1);
				if (actualType == "error")
					procedureCall->statementType = "error";
			}
			return;
		}
		if (record->amount == -1) {//如果是变参过程（本编译器涉及的变参过程(除了read)对参数类型没有要求，但不能为error）
			for (int i = 0; i < procedureCall->actualParaList.size(); i++) {
				string actualType = SemanticAnalyseExpression(procedureCall->actualParaList[i]);
				if (actualType == "error")
					procedureCall->statementType = "error";
			}
			return;
		}
		if (procedureCall->actualParaList.size() != record->amount) { //checked
			addNumberErrorInformation(procedureCall->procedureId.first, procedureCall->procedureId.second, int(procedureCall->actualParaList.size()), record->amount, "procedure");
			procedureCall->statementType = "error";
			return;
		}
		// 形参在符号表中的定位
		for (int i = 0; i < procedureCall->actualParaList.size(); i++) {//检查actualParaList各表达式的类型，检查实参和形参的类型一致性
			string actualType = SemanticAnalyseExpression(procedureCall->actualParaList[i]);
			string formalType = record->findXthFormalParaType(i + 1);
			bool isRefered = record->isXthFormalParaRefered(i + 1);
			if (isRefered && !(procedureCall->actualParaList[i]->type == "var" && (procedureCall->actualParaList[i]->variantReference->kind == "var" || procedureCall->actualParaList[i]->variantReference->kind == "array"))) {
				//该表达式不能作为引用形参对应的实参 checked
				addGeneralErrorInformation("[Referenced actual parameter error!] <Line " + itos(procedureCall->actualParaList[i]->lineNumber) + "> The " + itos(i + 1) + "th actual parameter expression should be a normal variable、value parameter、referenced parameter or array element.");
				continue;
			}
			//if(isRefered && procedureCall->actualParaList[i]->type==)
			if (!isRefered) { //传值参数支持integer到real的隐式类型转换
				if (actualType != formalType && !(actualType == "integer" && formalType == "real")) { //如果类型不一致
				    //checked
					addExpressionTypeErrorInformation(procedureCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of procedure call of \"" + procedureCall->procedureId.first + "\"");
					procedureCall->statementType = "error";
				}
			}
			else { //引用参数需保持类型强一致
				if (actualType != formalType) { //如果类型不一致
				    //checked
					addExpressionTypeErrorInformation(procedureCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of procedure call of \"" + procedureCall->procedureId.first + "\"");
					procedureCall->statementType = "error";
				}
			}
		}
    }
	else {
		cout << "[SemanticAnalyseStatement] statement type error" << endl;
		return;
	}
}

//对形式参数进行语义分析，形式参数一定是基本类型
void SemanticAnalyseFormalParameter(_FormalParameter* formalParameter){
	if(formalParameter==NULL){
		cout << "[SemanticAnalyseFormalParameter] pointer of _FormalParameter is null" << endl;
		return;
	}
	//检查是否与库程序名、主程序名、主程序参数同名
	//if (checkIsTheSameAsKey(formalParameter->paraId.first, formalParameter->paraId.second))
		//return;
	_SymbolRecord *record = findSymbolRecord(currentSymbolTable, formalParameter->paraId.first, 1);
	if (!checkIsTheSameAsKey(formalParameter->paraId.first, formalParameter->paraId.second)) {
		if (record != NULL) //如果重定义
			addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, formalParameter->paraId.second);
	}
	//检查是否与当前程序名以及在这之前定义的形参同名，如果同名，添加下划线进行恢复（在add函数中进行）
	if(formalParameter->flag==0)
		currentSymbolTable->addPara(formalParameter->paraId.first, formalParameter->paraId.second, formalParameter->type);
	else
		currentSymbolTable->addVarPara(formalParameter->paraId.first, formalParameter->paraId.second, formalParameter->type);
}

//对子程序定义进行语义分析
void SemanticAnalyseSubprogramDefinition(_FunctionDefinition* functionDefinition){
	if(functionDefinition==NULL){
		cout << "[SemanticAnalyseSubprogramDefinition] pointer of _FunctionDefinition is null" << endl;
		return;
	}
	_SymbolRecord *record=findSymbolRecord(currentSymbolTable, functionDefinition->functionID.first, 1);
	if(record!=NULL){//重定义 checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, functionDefinition->functionID.second);
		return;
	}
	string subprogramType;
	if (functionDefinition->type.first == "")
		subprogramType = "procedure";
	else
		subprogramType = "function";
    createSubSymbolTableAndInit();//创建并定位到子表
	//将子程序名等信息添加到子符号表中
	currentSymbolTable->addProgramName(functionDefinition->functionID.first, functionDefinition->functionID.second, subprogramType, int(functionDefinition->formalParaList.size()), functionDefinition->type.first);
	
	//根据type是否为NULL，分为addProcedure()和addFunction()，添加到主程序表中
	if (functionDefinition->type.first=="")//如果是过程
		mainSymbolTable->addProcedure(functionDefinition->functionID.first, functionDefinition->functionID.second, int(functionDefinition->formalParaList.size()), currentSymbolTable);
	else//如果是函数
		mainSymbolTable->addFunction(functionDefinition->functionID.first, functionDefinition->functionID.second, functionDefinition->type.first, int(functionDefinition->formalParaList.size()), currentSymbolTable);
	
	//对形式参数列表进行语义分析，并将形式参数添加到子符号表中
	for(int i=0;i<functionDefinition->formalParaList.size();i++)
		SemanticAnalyseFormalParameter(functionDefinition->formalParaList[i]);
	//对常量定义进行语义分析
	for (int i = 0; i<functionDefinition->constList.size(); i++)
		SemanticAnalyseConst(functionDefinition->constList[i]);
    //对变量定义进行语义分析
	for (int i = 0; i<functionDefinition->variantList.size(); i++)
		SemanticAnalyseVariant(functionDefinition->variantList[i]);
    //对compound进行语义分析
	SemanticAnalyseStatement(reinterpret_cast<_Statement*>(functionDefinition->compound));
	//对函数进行返回值语句的存在性检查
	if (functionDefinition->type.first != "") //checked
		returnExistedCheckFunctionDefinition(functionDefinition);
}

//对变量定义进行语义分析
void SemanticAnalyseVariant(_Variant* variant){
	if(variant==NULL){
		cout << "[SemanticAnalyseVariant] pointer of _Variant is null" << endl;
		return;
	}
    _SymbolRecord* record=findSymbolRecord(currentSymbolTable, variant->variantId.first, 1);//用variantId.first去查符号表，检查是否重定义
	if (checkIsTheSameAsKey(variant->variantId.first, variant->variantId.second))
		return;
	if (record != NULL){//如果重定义 checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, variant->variantId.second);
		return;
	}
	if(variant->type->flag==0)//如果是普通变量
		currentSymbolTable->addVar(variant->variantId.first, variant->variantId.second, variant->type->type.first);
	else {//如果是数组
		//数组定义时，上下界的限制是上界必须大于等于下界；按照文法定义，上下界均为无符号数，且通过了语法分析，就一定是无符号数
		vector< pair<int, int> > &tmp = variant->type->arrayRangeList;
		for (int i = 0; i < tmp.size(); i++) {
			if (tmp[i].first > tmp[i].second) { //checked
				addArrayRangeUpSideDownErrorInformation(variant->variantId.first, variant->variantId.second, i + 1, tmp[i].first, tmp[i].second);
				tmp[i].second = tmp[i].first; //如果上界小于下界，将上界设置为下界
			}
		}
		currentSymbolTable->addArray(variant->variantId.first, variant->variantId.second, variant->type->type.first, int(variant->type->arrayRangeList.size()), variant->type->arrayRangeList);
	}
}

//对常量定义进行语义分析
void SemanticAnalyseConst(_Constant* constant){
	if(constant==NULL){
		cout << "[SemanticAnalyseConst] pointer of _Constant is null" << endl;
		return;
	}
    //用constId.first去查符号表，检查是否重定义 
    _SymbolRecord* record=findSymbolRecord(currentSymbolTable, constant->constId.first, 1);
	if (checkIsTheSameAsKey(constant->constId.first, constant->constId.second))
		return;
	if (record != NULL) {//重定义 checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, constant->constId.second);
		return;
	}
    if(constant->type=="id"){ //如果该常量由另外的常量标识符定义
        _SymbolRecord* preRecord=findSymbolRecord(currentSymbolTable, constant->valueId.first);
		if(preRecord==NULL){//未定义 checked
			addUndefinedErrorInformation(constant->valueId.first, constant->valueId.second);
			return;
		}
		if (preRecord->flag != "constant") {//如果不是常量 checked
			addPreFlagErrorInformation(constant->valueId.first, constant->valueId.second, "constant", preRecord->lineNumber, preRecord->flag);
			return;
		}
		currentSymbolTable->addConst(constant->constId.first, constant->constId.second, preRecord->type, constant->isMinusShow^preRecord->isMinusShow, preRecord->value);
    }
    else//该常量由常数值定义
        currentSymbolTable->addConst(constant->constId.first, constant->constId.second, constant->type, constant->isMinusShow, constant->strOfVal);
}

//对分程序进行语义分析
void SemanticAnalyseSubprogram(_SubProgram* subprogram){
	if(subprogram==NULL){
		cout << "[SemanticAnalyseSubprogram] pointer of _Subprogram is null" << endl;
		return;
	}
    for(int i=0;i<subprogram->constList.size();i++)
        SemanticAnalyseConst(subprogram->constList[i]);
    for(int i=0;i<subprogram->variantList.size();i++)
        SemanticAnalyseVariant(subprogram->variantList[i]);
	for (int i = 0; i < subprogram->subprogramDefinitionList.size(); i++) {
		SemanticAnalyseSubprogramDefinition(subprogram->subprogramDefinitionList[i]);
		currentSymbolTable = mainSymbolTable;//符号表重定位
	}
    SemanticAnalyseStatement(reinterpret_cast<_Statement*>(subprogram->compound));
}

string SemanticAnalyseFunctionCall(_FunctionCall *functionCall) {
	if(functionCall==NULL){
		cout << "[SemanticAnalyseFunctionCall] pointer of _FunctionCall is null" << endl;
		return "";
	}
	//www
	//_SymbolRecord *record = findSymbolRecord(currentSymbolTable, functionCall->functionId.first);
	_SymbolRecord *record = findSymbolRecord(mainSymbolTable, functionCall->functionId.first);
	if (record == NULL)
		record = findSymbolRecord(currentSymbolTable, functionCall->functionId.first, 1);
	if (record == NULL) {//未定义 checked
		addUndefinedErrorInformation(functionCall->functionId.first, functionCall->functionId.second);
		return functionCall->returnType = "error";
	}
	if (record->flag != "function") {//不是函数 checked
		addPreFlagErrorInformation(functionCall->functionId.first, functionCall->functionId.second, "function", record->lineNumber, record->flag);
		return functionCall->returnType = "error";
	}
	if (record->amount == -1) {//如果是变参函数（本编译器涉及的变参函数对参数类型没有要求，但不能为error）
		for (int i = 0; i < functionCall->actualParaList.size(); i++) 
			string actualType = SemanticAnalyseExpression(functionCall->actualParaList[i]);
		return functionCall->returnType = record->type;
	}
	if (functionCall->actualParaList.size() != record->amount) {//参数个数不一致 checked
		addNumberErrorInformation(functionCall->functionId.first, functionCall->functionId.second, int(functionCall->actualParaList.size()), record->amount, "function");
		return functionCall->returnType = record->type;
	}
	//检查各位置的实参和形参类型是否一致 形参在符号表中的定位
	for (int i = 0; i < functionCall->actualParaList.size(); i++) {
		string actualType = SemanticAnalyseExpression(functionCall->actualParaList[i]);
		string formalType = record->findXthFormalParaType(i + 1);
		bool isRefered = record->isXthFormalParaRefered(i + 1);
		if (isRefered && !(functionCall->actualParaList[i]->type == "var" && (functionCall->actualParaList[i]->variantReference->kind == "var" || functionCall->actualParaList[i]->variantReference->kind == "array"))) {
			//该表达式不能作为引用形参对应的实参 checked
			addGeneralErrorInformation("[Referenced actual parameter error!] <Line " + itos(functionCall->actualParaList[i]->lineNumber) + "> The " + itos(i + 1) + "th actual parameter expression should be a normal variable、value parameter、referenced parameter or array element.");
			continue;
		}
		if (!isRefered) { //传值参数支持从integer到real的隐式类型转换
			if (actualType != formalType && !(actualType == "integer" && formalType == "real")) //checked
				addExpressionTypeErrorInformation(functionCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of function call of \"" + functionCall->functionId.first + "\"");
		}
		else { //引用参数需保持类型强一致
			if (actualType != formalType) //checked
				addExpressionTypeErrorInformation(functionCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of function call of \"" + functionCall->functionId.first + "\"");
		}
	}
	return functionCall->returnType = record->type;
}

string SemanticAnalyseExpression(_Expression* expression) {
	if(expression==NULL){
		cout << "[SemanticAnalyseExpression] pointer of _Expression is null" << endl;
		return "";
	}
	if (expression->type == "var") { //获得普通变量/常量/数组的类型 //如果是integer类型的常量
		string variantReferenceType = SemanticAnalyseVariantReference(expression->variantReference);
		if (variantReferenceType == "integer" && expression->variantReference->kind == "constant") {//int类型的常量
			//查符号表查出常量值
			_SymbolRecord* record = findSymbolRecord(currentSymbolTable, expression->variantReference->variantId.first);
			if (record == NULL) {
				cout << "[SemanticAnalyseExpression] pointer of record is null" << endl;
				return "";
			}
			if (record->flag != "constant") {
				cout << "[SemanticAnalyseExpression] the record should be a constant" << endl;
				return "";
			}
			expression->totalIntValue = str2int(record->value);
			if (record->isMinusShow)
				expression->totalIntValue = -expression->totalIntValue;
			expression->totalIntValueValid = true;
		}
		return expression->expressionType = variantReferenceType;
	}
	else if (expression->type == "integer") {
		expression->totalIntValue = expression->intNum;
		expression->totalIntValueValid = true;
		return expression->expressionType = "integer";
	}
	else if (expression->type == "real")
		return expression->expressionType = "real";
	else if (expression->type == "char")
		return expression->expressionType = "char";
	else if (expression->type == "function") //获得函数调用的返回值类型
		return expression->expressionType = SemanticAnalyseFunctionCall(expression->functionCall);
	else if (expression->type == "compound") {
		if (expression->operationType == "relop") {
			string epType1 = SemanticAnalyseExpression(expression->operand1);
			string epType2 = SemanticAnalyseExpression(expression->operand2);
			if ((epType1 == epType2 && epType1 != "error") || (epType1 == "integer" && epType2 == "real") || (epType1 == "real" && epType2 == "integer"))
				return expression->expressionType = "boolean";
			else{
				if(epType1 != epType2 && epType1 != "error" && epType2 != "error") //checked
					addOperandExpressionsTypeMismatchErrorInformation(expression->operand1, expression->operand2);
				return expression->expressionType = "error";
			}
		}
		else if (expression->operation == "not") {
			string type = SemanticAnalyseExpression(expression->operand1);
			if (type == "boolean")
				return expression->expressionType = "boolean";
			else{
				if(type != "error" && type != "boolean") //checked
					addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand1, "boolean");
				return expression->expressionType = "error";
			}
		}
		else if (expression->operation == "minus") {
			string epType = SemanticAnalyseExpression(expression->operand1);
			if(epType == "integer" && expression->operand1->totalIntValueValid){
				expression->totalIntValue = - expression->operand1->totalIntValue;
				expression->totalIntValueValid = true;
			}
			if (epType == "integer" || epType == "real")
				return expression->expressionType = epType;
			else{
				if(epType != "error" && epType !="integer" && epType !="real") //checked
					addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand1, "integer or real");
				return expression->expressionType = "error";
			}
		}
		else if (expression->operation == "bracket"){
			expression->expressionType = SemanticAnalyseExpression(expression->operand1);
			if(expression->expressionType == "integer" && expression->operand1->totalIntValueValid){
				expression->totalIntValue = expression->operand1->totalIntValue;
				expression->totalIntValueValid = true;
			}
			return expression->expressionType;
		}
		else if (expression->operation == "+" || expression->operation == "-" || expression->operation == "*" || expression->operation == "/") {
			string epType1 = SemanticAnalyseExpression(expression->operand1);
			string epType2 = SemanticAnalyseExpression(expression->operand2);
			//checked
			if (expression->operation=="/" && epType2 == "integer" && expression->operand2->totalIntValueValid && expression->operand2->totalIntValue == 0)
				addDivideZeroErrorInformation(expression->operation, expression->operand2);
			if(epType1 == "integer" && epType2 == "integer" && expression->operand1->totalIntValueValid && expression->operand2->totalIntValueValid){
				expression->totalIntValueValid = true;
				if(expression->operation == "+")
					expression->totalIntValue = expression->operand1->totalIntValue + expression->operand2->totalIntValue;
				else if(expression->operation == "-")
					expression->totalIntValue = expression->operand1->totalIntValue - expression->operand2->totalIntValue;
				else if(expression->operation == "*")
					expression->totalIntValue = expression->operand1->totalIntValue * expression->operand2->totalIntValue;
				else 
					expression->totalIntValue = expression->operand1->totalIntValue / expression->operand2->totalIntValue;
			}
			if((epType1=="integer" || epType1=="real") && (epType2=="integer" || epType2=="real")){
				if(epType1=="integer" && epType2=="integer")
					return expression->expressionType="integer";
				return expression->expressionType="real";
			}
			if(epType1 != "error" && epType1 != "integer" && epType1 != "real") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand1, "integer or real");
			if(epType2 != "error" && epType2 != "integer" && epType2 != "real") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand2, "integer or real");
			return expression->expressionType = "error";
		}
		else if (expression->operation == "div" || expression->operation == "mod") {
			string epType1 = SemanticAnalyseExpression(expression->operand1);
			string epType2 = SemanticAnalyseExpression(expression->operand2);
			//checked
			if (epType2 == "integer" && expression->operand2->totalIntValueValid && expression->operand2->totalIntValue == 0)
				addDivideZeroErrorInformation(expression->operation, expression->operand2);
			if (epType1 == "integer" && epType2 == "integer"){
				if(expression->operand1->totalIntValueValid && expression->operand2->totalIntValueValid){
					if(expression->operation == "div")
						expression->totalIntValue = expression->operand1->totalIntValue / expression->operand2->totalIntValue;
					else
						expression->totalIntValue = expression->operand1->totalIntValue % expression->operand2->totalIntValue;
					expression->totalIntValueValid = true;
				}
				return expression->expressionType = "integer";
			}
			if(epType1!="error" && epType1!="integer") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand1, "integer");
			if(epType2!="error" && epType2!="integer") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand2, "integer");
			return expression->expressionType = "error";
		}
		else if (expression->operation == "and" || expression->operation == "or") {
			string epType1 = SemanticAnalyseExpression(expression->operand1);
			string epType2 = SemanticAnalyseExpression(expression->operand2);
			if (epType1 == "boolean" && epType2 == "boolean")
				return expression->expressionType = "boolean";
			if (epType1 != "error" && epType1 != "boolean") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand1, "boolean");
			if (epType2 != "error" && epType2 != "boolean") //checked
				addSingleOperandExpressionTypeMismatchErrorInformation(expression->operand2, "boolean");
			return expression->expressionType = "error";
		}
		else {
			cout << "[_Expression::SemanticAnalyseExpression] ERROR: operation not found" << endl;
			return "error";
		}
	}
	else {
		//cout << "[_Expression::SemanticAnalyseExpression] ERROR: expression type not found" << endl;
		return "error";
	}
}

//对程序进行语义分析
void SemanticAnalyseProgram(_Program *program) {
	if (program == NULL) {
		cout << "[SemanticAnalyseProgram] pointer of _Program is null" << endl;
		return;
	}
	//库函数名、主程序名、主程序参数，在检查是否重定义时，优先级按照前面列举的顺序，
	//即主程序名不能和库函数名，主程序参数不能和库函数名、主程序名同名
	//添加主程序名、行号、参数个数等信息
	set<string> lib;
	lib.insert("read");
	lib.insert("write");
	lib.insert("writeln");
	lib.insert("exit");
	if (lib.count(program->programId.first)) //checked
		addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(program->programId.second) + "> Name of program \"" + program->programId.first + "\" has been defined as a lib program.");
	mainSymbolTable->addProgramName(program->programId.first, program->programId.second, "procedure", int(program->paraList.size()), "");
	//将主程序的参数添加到主符号表中，flag定为"parameter of program"
	for (int i = 0; i < program->paraList.size(); i++) {//检查主程序参数是否和主程序名同名
		if (program->paraList[i].first == program->programId.first) //checked
			addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(program->programId.second) + "> parameter of program \"" + program->programId.first + "\" is the same as name of program.");
		else if (lib.count(program->paraList[i].first)) //checked
			addGeneralErrorInformation("[Dulicate defined error!] <Line " + itos(program->paraList[i].second) + "> parameter of program \"" + program->paraList[i].first + "\" has been defined as a lib program.");
		mainSymbolTable->addVoidPara(program->paraList[i].first, program->paraList[i].second);
	}
	//主符号表需提前加入read、write、exit等库函数
	//对于库函数来说，setProcedure的后三个参数,lineNumber,amount,subSymbolTable是没有用的
	//lineNumber=-1且subSymbolTable=NULL表示是库函数
	//amount=-1表示变参
	//添加read过程，该过程变参
	mainSymbolTable->addProcedure("read", -1, -1, NULL);
	//添加write过程，该过程变参
	mainSymbolTable->addProcedure("write", -1, -1, NULL);
	//添加writeln过程，该过程变参
	mainSymbolTable->addProcedure("writeln", -1, -1, NULL);
	//添加exit过程，该过程的参数个数需要分情况讨论，程序里会有特判，这里指定为0没有特殊含义
	mainSymbolTable->addProcedure("exit", -1, 0, NULL);
	SemanticAnalyseSubprogram(program->subProgram);
}

string itos(int num) {
	stringstream ssin;
	ssin << num;
	return ssin.str();
}

void addDuplicateDefinitionErrorInformation(string preId, int preLineNumber, string preFlag, string preType, int curLineNumber){
	string errorInformation = "[Duplicate defined error!] <Line " + itos(curLineNumber) + "> ";
	if (preLineNumber != -1)
		errorInformation += "\"" + preId + "\"" + " has already been defined as a " + preFlag + " at line " + itos(preLineNumber) + ".";
	else
		errorInformation += "\"" + preId + "\"" + " has already been defined as a lib program.";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addUndefinedErrorInformation(string id, int curLineNumber) {
	string errorInformation;
	errorInformation = "[Undefined identifier!] <Line " + itos(curLineNumber) + "> ";
	errorInformation += id + " has not been defined.";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addUsageTypeErrorInformation(string curId, int curLineNumber, string curType, string usage, string correctType) {
	string errorInformation;
	errorInformation = "[Usage type error!] <Line " + itos(curLineNumber) + "> ";
	errorInformation += "\"" + curId + "\"" + " used for " + usage + " should be " + correctType + " but not " + curType + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

//数组下标个数不匹配、函数或过程的实参和形参的个数不匹配
void addNumberErrorInformation(string curId, int curLineNumber, int curNumber, int correctNumber, string usage) {
	string errorInformation;
	if (usage == "array") {
		errorInformation += "[Array index number mismatch!] ";
		errorInformation += "<Line " + itos(curLineNumber) + "> ";
		errorInformation += "Array \"" + curId + "\"" + " should have " + itos(correctNumber) + " but not " + itos(curNumber) + " indices.";
	}
	else if (usage == "procedure") {
		errorInformation += "[Procedure parameter number mismatch!] ";
		errorInformation += "<Line " + itos(curLineNumber) + "> ";
		errorInformation += "Procedure \"" + curId + "\"" + " should have " + itos(correctNumber) + " but not " + itos(curNumber) + " parameters.";
	}
	else if (usage == "function") {
		errorInformation += "[Function parameter number mismatch!] ";
		errorInformation += "<Line " + itos(curLineNumber) + "> ";
		errorInformation += "Function \"" + curId + "\"" + " should have " + itos(correctNumber) + " but not " + itos(curNumber) + " parameters.";
	}
	else {
		cout << "[addNumberErrorInformation] usage error" << endl;
		return;
	}
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addPreFlagErrorInformation(string curId, int curLineNumber, string curFlag, int preLineNumber, string preFlag) {
	string errorInformation;
	errorInformation += "[Symbol kinds mismatch!] ";
	errorInformation += "<Line " + itos(curLineNumber) + "> ";
	errorInformation += "\"" + curId + "\"" + " defined at line " + itos(preLineNumber) + " is a " + preFlag + " but not a " + curFlag + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addExpressionTypeErrorInformation(_Expression *exp, string curType, string correctType, string description) {
	string errorInformation;
	errorInformation += "[Expression type error!] ";
	errorInformation += "<Line " + itos(exp->lineNumber) + "> ";
	string expression;
	inputExpression(exp, expression, 1);
	errorInformation += "Expression \"" + expression + "\" used for " + description + " should be " + correctType + " but not " + curType + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addAssignTypeMismatchErrorInformation(_VariantReference *leftVariantReference, _Expression *rightExpression) {
	string errorInformation;
	errorInformation += "[Assign statement type mismatch!] ";
	errorInformation += "<Left at line " + itos(leftVariantReference->variantId.second) + ", right at line " + itos(rightExpression->lineNumber) + "> ";
	string varRef, exp;
	inputVariantRef(leftVariantReference, varRef, 1);
	inputExpression(rightExpression, exp, 1);
	errorInformation += "Left \"" + varRef + "\" type is " + leftVariantReference->variantType + " while right \"" + exp + "\" type is " + rightExpression->expressionType + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addArrayRangeOutOfBoundErrorInformation(_Expression *expression, string arrayId, int X, pair<int,int> range){
	string errorInformation;
	errorInformation += "[Array range out of bound!] ";
	errorInformation += "<Line " + itos(expression->lineNumber) + "> ";
	string exp;
	inputExpression(expression, exp, 1);
	errorInformation += "The value of expression \"" + exp + "\"" + " is " + itos(expression->totalIntValue);
	errorInformation += ", but the range of array \"" + arrayId + "\" " + itos(X) + "th index is " + itos(range.first) + " to " + itos(range.second) + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}


void addArrayRangeUpSideDownErrorInformation(string curId, int curLineNumber, int X, int lowBound, int highBound) {
	string errorInformation;
	errorInformation += "[Array range upsidedown error!] ";
	errorInformation += "<Line " + itos(curLineNumber) + "> ";
	errorInformation += itos(X) + "th range of array \"" + curId + "\" have larger low bound and smaller high bound, which is " + itos(lowBound) + " and " + itos(highBound) + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addOperandExpressionsTypeMismatchErrorInformation(_Expression *exp1, _Expression *exp2){
	string errorInformation;
	errorInformation += "[Operands expression type mismatch!] ";
	errorInformation += "<Left at line " + itos(exp1->lineNumber) + ", right at line " + itos(exp2->lineNumber) + "> ";
	string expStr1, expStr2;
	inputExpression(exp1, expStr1, 1);
	inputExpression(exp2, expStr2, 1);
	errorInformation += "Left \"" + expStr1 + "\" type is " + exp1->expressionType + " while right " + "\"" + expStr2 + "\" type is " + exp2->expressionType + ".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addSingleOperandExpressionTypeMismatchErrorInformation(_Expression *exp, string correctType){
	string errorInformation;
	errorInformation += "[Operand expression type error!] ";
	errorInformation += "<Line " + itos(exp->lineNumber) + "> ";
	string expStr;
	inputExpression(exp, expStr, 1);
	errorInformation += "Expression \"" + expStr + "\" type should be " + correctType + " but not " + exp->expressionType + ".";
	semanticErrorInformation.push_back(errorInformation); 
	CHECK_ERROR_BOUND
}

void addactualParameterOfReadErrorInformation(int curLineNumber, string procedureId, int X, _Expression *exp) {
	string errorInformation;
	errorInformation += "[Actual parameter of read procedure type error!] ";
	errorInformation += "<Line " + itos(curLineNumber) + "> ";
	string expression;
	inputExpression(exp, expression, 1);
	errorInformation += "\"" + procedureId + "\" " + itos(X) + "th expression parameter \"" + expression + "\" is not a variant or an array element.";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addDivideZeroErrorInformation(string operation, _Expression *exp) {
	string errorInformation;
	errorInformation += "[Divide zero error!] ";
	errorInformation += "<Line " + itos(exp->lineNumber) + "> ";
	string expression;
	inputExpression(exp, expression, 1);
	errorInformation += "The value of expression \"" + expression + "\" is 0, which is the second operand of operation \"" + operation + "\".";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addReadBooleanErrorInformation(_Expression *exp, int X) {
	string errorInformation;
	string expression;
	inputExpression(exp, expression, 1);
	errorInformation = "[Read boolean error!] ";
	errorInformation += "<Line " + itos(exp->lineNumber) + "> ";
	errorInformation += "The " + itos(X) + "th actual parameter of read \"" + expression + "\" is boolean, it can't be read.";
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void addGeneralErrorInformation(string errorInformation) {
	semanticErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

bool checkIsTheSameAsKey(string id, int lineNumber) { //这里的Key指的是库程序名、主程序名、主程序参数
	for (int i = 0; i <= mainSymbolTable->recordList[0]->amount + 4; i++) {
		if (id == mainSymbolTable->recordList[i]->id) {
			if (i == 0) //与主程序名同名 checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as the name of program at Line " + itos(mainSymbolTable->recordList[i]->lineNumber) + ".");
			else if (i >= 1 && i <= mainSymbolTable->recordList[0]->amount) //与主程序参数同名 checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as a program parameter at Line " + itos(mainSymbolTable->recordList[i]->lineNumber) + ".");
			else //与库程序同名 checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as a lib program.");
			return true;
		}
	}
	return false;
}