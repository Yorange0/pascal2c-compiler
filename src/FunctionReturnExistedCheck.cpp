#include "ASTnodes.h"
#include <iostream>
using namespace std;
//�����ķ���ֵ���Ĵ����Լ�飬ֻ��Ҫ��ע�����ԣ����ͼ����SemanticAnalyse.cpp���


extern vector<string> semanticWarningInformation;//�洢������Ϣ���б�
extern string itos(int num);//������ת��Ϊ�ַ���


							//��鵱ǰ����Ƿ��ǡ�����ֵ�걸����
bool returnExistedCheckStatement(_Statement *statementNode);
//��麯������ĳ���������б��Ƿ���ڡ�����ֵ�걸�������
void returnExistedCheckMainCompound(_Compound *compoundNode, string functionId, int functionLineNumber);
//��麯�������Ƿ��ǡ�����ֵ�걸����
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
	else if (statementNode->type == "assign") { //�ж��Ƿ��Ƿ���ֵ���
		_AssignStatement *assignStatement = reinterpret_cast<_AssignStatement*>(statementNode);
		return assignStatement->isReturnStatement; //��֮ǰ����������У��Ѿ������Ƿ��Ƿ���ֵ���
												   //��fun:=expression�����
	}
	else if (statementNode->type == "procedure") { //�ж��Ƿ��Ƿ���ֵ���
		_ProcedureCall *procedureCall = reinterpret_cast<_ProcedureCall*>(statementNode);
		return procedureCall->isReturnStatement; //��֮ǰ����������У��Ѿ������Ƿ��Ƿ���ֵ���
												 //��exit()�����
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