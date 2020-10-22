#include "ASTnodes.h"
#include <iostream>
using namespace std;
//函数的返回值语句的存在性检查，只需要关注存在性，类型检查由SemanticAnalyse.cpp完成


extern vector<string> semanticWarningInformation;//存储警告信息的列表
extern string itos(int num);//将整数转化为字符串


							//检查当前语句是否是“返回值完备”的
bool returnExistedCheckStatement(_Statement *statementNode);
//检查函数定义的程序体语句列表是否存在“返回值完备”的语句
void returnExistedCheckMainCompound(_Compound *compoundNode, string functionId, int functionLineNumber);
//检查函数定义是否是“返回值完备”的
void returnExistedCheckFunctionDefinition(_FunctionDefinition* functionDefinitionNode);


bool returnExistedCheckStatement(_Statement *statementNode) {
	if (statementNode == NULL) {
		cout << "[returnExistedCheckStatement] pointer of _Statement is null" << endl;
		return false;
	}
	if (statementNode->type == "compound") {
		_Compound *compound = reinterpret_cast<_Compound*>(statementNode);
		for (int i = 0; i < compound->statementList.size(); i++) {
			if (returnExistedCheckStatement(compound->statementList[i]))
				return true;
		}
		return false;
	}
	else if (statementNode->type == "repeat") {
		_RepeatStatement *repeatStatement = reinterpret_cast<_RepeatStatement*>(statementNode);
		return returnExistedCheckStatement(repeatStatement->_do);
	}
	else if (statementNode->type == "while") {
		_WhileStatement *whileStatement = reinterpret_cast<_WhileStatement*>(statementNode);
		return returnExistedCheckStatement(whileStatement->_do);
	}
	else if (statementNode->type == "for") {
		_ForStatement *forStatement = reinterpret_cast<_ForStatement*>(statementNode);
		return returnExistedCheckStatement(forStatement->_do);
	}
	else if (statementNode->type == "if") {
		_IfStatement *ifStatement = reinterpret_cast<_IfStatement*>(statementNode);
		if (ifStatement->then == NULL) {
			cout << "[returnExistedCheckStatement][IfStatement] the pointer of then statement is null";
			return false;
		}
		if (ifStatement->els == NULL)
			return false;
		else
			return returnExistedCheckStatement(ifStatement->then) && returnExistedCheckStatement(ifStatement->els);
	}
	else if (statementNode->type == "assign") { //判断是否是返回值语句
		_AssignStatement *assignStatement = reinterpret_cast<_AssignStatement*>(statementNode);
		return assignStatement->isReturnStatement; //在之前的语义分析中，已经保存是否是返回值语句
												   //即fun:=expression的情况
	}
	else if (statementNode->type == "procedure") { //判断是否是返回值语句
		_ProcedureCall *procedureCall = reinterpret_cast<_ProcedureCall*>(statementNode);
		return procedureCall->isReturnStatement; //在之前的语义分析中，已经保存是否是返回值语句
												 //即exit()的情况
	}
	else {
		cout << "[returnExistedCheckStatement] statement type error" << endl;
		return false;
	}
}

void returnExistedCheckMainCompound(_Compound *compoundNode, string functionId, int functionLineNumber) {
	if (compoundNode == NULL) {
		cout << "[returnExistedCheckCompound] pointer of _Compound is null" << endl;
		return;
	}
	bool ok = false;
	for (int i = 0; i < compoundNode->statementList.size() && !ok; i++) {
		if (returnExistedCheckStatement(compoundNode->statementList[i]))
			ok = true;
	}
	if (!ok) {
		string warningInformation;
		warningInformation = "[Return value statement missing!] ";
		warningInformation += "<Line " + itos(functionLineNumber) + "> ";
		warningInformation += "Incomplete return value statement of function \"" + functionId + "\".";
		semanticWarningInformation.push_back(warningInformation);
	}
}

void returnExistedCheckFunctionDefinition(_FunctionDefinition* functionDefinitionNode) {
	if (functionDefinitionNode == NULL) {
		cout << "[returnExistedCheckFunction] pointer of _FunctionDefinition is null" << endl;
		return;
	}
	returnExistedCheckMainCompound(functionDefinitionNode->compound, functionDefinitionNode->functionID.first, functionDefinitionNode->functionID.second);
}