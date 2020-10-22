/*
�������ʵ�ִ���
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

extern _SymbolTable *mainSymbolTable;//�����ű�
extern _SymbolTable *currentSymbolTable;//��ǰ���ű�
extern _SymbolRecord* findSymbolRecord(_SymbolTable* currentSymbolTable, string id, int mode=0);//�ӷ��ű����ҳ�id��Ӧ�ļ�¼
extern void inputFunctionCall(_FunctionCall *functionCallNode, string &functionCall, int mode=0);//��ȡ��������
extern int inputExpression(_Expression *expressionNode, string &expression, int mode=0, bool isReferedActualPara=false);//��ȡ���ʽ
extern void inputVariantRef(_VariantReference *variantRefNode, string &variantRef, int mode=0, bool isReferedActualPara=false);//��ȡ��������
extern int str2int(string str);
extern void returnExistedCheckFunctionDefinition(_FunctionDefinition* functionDefinitionNode);

vector<string> semanticErrorInformation;//�洢������Ϣ���б�
vector<string> semanticWarningInformation;//�洢������Ϣ���б�

void SemanticAnalyse(_Program *ASTRoot);

void createSymbolTableAndInit();//���������ű���ʼ��
void createSubSymbolTableAndInit();//�����ӷ��ű���ʼ��

string SemanticAnalyseVariantReference(_VariantReference* variantReference);//�Ա������ý����������
void SemanticAnalyseStatement(_Statement *statement);//���������������
void SemanticAnalyseSubprogramDefinition(_FunctionDefinition* functionDefinition);//���ӳ���������������
void SemanticAnalyseVariant(_Variant* variant);//�Ա�����������������
void SemanticAnalyseConst(_Constant* constant);//�Գ�����������������
void SemanticAnalyseSubprogram(_SubProgram* subprogram);//�Էֳ�������������
void SemanticAnalyseProgram(_Program *program);//�Գ�������������
void SemanticAnalyseFormalParameter(_FormalParameter *formalParameter);//����ʽ���������������
string SemanticAnalyseFunctionCall(_FunctionCall *functionCall);//�Ժ������ý����������
string SemanticAnalyseExpression(_Expression* expression);//�Ա��ʽ�����������

string itos(int num);//��intת��Ϊstring

//����ض��������Ϣ
void addDuplicateDefinitionErrorInformation(string preId, int preLineNumber, string preFlag, string preType,int curLineNumber);//����ظ���������������Ϣ
//���δ���������Ϣ
void addUndefinedErrorInformation(string id, int curLineNumber);
//��ӱ�ʶ�����ʹ�����Ϣ
void addUsageTypeErrorInformation(string curId, int curLineNumber, string curType, string usage, string correctType);
//��Ӹ�����ƥ�������Ϣ
void addNumberErrorInformation(string curId, int curLineNumber, int curNumber, int correctNumber, string usage);
//��ӱ�ʶ�����������Ϣ
void addPreFlagErrorInformation(string curId, int curLineNumber, string curFlag, int preLineNumber, string preFlag);
//��ӱ��ʽ���ʹ�����Ϣ
void addExpressionTypeErrorInformation(_Expression *exp, string curType, string correctType, string description);
//��Ӹ�ֵ�����ֵ����ֵ���Ͳ�ƥ�������Ϣ
void addAssignTypeMismatchErrorInformation(_VariantReference *leftVariantReference, _Expression *rightExpression);
//��������±�Խ�������Ϣ
void addArrayRangeOutOfBoundErrorInformation(_Expression *expression, string arrayId, int X, pair<int,int> range);
//��������½���Ͻ��Ĵ�����Ϣ
void addArrayRangeUpSideDownErrorInformation(string curId, int curLineNumber, int X, int lowBound, int highBound);
//�����������ߵĲ��������Ͳ�һ�µĴ�����Ϣ
void addOperandExpressionsTypeMismatchErrorInformation(_Expression *exp1, _Expression *exp2);
//���ĳ�����������ʹ������Ϣ
void addSingleOperandExpressionTypeMismatchErrorInformation(_Expression *exp, string correctType);
//���read��ʵ�δ�����Ϣ
void addactualParameterOfReadErrorInformation(int curLineNumber, string procedureId, int X, _Expression *exp);
//��ӳ�0������Ϣ
void addDivideZeroErrorInformation(string operation, _Expression *exp);
//���read��ȡboolean���ͱ����������Ϣ
void addReadBooleanErrorInformation(_Expression *exp, int X);
//��������Ϣֱ����ӵ�������Ϣ���б���
void addGeneralErrorInformation(string errorInformation);

//���id�Ƿ������������������������������ͬ�� ���Ҳ�����������ű�ӿ�
bool checkIsTheSameAsKey(string id, int lineNumber); //�����Keyָ���ǿ���������������������������

void SemanticAnalyse(_Program *ASTRoot) {
	createSymbolTableAndInit();
	SemanticAnalyseProgram(ASTRoot);
}

void createSymbolTableAndInit() {//���������ű�
	currentSymbolTable = mainSymbolTable = new _SymbolTable("main");//ָ��Ϊ�����ű�󣬻��Զ�����read, write�ȿ⺯��
}

void createSubSymbolTableAndInit(){//�����ӷ��ű� ���ű�λ
	currentSymbolTable = new _SymbolTable("sub");//��������λ���ӷ��ű�
}

//�Ա������ý����������
//����������Ϊ��ֵ�������Ǵ�ֵ���������ò�������ͨ����������Ԫ�ء�����������������ֵ��
//����������Ϊ��ֵ�������Ǵ�ֵ���������ò�������ͨ����������Ԫ�ء������������������ĺ�����
string SemanticAnalyseVariantReference(_VariantReference* variantReference){
	if(variantReference==NULL){
		cout << "[SemanticAnalyseVariantReference] pointer of _VariantReference is null" << endl;
		return "";
	}
	_SymbolRecord* record = findSymbolRecord(currentSymbolTable, variantReference->variantId.first);
	if (record == NULL) {//δ���� //checked
		addUndefinedErrorInformation(variantReference->variantId.first, variantReference->variantId.second);
		return variantReference->variantType = "error";//�޷��ҵ��������õ�����
	}
	//���ȱ����Ǻ���
	//��Ϊ��ֵ�����ǵ�ǰ������
	//��Ϊ��ֵ����������⺯�������ݹ����Ҳ���ԣ�
	//����ֵ������ֵ���ǹ��̻��Ǻ������ǵ�ǰ���ű��Ӧ���ӳ������ơ����Ǳ���ӳ�������

	if (variantReference->flag == 0) {//����Ƿ�����
		if (record->flag == "(sub)program name") {
			if (record->subprogramType == "procedure") {//��������˲鵽�������������
				//�����пյĻ�����ðѲ鵽���������ı������ó��� hhh checked
				addGeneralErrorInformation("[Invalid reference] <Line " + itos(variantReference->variantId.second) + "> Procedure name \"" + record->id + "\" can't be referenced");
				return variantReference->variantType = "error";
			}
			//����Ǻ�������ôһ���ǵ�ǰ���ű�
			//�������ֵ����ô�Ƿ�����䣬����Ҫ��ע����
			//�������ֵ����ô�ǵݹ���ã���Ҫ��ע�������������ʽ���������Ƿ���0��
			if (variantReference->locFlag == -1) { //�������ֵ
				variantReference->kind = "function return reference";
				return variantReference->variantType = record->type;
			}
			//�������ֵ
			if (record->amount != 0) {//����βθ�����Ϊ0 checked �ݹ����
				addNumberErrorInformation(variantReference->variantId.first, variantReference->variantId.second, 0, record->amount, "function");
				return variantReference->variantType = record->type;
			}
			//����βθ���Ϊ0
			//������Ӧ����������У�ʵ�������޲κ����ĵݹ����
			variantReference->kind = "function call";
			return variantReference->variantType = record->type;
		}
		if (record->flag == "function") {//����Ǻ��� ��һ�����������ű��в鵽��
			variantReference->kind = "function";
			//������Ϊ��ֵ��������Ϊ��ֵ�����βθ�������Ϊ0
			//��ʶ��ΪvariantReference�ĺ�������һ������ʵ�Σ�������Ҫ����βθ���
			if (variantReference->locFlag == -1) {//�������ֵ checked
				addGeneralErrorInformation("[Invalid reference!] <Line " + itos(variantReference->variantId.second) + "> function name \"" + record->id + "\" can't be referenced as l-value.");
				return variantReference->variantType = "error";
			}
			//�������ֵ
			if (record->amount != 0) {//����βθ�����Ϊ0 checked
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
	else if (variantReference->flag == 1) {//���������
		if (record->flag != "array") {//������ű��м�¼�Ĳ������� checked
			addPreFlagErrorInformation(variantReference->variantId.first, variantReference->variantId.second, "array", record->lineNumber, record->flag);
			return variantReference->variantType = "error";
		}
		variantReference->kind = "array";
		//checked
		if (variantReference->expressionList.size() != record->amount) {//�������ʱ���±�ά���ͷ��ű����治һ��
			addNumberErrorInformation(variantReference->variantId.first, variantReference->variantId.second, int(variantReference->expressionList.size()), record->amount, "array");
			variantReference->variantType = "error";
			return record->type;
		}
		variantReference->variantType = record->type;
		for (int i = 0; i < variantReference->expressionList.size(); i++) {
			string type = SemanticAnalyseExpression(variantReference->expressionList[i]);
			//����±���ʽ�������Ƿ������� checked
			if (type != "integer") {
				addExpressionTypeErrorInformation(variantReference->expressionList[i], type, "integer", itos(i + 1) + "th index of array \"" + variantReference->variantId.first + "\"");
				variantReference->variantType = "error";
			}
			//���Խ�� checked
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

//���������������
void SemanticAnalyseStatement(_Statement *statement){
	if(statement==NULL){
		cout << "[SemanticAnalyseStatement] pointer of _Statement is null" << endl;
		return;
	}
    if(statement->type=="compound"){
        _Compound *compound = reinterpret_cast<_Compound*>(statement);//�Ը��������е�ÿһ���������������
        for(int i=0;i<compound->statementList.size();i++)
            SemanticAnalyseStatement(compound->statementList[i]);
    }
    else if(statement->type=="repeat"){
        _RepeatStatement *repeatStatement = reinterpret_cast<_RepeatStatement*>(statement);
		string type = SemanticAnalyseExpression(repeatStatement->condition);
		if (type != "boolean") {//repeat������ͼ��,condition���ʽ���ͼ�� checked
			addExpressionTypeErrorInformation(repeatStatement->condition, type, "boolean", "condition of repeat-until statement");
			repeatStatement->statementType = "error";
		}
		else
            repeatStatement->statementType="void";
        SemanticAnalyseStatement(repeatStatement->_do);//��ѭ�����������������
    }
    else if(statement->type=="while"){
        _WhileStatement *whileStatement = reinterpret_cast<_WhileStatement*>(statement);
		string type = SemanticAnalyseExpression(whileStatement->condition);
		if (type != "boolean") {//while������ͼ��,condition���ʽ���ͼ�� checked
			addExpressionTypeErrorInformation(whileStatement->condition, type, "boolean", "condition of while statement");
			whileStatement->statementType = "error";
		}
		else
            whileStatement->statementType="void";
        SemanticAnalyseStatement(whileStatement->_do);//��ѭ�����������������
    }
    else if(statement->type=="for"){
        _ForStatement *forStatement = reinterpret_cast<_ForStatement*>(statement);
        //���ѭ�������Ƿ��Ѿ����壬���Ѿ����壬�Ƿ�Ϊinteger�ͱ���
		_SymbolRecord *record = findSymbolRecord(currentSymbolTable, forStatement->id.first);
		if(record==NULL){//ѭ������δ���壬������Ϣ checked
			addUndefinedErrorInformation(forStatement->id.first, forStatement->id.second);
			return;
		}
		//����޷���Ϊѭ������ checked
		if (!(record->flag == "value parameter" || record->flag == "var parameter" || record->flag == "normal variant")) {//�����ǰ�������಻������Ϊѭ������
			addPreFlagErrorInformation(forStatement->id.first, forStatement->id.second, "value parameter, var parameter or normal variant", record->lineNumber, record->flag);
			return;
		}
		//������Ͳ������� checked
		if (record->type != "integer") {
			addUsageTypeErrorInformation(forStatement->id.first, forStatement->id.second, record->type, "cyclic variable of for statement", "integer");
			return;
		}
        //for������ͼ��,start��end���ʽ���ͼ��
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
        //��ѭ�����������������
        SemanticAnalyseStatement(forStatement->_do);
    }
    else if(statement->type=="if"){
        _IfStatement *ifStatement = reinterpret_cast<_IfStatement*>(statement);
		string type = SemanticAnalyseExpression(ifStatement->condition);
		if (type != "boolean") {//if������ͼ��,condition���ʽ���ͼ�� checked
			addExpressionTypeErrorInformation(ifStatement->condition, type, "boolean", "condition of if statement");
			ifStatement->statementType = "error";
		}
		else
            ifStatement->statementType="void";
        SemanticAnalyseStatement(ifStatement->then);//��then�������������
        if(ifStatement->els!=NULL)//��else������������
            SemanticAnalyseStatement(ifStatement->els);
    }
    else if(statement->type=="assign"){//��ֵ����
        _AssignStatement *assignStatement = reinterpret_cast<_AssignStatement*>(statement);
        //����ֵ�������ý����������,���leftType
		assignStatement->statementType = "void";
		assignStatement->variantReference->locFlag = -1;//���Ϊ��ֵ
        string leftType = SemanticAnalyseVariantReference(assignStatement->variantReference);
		if (assignStatement->variantReference->kind == "constant") {
			//��ֵ����Ϊ���� checked
			addGeneralErrorInformation("[Constant as l-value error!] <Line" + itos(assignStatement->variantReference->variantId.second) + "> Costant \"" + assignStatement->variantReference->variantId.first + "\" can't be referenced as l-value.");
			return;
		}
		//����ֵ���ʽ�������ͼ��,���rightType
        string rightType = SemanticAnalyseExpression(assignStatement->expression);
		if (assignStatement->variantReference->kind == "function return reference") {//����Ƿ���ֵ���
			//���鷵��ֵ���ʽ�Ƿ�ͺ�������ֵ����һ��
			if (assignStatement->variantReference->variantType != rightType && !(assignStatement->variantReference->variantType=="real" and rightType=="integer")) {
				//checked
				addGeneralErrorInformation("[Return type of funciton mismatch!] <Line " + itos(assignStatement->expression->lineNumber) + "> The type of return expression is " + rightType + " ,but not " + assignStatement->variantReference->variantType + " as function \"" + assignStatement->variantReference->variantId.first + "\" defined.");
				assignStatement->statementType = "error";
			}
			assignStatement->isReturnStatement = true;
			return;
		}
        //�Ƚ���ֵ����ֵ����,��ø�ֵ�������ͣ����Ͳ�ͬʱ��ֻ֧�����͵�ʵ�͵���ʽת��
		if (leftType != rightType && !(leftType=="real" && rightType=="integer")) {
			//checked
			addAssignTypeMismatchErrorInformation(assignStatement->variantReference, assignStatement->expression);
			assignStatement->statementType = "error";
		}
		else
			assignStatement->statementType = "void";
    }
    else if(statement->type=="procedure"){//read�Ĳ���ֻ���Ǳ���������Ԫ��;
        _ProcedureCall *procedureCall = reinterpret_cast<_ProcedureCall*>(statement);
        //ͨ��procedureId�����ò����������������͵���Ϣ
		//www
		//_SymbolRecord *record = findSymbolRecord(currentSymbolTable, procedureCall->procedureId.first);
		_SymbolRecord *record = findSymbolRecord(mainSymbolTable, procedureCall->procedureId.first);
		if (record == NULL)
			record = findSymbolRecord(currentSymbolTable, procedureCall->procedureId.first, 1);
		procedureCall->statementType = "void";
		if (record == NULL) {//δ���� checked
			addUndefinedErrorInformation(procedureCall->procedureId.first, procedureCall->procedureId.second);
			procedureCall->statementType = "error";
			return;
		}
		if (record->flag != "procedure") {//������ǹ��� checked
			addPreFlagErrorInformation(procedureCall->procedureId.first, procedureCall->procedureId.second, "procedure", record->lineNumber, record->flag);
			procedureCall->statementType = "error";
			return;
		}
		if (record->id == "exit") {
			/*exit�ȿ��Գ����ڹ����У�Ҳ���Գ����ں����У������ڹ�����ʱ��
			exit���ܴ������������ں�����ʱ��exitֻ�ܴ�һ��������
			�Ҹò������ʽ�����ͱ���ͺ����ķ���ֵ����һ��*/
			//�������жϵ�ǰ�����ǹ��̻��Ǻ���
			if (currentSymbolTable->recordList[0]->subprogramType == "procedure") {//����ǹ���
				//exit���ܴ��������ʽ
				if (procedureCall->actualParaList.size() != 0) {//���ʵ�θ�����Ϊ0 checked
					addGeneralErrorInformation("[Return value redundancy!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of procedure must be 0, that is, exit must have no actual parameters.");
					procedureCall->statementType = "error";
				}
				return;
			}
			//����Ǻ���
			if (procedureCall->actualParaList.size() != 1) {//���ʵ�θ�����Ϊ1
				if (procedureCall->actualParaList.size() == 0) //checked
					addGeneralErrorInformation("[Return value missing!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of function must be 1, that is, exit must have 1 actual parameters.");
				else //checked
					addGeneralErrorInformation("[Return value redundancy!] <Line " + itos(procedureCall->procedureId.second) + "> Number of return value of function must be 1, that is, exit must have 1 actual parameters.");
				return;
			}
			//���ʵ�θ���Ϊ1�����ʵ�α��ʽ�����ͣ�����Ƿ��뺯������ֵ����һ��
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
			if (procedureCall->actualParaList.size() == 0) { //read��write�Ĳ�����������Ϊ0 checked
				string tmp = record->id;
				tmp[0] -= 'a' - 'A';
				addGeneralErrorInformation("[" + tmp + " actual parameter missing!] <Line " + itos(procedureCall->procedureId.second) + "> procedure \"" + record->id + "\" must have at least one actual parameter.");
				procedureCall->statementType = "error";
			}
		}
		if (record->id == "read") {//����ֻ���Ǳ���������Ԫ�أ������ǳ��������ʽ��
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
		if (record->amount == -1) {//����Ǳ�ι��̣����������漰�ı�ι���(����read)�Բ�������û��Ҫ�󣬵�����Ϊerror��
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
		// �β��ڷ��ű��еĶ�λ
		for (int i = 0; i < procedureCall->actualParaList.size(); i++) {//���actualParaList�����ʽ�����ͣ����ʵ�κ��βε�����һ����
			string actualType = SemanticAnalyseExpression(procedureCall->actualParaList[i]);
			string formalType = record->findXthFormalParaType(i + 1);
			bool isRefered = record->isXthFormalParaRefered(i + 1);
			if (isRefered && !(procedureCall->actualParaList[i]->type == "var" && (procedureCall->actualParaList[i]->variantReference->kind == "var" || procedureCall->actualParaList[i]->variantReference->kind == "array"))) {
				//�ñ��ʽ������Ϊ�����βζ�Ӧ��ʵ�� checked
				addGeneralErrorInformation("[Referenced actual parameter error!] <Line " + itos(procedureCall->actualParaList[i]->lineNumber) + "> The " + itos(i + 1) + "th actual parameter expression should be a normal variable��value parameter��referenced parameter or array element.");
				continue;
			}
			//if(isRefered && procedureCall->actualParaList[i]->type==)
			if (!isRefered) { //��ֵ����֧��integer��real����ʽ����ת��
				if (actualType != formalType && !(actualType == "integer" && formalType == "real")) { //������Ͳ�һ��
				    //checked
					addExpressionTypeErrorInformation(procedureCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of procedure call of \"" + procedureCall->procedureId.first + "\"");
					procedureCall->statementType = "error";
				}
			}
			else { //���ò����豣������ǿһ��
				if (actualType != formalType) { //������Ͳ�һ��
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

//����ʽ�������������������ʽ����һ���ǻ�������
void SemanticAnalyseFormalParameter(_FormalParameter* formalParameter){
	if(formalParameter==NULL){
		cout << "[SemanticAnalyseFormalParameter] pointer of _FormalParameter is null" << endl;
		return;
	}
	//����Ƿ������������������������������ͬ��
	//if (checkIsTheSameAsKey(formalParameter->paraId.first, formalParameter->paraId.second))
		//return;
	_SymbolRecord *record = findSymbolRecord(currentSymbolTable, formalParameter->paraId.first, 1);
	if (!checkIsTheSameAsKey(formalParameter->paraId.first, formalParameter->paraId.second)) {
		if (record != NULL) //����ض���
			addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, formalParameter->paraId.second);
	}
	//����Ƿ��뵱ǰ�������Լ�����֮ǰ������β�ͬ�������ͬ��������»��߽��лָ�����add�����н��У�
	if(formalParameter->flag==0)
		currentSymbolTable->addPara(formalParameter->paraId.first, formalParameter->paraId.second, formalParameter->type);
	else
		currentSymbolTable->addVarPara(formalParameter->paraId.first, formalParameter->paraId.second, formalParameter->type);
}

//���ӳ���������������
void SemanticAnalyseSubprogramDefinition(_FunctionDefinition* functionDefinition){
	if(functionDefinition==NULL){
		cout << "[SemanticAnalyseSubprogramDefinition] pointer of _FunctionDefinition is null" << endl;
		return;
	}
	_SymbolRecord *record=findSymbolRecord(currentSymbolTable, functionDefinition->functionID.first, 1);
	if(record!=NULL){//�ض��� checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, functionDefinition->functionID.second);
		return;
	}
	string subprogramType;
	if (functionDefinition->type.first == "")
		subprogramType = "procedure";
	else
		subprogramType = "function";
    createSubSymbolTableAndInit();//��������λ���ӱ�
	//���ӳ���������Ϣ��ӵ��ӷ��ű���
	currentSymbolTable->addProgramName(functionDefinition->functionID.first, functionDefinition->functionID.second, subprogramType, int(functionDefinition->formalParaList.size()), functionDefinition->type.first);
	
	//����type�Ƿ�ΪNULL����ΪaddProcedure()��addFunction()����ӵ����������
	if (functionDefinition->type.first=="")//����ǹ���
		mainSymbolTable->addProcedure(functionDefinition->functionID.first, functionDefinition->functionID.second, int(functionDefinition->formalParaList.size()), currentSymbolTable);
	else//����Ǻ���
		mainSymbolTable->addFunction(functionDefinition->functionID.first, functionDefinition->functionID.second, functionDefinition->type.first, int(functionDefinition->formalParaList.size()), currentSymbolTable);
	
	//����ʽ�����б�������������������ʽ������ӵ��ӷ��ű���
	for(int i=0;i<functionDefinition->formalParaList.size();i++)
		SemanticAnalyseFormalParameter(functionDefinition->formalParaList[i]);
	//�Գ�����������������
	for (int i = 0; i<functionDefinition->constList.size(); i++)
		SemanticAnalyseConst(functionDefinition->constList[i]);
    //�Ա�����������������
	for (int i = 0; i<functionDefinition->variantList.size(); i++)
		SemanticAnalyseVariant(functionDefinition->variantList[i]);
    //��compound�����������
	SemanticAnalyseStatement(reinterpret_cast<_Statement*>(functionDefinition->compound));
	//�Ժ������з���ֵ���Ĵ����Լ��
	if (functionDefinition->type.first != "") //checked
		returnExistedCheckFunctionDefinition(functionDefinition);
}

//�Ա�����������������
void SemanticAnalyseVariant(_Variant* variant){
	if(variant==NULL){
		cout << "[SemanticAnalyseVariant] pointer of _Variant is null" << endl;
		return;
	}
    _SymbolRecord* record=findSymbolRecord(currentSymbolTable, variant->variantId.first, 1);//��variantId.firstȥ����ű�����Ƿ��ض���
	if (checkIsTheSameAsKey(variant->variantId.first, variant->variantId.second))
		return;
	if (record != NULL){//����ض��� checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, variant->variantId.second);
		return;
	}
	if(variant->type->flag==0)//�������ͨ����
		currentSymbolTable->addVar(variant->variantId.first, variant->variantId.second, variant->type->type.first);
	else {//���������
		//���鶨��ʱ�����½���������Ͻ������ڵ����½磻�����ķ����壬���½��Ϊ�޷���������ͨ�����﷨��������һ�����޷�����
		vector< pair<int, int> > &tmp = variant->type->arrayRangeList;
		for (int i = 0; i < tmp.size(); i++) {
			if (tmp[i].first > tmp[i].second) { //checked
				addArrayRangeUpSideDownErrorInformation(variant->variantId.first, variant->variantId.second, i + 1, tmp[i].first, tmp[i].second);
				tmp[i].second = tmp[i].first; //����Ͻ�С���½磬���Ͻ�����Ϊ�½�
			}
		}
		currentSymbolTable->addArray(variant->variantId.first, variant->variantId.second, variant->type->type.first, int(variant->type->arrayRangeList.size()), variant->type->arrayRangeList);
	}
}

//�Գ�����������������
void SemanticAnalyseConst(_Constant* constant){
	if(constant==NULL){
		cout << "[SemanticAnalyseConst] pointer of _Constant is null" << endl;
		return;
	}
    //��constId.firstȥ����ű�����Ƿ��ض��� 
    _SymbolRecord* record=findSymbolRecord(currentSymbolTable, constant->constId.first, 1);
	if (checkIsTheSameAsKey(constant->constId.first, constant->constId.second))
		return;
	if (record != NULL) {//�ض��� checked
		addDuplicateDefinitionErrorInformation(record->id, record->lineNumber, record->flag, record->type, constant->constId.second);
		return;
	}
    if(constant->type=="id"){ //����ó���������ĳ�����ʶ������
        _SymbolRecord* preRecord=findSymbolRecord(currentSymbolTable, constant->valueId.first);
		if(preRecord==NULL){//δ���� checked
			addUndefinedErrorInformation(constant->valueId.first, constant->valueId.second);
			return;
		}
		if (preRecord->flag != "constant") {//������ǳ��� checked
			addPreFlagErrorInformation(constant->valueId.first, constant->valueId.second, "constant", preRecord->lineNumber, preRecord->flag);
			return;
		}
		currentSymbolTable->addConst(constant->constId.first, constant->constId.second, preRecord->type, constant->isMinusShow^preRecord->isMinusShow, preRecord->value);
    }
    else//�ó����ɳ���ֵ����
        currentSymbolTable->addConst(constant->constId.first, constant->constId.second, constant->type, constant->isMinusShow, constant->strOfVal);
}

//�Էֳ�������������
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
		currentSymbolTable = mainSymbolTable;//���ű��ض�λ
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
	if (record == NULL) {//δ���� checked
		addUndefinedErrorInformation(functionCall->functionId.first, functionCall->functionId.second);
		return functionCall->returnType = "error";
	}
	if (record->flag != "function") {//���Ǻ��� checked
		addPreFlagErrorInformation(functionCall->functionId.first, functionCall->functionId.second, "function", record->lineNumber, record->flag);
		return functionCall->returnType = "error";
	}
	if (record->amount == -1) {//����Ǳ�κ��������������漰�ı�κ����Բ�������û��Ҫ�󣬵�����Ϊerror��
		for (int i = 0; i < functionCall->actualParaList.size(); i++) 
			string actualType = SemanticAnalyseExpression(functionCall->actualParaList[i]);
		return functionCall->returnType = record->type;
	}
	if (functionCall->actualParaList.size() != record->amount) {//����������һ�� checked
		addNumberErrorInformation(functionCall->functionId.first, functionCall->functionId.second, int(functionCall->actualParaList.size()), record->amount, "function");
		return functionCall->returnType = record->type;
	}
	//����λ�õ�ʵ�κ��β������Ƿ�һ�� �β��ڷ��ű��еĶ�λ
	for (int i = 0; i < functionCall->actualParaList.size(); i++) {
		string actualType = SemanticAnalyseExpression(functionCall->actualParaList[i]);
		string formalType = record->findXthFormalParaType(i + 1);
		bool isRefered = record->isXthFormalParaRefered(i + 1);
		if (isRefered && !(functionCall->actualParaList[i]->type == "var" && (functionCall->actualParaList[i]->variantReference->kind == "var" || functionCall->actualParaList[i]->variantReference->kind == "array"))) {
			//�ñ��ʽ������Ϊ�����βζ�Ӧ��ʵ�� checked
			addGeneralErrorInformation("[Referenced actual parameter error!] <Line " + itos(functionCall->actualParaList[i]->lineNumber) + "> The " + itos(i + 1) + "th actual parameter expression should be a normal variable��value parameter��referenced parameter or array element.");
			continue;
		}
		if (!isRefered) { //��ֵ����֧�ִ�integer��real����ʽ����ת��
			if (actualType != formalType && !(actualType == "integer" && formalType == "real")) //checked
				addExpressionTypeErrorInformation(functionCall->actualParaList[i], actualType, formalType, itos(i + 1) + "th actual parameter of function call of \"" + functionCall->functionId.first + "\"");
		}
		else { //���ò����豣������ǿһ��
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
	if (expression->type == "var") { //�����ͨ����/����/��������� //�����integer���͵ĳ���
		string variantReferenceType = SemanticAnalyseVariantReference(expression->variantReference);
		if (variantReferenceType == "integer" && expression->variantReference->kind == "constant") {//int���͵ĳ���
			//����ű�������ֵ
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
	else if (expression->type == "function") //��ú������õķ���ֵ����
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

//�Գ�������������
void SemanticAnalyseProgram(_Program *program) {
	if (program == NULL) {
		cout << "[SemanticAnalyseProgram] pointer of _Program is null" << endl;
		return;
	}
	//�⺯����������������������������ڼ���Ƿ��ض���ʱ�����ȼ�����ǰ���оٵ�˳��
	//�������������ܺͿ⺯������������������ܺͿ⺯��������������ͬ��
	//��������������кš�������������Ϣ
	set<string> lib;
	lib.insert("read");
	lib.insert("write");
	lib.insert("writeln");
	lib.insert("exit");
	if (lib.count(program->programId.first)) //checked
		addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(program->programId.second) + "> Name of program \"" + program->programId.first + "\" has been defined as a lib program.");
	mainSymbolTable->addProgramName(program->programId.first, program->programId.second, "procedure", int(program->paraList.size()), "");
	//��������Ĳ�����ӵ������ű��У�flag��Ϊ"parameter of program"
	for (int i = 0; i < program->paraList.size(); i++) {//�������������Ƿ����������ͬ��
		if (program->paraList[i].first == program->programId.first) //checked
			addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(program->programId.second) + "> parameter of program \"" + program->programId.first + "\" is the same as name of program.");
		else if (lib.count(program->paraList[i].first)) //checked
			addGeneralErrorInformation("[Dulicate defined error!] <Line " + itos(program->paraList[i].second) + "> parameter of program \"" + program->paraList[i].first + "\" has been defined as a lib program.");
		mainSymbolTable->addVoidPara(program->paraList[i].first, program->paraList[i].second);
	}
	//�����ű�����ǰ����read��write��exit�ȿ⺯��
	//���ڿ⺯����˵��setProcedure�ĺ���������,lineNumber,amount,subSymbolTable��û���õ�
	//lineNumber=-1��subSymbolTable=NULL��ʾ�ǿ⺯��
	//amount=-1��ʾ���
	//���read���̣��ù��̱��
	mainSymbolTable->addProcedure("read", -1, -1, NULL);
	//���write���̣��ù��̱��
	mainSymbolTable->addProcedure("write", -1, -1, NULL);
	//���writeln���̣��ù��̱��
	mainSymbolTable->addProcedure("writeln", -1, -1, NULL);
	//���exit���̣��ù��̵Ĳ���������Ҫ��������ۣ�������������У�����ָ��Ϊ0û�����⺬��
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

//�����±������ƥ�䡢��������̵�ʵ�κ��βεĸ�����ƥ��
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

bool checkIsTheSameAsKey(string id, int lineNumber) { //�����Keyָ���ǿ���������������������������
	for (int i = 0; i <= mainSymbolTable->recordList[0]->amount + 4; i++) {
		if (id == mainSymbolTable->recordList[i]->id) {
			if (i == 0) //����������ͬ�� checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as the name of program at Line " + itos(mainSymbolTable->recordList[i]->lineNumber) + ".");
			else if (i >= 1 && i <= mainSymbolTable->recordList[0]->amount) //�����������ͬ�� checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as a program parameter at Line " + itos(mainSymbolTable->recordList[i]->lineNumber) + ".");
			else //������ͬ�� checked
				addGeneralErrorInformation("[Duplicate defined error!] <Line " + itos(lineNumber) + "> \"" + id + "\" has been defined as a lib program.");
			return true;
		}
	}
	return false;
}