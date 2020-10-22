/*
符号表实现代码
*/
#include "symbolTable.h"
#include <iostream>
using namespace std;

_SymbolTable *mainSymbolTable;//主符号表
_SymbolTable *currentSymbolTable;//当前符号表

_SymbolRecord* findSymbolRecord(_SymbolTable* currentSymbolTable, string id, int mode = 0);

void _SymbolTable::addPara(string id, int lineNumber, string type) {
	if (this->idToLoc.count(id)) {
		for (int i = 0; i < idCount[id]; i++)
			id = "_" + id;
	}
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setPara(id, lineNumber, type);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
	if (this->idCount.count(id))
		this->idCount[id]++;
	else
		this->idCount[id] = 1;
}

void _SymbolTable::addVarPara(string id, int lineNumber, string type) {
	if (this->idToLoc.count(id)) {
		for (int i = 0; i < idCount[id]; i++)
			id = "_" + id;
	}
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setVarPara(id, lineNumber, type);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
	if (this->idCount.count(id))
		this->idCount[id]++;
	else
		this->idCount[id] = 1;
}

void _SymbolTable::addVar(string id, int lineNumber, string type) {
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setVar(id, lineNumber, type);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
}

void _SymbolTable::addConst(string id, int lineNumber, string type, bool isMinusShow, string value) {
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setConst(id, lineNumber, type, isMinusShow, value);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
}

void _SymbolTable::addArray(string id, int lineNumber, string type, int amount, vector< pair<int, int> > arrayRangeList) {
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setArray(id, lineNumber, type, amount, arrayRangeList);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
}

void _SymbolTable::addProcedure(string id, int lineNumber, int amount, _SymbolTable *subSymbolTable) {
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setProcedure(id, lineNumber, amount, subSymbolTable);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
}

void _SymbolTable::addFunction(string id, int lineNumber, string type, int amount, _SymbolTable *subSymbolTable) {
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setFunction(id, lineNumber, type, amount, subSymbolTable);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
}

void _SymbolTable::addSubSymbolTable(string id, _SymbolTable* symbolTable) {
	if (idToLoc.count(id)) {
		int loc = idToLoc[id];
		recordList[loc]->subSymbolTable = symbolTable;
	}
	else 
		cout << "[_SymbolTable::addSubSymbolTable] ERROR: subprogram id not found" << endl;
}

void _SymbolTable::addProgramName(string id, int lineNumber, string subprogramType, int amount, string returnType) {
	if (recordList.size() != 0) {
		cout << "[_SymbolTable::addProgramName] recordList.size() is not equal to 0" << endl;
		return;
	}
	if (this->idToLoc.count(id)) {
		for (int i = 0; i < idCount[id]; i++)
			id = "_" + id;
	}
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setProgramName(id, lineNumber, subprogramType, amount, returnType);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
	if (this->idCount.count(id))
		this->idCount[id]++;
	else
		this->idCount[id] = 1;
}

void _SymbolTable::addVoidPara(string id, int lineNumber) {
	if (this->idToLoc.count(id)) {
		for (int i = 0; i < idCount[id]; i++)
			id = "_" + id;
	}
	_SymbolRecord* tmpRecord = new _SymbolRecord;
	tmpRecord->setVoidPara(id, lineNumber);
	this->recordList.push_back(tmpRecord);
	this->idToLoc[id] = int(recordList.size() - 1);
	if (this->idCount.count(id))
		this->idCount[id]++;
	else
		this->idCount[id] = 1;
}

_SymbolTable::_SymbolTable(string type) {
	recordList.clear();
	idToLoc.clear();
	this->tableType=type;
}

void _SymbolRecord::setPara(string id, int lineNumber, string type) {
	flag = "value parameter";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
}

void _SymbolRecord::setVarPara(string id,int lineNumber, string type) {
	flag = "var parameter";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
}

void _SymbolRecord::setVar(string id, int lineNumber, string type) {
	flag = "normal variant";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
}

void _SymbolRecord::setConst(string id, int lineNumber, string type, bool isMinusShow, string value) {
	flag = "constant";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
	this->isMinusShow = isMinusShow;
	if (type == "char")
		this->value += "'";
	this->value += value;
	if (type == "char")
		this->value += "'";
}

void _SymbolRecord::setArray(string id, int lineNumber, string type, int amount, vector< pair<int,int> > arrayRangeList) {
	flag = "array";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
	this->amount = amount;
	this->arrayRangeList = arrayRangeList;
}

void _SymbolRecord::setProcedure(string id, int lineNumber, int amount, _SymbolTable *subSymbolTable) {
	flag = "procedure";
	this->id = id;
	this->lineNumber = lineNumber;
	this->amount = amount;
	this->subSymbolTable = subSymbolTable;
}

void _SymbolRecord::setFunction(string id, int lineNumber, string type, int amount, _SymbolTable *subSymbolTable) {
	flag = "function";
	this->id = id;
	this->lineNumber = lineNumber;
	this->type = type;
	this->amount = amount;
	this->subSymbolTable = subSymbolTable;
}

//这是一条特殊的记录，表示当前符号表对应的程序名、行号、"procedure"还是"function"、参数个数
//如果是"function"，则returnType为其返回值类型
//如果是"procedure"，则returnType为""
void _SymbolRecord::setProgramName(string id, int lineNumber, string subprogramType, int amount, string returnType) {
	flag = "(sub)program name";
	this->id = id;
	this->lineNumber = lineNumber;
	this->subprogramType = subprogramType;
	this->amount = amount;
	this->type = returnType;
}

void _SymbolRecord::setVoidPara(string id, int lineNumber) {
	flag = "parameter of program";
	this->id = id;
	this->lineNumber = lineNumber;
}

string _SymbolRecord::findXthFormalParaType(int X) {
	if ((flag == "function" || flag == "procedure") && X >= 1 && X < amount + 1)
		return subSymbolTable->recordList[X]->type;
	else {
		cout << "[_SymbolRecord::findXthFormalParaType] record is not a function or procedure, or formal parameter index out of range" << endl;
		return "";
	}
}

bool _SymbolRecord::isXthFormalParaRefered(int X) {
	if ((flag == "function" || flag == "procedure") && X >= 1 && X < amount + 1)
		return subSymbolTable->recordList[X]->flag=="var parameter";
	else {
		cout << "[_SymbolRecord::isXthFormalParaRefered] record is not a function or procedure, or formal parameter index out of range" << endl;
		return "";
	}
}

bool _SymbolRecord::checkArrayXthIndexRange(int X, int index) {
	if ((flag == "array") && X >= 0 && X < amount)
		return index >= arrayRangeList[X].first&&index <= arrayRangeList[X].second;
	else {
		cout << "[_SymbolRecord::checkArrayXthIndexRange] record is not a array, or index out of range" << endl;
		return false;
	}
}

//找出标识符在符号表中的位置
_SymbolRecord* findSymbolRecord(_SymbolTable* currentSymbolTable, string id, int mode) {
	if (currentSymbolTable->idToLoc.count(id)) {
		int loc = currentSymbolTable->idToLoc[id];
		return currentSymbolTable->recordList[loc];
	}
	if (mode != 0)
		return NULL;
	if (currentSymbolTable->tableType == "sub" && mainSymbolTable->idToLoc.count(id)) {
		int loc = mainSymbolTable->idToLoc[id];
		return mainSymbolTable->recordList[loc];
	}
	return NULL;
}

//检查id是否是引用参数
bool checkIsReferedPara(_SymbolTable* currentSymbolTable, string id) {
	if (currentSymbolTable->tableType == "main")
		return false;
	if (currentSymbolTable->idToLoc.count(id)) {
		int loc = currentSymbolTable->idToLoc[id];
		_SymbolRecord *record = currentSymbolTable->recordList[loc];
		return record->flag == "var parameter";
	}
	else //子符号表没找到，那么就是在主符号表中，主符号表中没有传值参数这一类
		return false;
}