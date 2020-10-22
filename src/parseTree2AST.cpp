/*
普通语法分析树到抽象语法树的转换
*/
#include "main.h"
#include "ASTnodes.h"
#include <algorithm>

int str2int(string str);
float str2float(string str);
void getIdList(Type *now, vector< pair<string, int> >& res,bool reverseFlag);
void getArrayRangeList(Type *now, vector< pair<int, int> >& _arrayRangeList);
_Type* getType(Type *now);
void getVariant(Type *now, vector<_Variant*>& _variantList);
void getVariantList(Type *now, vector<_Variant*>& _variantList);
void setConst(Type *now, _Constant* &_constant);
void getConst(Type *now, vector<_Constant*>& _constantList);
void getConstList(Type *now, vector<_Constant*>& _constantList);
void getValueParameter(Type *now, vector<_FormalParameter*>& _formalParaList, int flag = 0);
void getParameter(Type *now, vector<_FormalParameter*>& _formalParaList);
void getFormalParameter(Type *now, vector<_FormalParameter*>& _formalParaList);
void getFormalParaList(Type *now, vector<_FormalParameter*>& _formalParaList);
void getSubprogramHead(Type *now, pair<string, int>& functionID, vector<_FormalParameter*>& _formalParaList, pair<string,int> &_type);
_Expression* getFactor(Type *now);
_Expression* getTerm(Type *now);
_Expression* getSimpleExpression(Type *now);
_Expression* getExpression(Type *now);
void getExpressionList(Type *now, vector<_Expression*>& _expressionList);
void getVariantReferenceList(Type *now, vector<_Expression*>& _expressionList);
_VariantReference* getVariantReference(Type *now);
_Statement* getElseStatement(Type *now);
_Statement* getProcedureCall(Type *now);
_Statement* getStatement(Type *now);
void getStatementList(Type *now, vector<_Statement*>& _statementList);
_Compound* getCompoundStatement(Type *now);
void getSubprogramBody(Type *now, vector<_Constant*>& _constList, vector<_Variant*>& _variantList, _Compound* &_compound);
_FunctionDefinition* getSubprogramDefinition(Type *now);
void getSubprogramDefinitionList(Type *now, vector<_FunctionDefinition*>& _subprogramDefinitionList);
_SubProgram* getProgramBody(Type *now);
void getProgramHead(Type *now, pair<string, int>& _programId, vector< pair<string, int> >& _paraList);
_Program* getProgram(Type *now);

int str2int(string str){
    int res=0;
    int len=int(str.length());
    for(int i=0;i<len;i++){
        res=res*10;
        res+=str[i]-'0';
    }
    return res;
}

float str2float(string str){
    float res=0;
    int len=int(str.length());
    int loc=int(str.find('.'));
    for(int i=0;i<loc;i++){
        res*=10;
        res+=str[i]-'0';
    }
    float base=1;
    for(int i=loc+1;i<len;i++){
        base=base*0.1;
        res+=base*(str[i]-'0');
    }
    return res;
}

void getIdList(Type *now,vector< pair<string,int> >& res,bool reverseFlag){
    if(now->token!="idlist"){
        cout << "getIdList error" << endl;
        return;
    }
    if(now->children[0]->str==""){
        res.push_back(make_pair(now->children[2]->str,now->children[2]->lineNumber));
        getIdList(now->children[0],res,reverseFlag);
    }
    else{
        res.push_back(make_pair(now->children[0]->str,now->children[0]->lineNumber));
		if(reverseFlag)
			reverse(res.begin(),res.end());
    }
}

void getArrayRangeList(Type *now,vector< pair<int,int> >& _arrayRangeList){
    if(now->token!="period"){
        cout << "getArrayRangeList error" << endl;
        return;
    }
    int loc=int(now->children.size()-3);
    _arrayRangeList.push_back(make_pair(str2int(now->children[loc]->str),str2int(now->children[loc+2]->str)));
    if(loc==2)
        getArrayRangeList(now->children[0],_arrayRangeList);
    else
        reverse(_arrayRangeList.begin(),_arrayRangeList.end());
}

_Type* getType(Type *now){
    if(now->token!="type"){
        cout << "getType error" << endl;
        return NULL;
    }
    _Type* _type = new _Type;
    int loc=int(now->children.size()-1);
    _type->type=make_pair(now->children[loc]->str,now->children[loc]->lineNumber);
    if(loc==5){
        _type->flag=1;
        getArrayRangeList(now->children[2],_type->arrayRangeList);
    }
    else
        _type->flag=0;
    return _type;
}

void getVariant(Type *now,vector<_Variant*>& _variantList){
    if(now->token!="var_declaration"){
        cout << "getVariant error" << endl;
        return;
    }
    vector< pair<string,int> > _idList;
    int loc=int(now->children.size()-3);
    getIdList(now->children[loc],_idList,false);
    _Type *_type=getType(now->children[loc+2]);
    for(int i=0;i<_idList.size();i++)
        _variantList.push_back(new _Variant(_idList[i],_type));
    if(loc==2)
        getVariant(now->children[0],_variantList);
    else
        reverse(_variantList.begin(),_variantList.end());
}

void getVariantList(Type *now,vector<_Variant*>& _variantList){
    if(now->token!="var_declarations"){
        cout << "getVariantList error" << endl;
        return;
    }
    if(now->children.size())
        getVariant(now->children[1],_variantList);
}

void setConst(Type *now,_Constant* &_constant){//pascal在定义常量时，并没有指定常量的类型，所以需要自行判断
    if(now->token!="const_value"){
        cout << "setConst error" << endl;
        return;
    }
    int loc=1;
    if(now->children.size()==1)
        loc=0;
    if(now->children[loc]->token=="IDENTIFIER"){//如果右值是标识符
        _constant->type="id";
        _constant->valueId = make_pair(now->children[loc]->str,now->children[loc]->lineNumber);
		_constant->strOfVal = now->children[loc]->str;
		_constant->isMinusShow = (loc == 1 && now->children[0]->token == "-");
    }
    else if(now->children[loc]->token=="UINUM"){
        _constant->type="integer";
        _constant->intValue=str2int(now->children[loc]->str);
		_constant->strOfVal = now->children[loc]->str;
		_constant->isMinusShow = (loc == 1 && now->children[0]->token == "-");
    }
    else if(now->children[loc]->token=="UFNUM"){
        _constant->type="real";
        _constant->realValue=str2float(now->children[loc]->str);
		_constant->strOfVal = now->children[loc]->str;
		_constant->isMinusShow = (loc == 1 && now->children[0]->token == "-");
    }
    else if(now->children[loc]->token=="CHAR"){
        _constant->type="char";
        _constant->charValue=now->children[loc]->str[0];
		_constant->strOfVal = now->children[loc]->str;
    }
    else{
        cout << "setConst error" << endl;
    }
}

void getConst(Type *now,vector<_Constant*>& _constantList){
    if(now->token!="const_declaration"){
        cout << "getConst error" << endl;
        return;
    }
    int loc=int(now->children.size()-3);
    _Constant* _constant=new _Constant;
    _constant->constId=make_pair(now->children[loc]->str,now->children[loc]->lineNumber);
    setConst(now->children[loc+2],_constant);
    _constantList.push_back(_constant);
    if(loc==2)
        getConst(now->children[0],_constantList);
    else
        reverse(_constantList.begin(),_constantList.end());
}

void getConstList(Type *now,vector<_Constant*>& _constantList){
    if(now->token!="const_declarations"){
        cout << "getConstList error" << endl;
        return;
    }
    if(now->children.size())
        getConst(now->children[1],_constantList);
}

void getValueParameter(Type *now,vector<_FormalParameter*>& _formalParaList,int flag){
    if(now->token!="value_parameter"){
        cout << "getValueParameter error" << endl;
        return;
    }
    vector< pair<string,int> > _idList;
    getIdList(now->children[0],_idList,false);
    string _type=now->children[2]->str;
    for(int i=0;i<_idList.size();i++)
        _formalParaList.push_back(new _FormalParameter(_idList[i],_type,flag));
}

void getParameter(Type *now,vector<_FormalParameter*>& _formalParaList){
    if(now->token!="parameter"){
        cout << "getParameter error" << endl;
        return;
    }
    if(now->children[0]->token=="var_parameter")
        getValueParameter(now->children[0]->children[1],_formalParaList,1);
    else if(now->children[0]->token=="value_parameter")
        getValueParameter(now->children[0],_formalParaList,0);
    else
        cout << "getParameter error" << endl;
}

void getFormalParameter(Type *now,vector<_FormalParameter*>& _formalParaList){
    if(now->token!="parameter_list"){
        cout << "getFormalParameter error" << endl;
        return;
    }
    int loc=int(now->children.size()-1);
    getParameter(now->children[loc],_formalParaList);
    if(loc==2)
        getFormalParameter(now->children[0],_formalParaList);
    else
        reverse(_formalParaList.begin(),_formalParaList.end());
}

void getFormalParaList(Type *now,vector<_FormalParameter*>& _formalParaList){
    if(now->token!="formal_parameter"){
        cout << "getFormalParaList error" << endl;
        return;
    }
    if(now->children.size())
        getFormalParameter(now->children[1],_formalParaList);
}

void getSubprogramHead(Type *now,pair<string,int>& functionID,vector<_FormalParameter*>& _formalParaList,pair<string,int> &_type){
    if(now->token!="subprogram_head"){
        cout << "getSubprogramHead error" << endl;
        return;
    }
    functionID=make_pair(now->children[1]->str,now->children[1]->lineNumber);
    getFormalParaList(now->children[2],_formalParaList);
	_type=make_pair("",-1);
    if (now->children.size() == 5) 
		_type = make_pair(now->children[4]->str, now->children[4]->lineNumber);
}

//"var"表示变量,"integer"表示整数,"real"表示浮点数,"char"表示字符常量
//"function"表示函数调用,"compound"表示复合表达式
//compound有普通的二目运算符，还有minus、not、bracket等单目运算符
_Expression* getFactor(Type *now){
    if(now->token!="factor"){
        cout << "getFactor error" << endl;
		return NULL;
    }
    _Expression* _expression = new _Expression;
    _expression->operand1=_expression->operand2=NULL;
    if(now->children[0]->token=="UINUM"){
        _expression->type="integer";
		_expression->strOfNum = now->children[0]->str;
        _expression->intNum=str2int(now->children[0]->str);
        _expression->lineNumber=now->children[0]->lineNumber;
    }
    else if(now->children[0]->token=="UFNUM"){
        _expression->type="real";
		_expression->strOfNum = now->children[0]->str;
        _expression->realNum=str2float(now->children[0]->str);
        _expression->lineNumber=now->children[0]->lineNumber;
    }
    else if(now->children[0]->token=="variable"){
        _expression->type="var";
        _expression->variantReference=getVariantReference(now->children[0]);
		_expression->lineNumber = _expression->variantReference->variantId.second;
    }
    else if(now->children[0]->token=="IDENTIFIER"){
        _expression->type="function";
        _expression->functionCall = new _FunctionCall;
        _expression->functionCall->functionId=make_pair(now->children[0]->str,now->children[0]->lineNumber);
        getExpressionList(now->children[2],_expression->functionCall->actualParaList);
		_expression->lineNumber = _expression->functionCall->functionId.second;
    }
    else if(now->children[0]->token=="("){
        _expression->type="compound";
		_expression->operationType = "single";
        _expression->operation="bracket";
        _expression->operand1=getExpression(now->children[1]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
    else if(now->children[0]->token=="NOT"){
        _expression->type="compound";
		_expression->operationType = "single";
        _expression->operation="not";
        _expression->operand1=getFactor(now->children[1]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
    else if(now->children[0]->token=="-"){
        _expression->type="compound";
		_expression->operationType = "single";
        _expression->operation="minus";
        _expression->operand1=getFactor(now->children[1]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
	else if (now->children[0]->token == "CHAR") {
		_expression->type = "char";
		_expression->charVal = now->children[0]->str[0];
		_expression->lineNumber = now->children[0]->lineNumber;
	}
    else{
        cout << "getFactor error" << endl;
        return NULL;
    }
    return _expression;
}

_Expression* getTerm(Type *now){
    if(now->token!="term"){
        cout << "term" << endl;
		return NULL;
    }
    _Expression* _expression=NULL;
    if(now->children.size()==3){
        _expression = new _Expression;
        _expression->type="compound";
        _expression->operation=now->children[1]->str;
        _expression->operationType="mulop";
        _expression->operand1=getTerm(now->children[0]);
        _expression->operand2=getFactor(now->children[2]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
    else
        _expression=getFactor(now->children[0]);
    return _expression;
}

_Expression* getSimpleExpression(Type *now){
    if(now->token!="simple_expression"){
        cout << "getSimpleExpression error" << endl;
		return NULL;
    }
    _Expression* _expression=NULL;
    if(now->children.size()==3){
        _expression = new _Expression;
        _expression->type="compound";
        _expression->operation=now->children[1]->str;
        _expression->operationType="addop";
        _expression->operand1=getSimpleExpression(now->children[0]);
        _expression->operand2=getTerm(now->children[2]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
    else
        _expression=getTerm(now->children[0]);
    return _expression;
}

_Expression* getExpression(Type *now){
    if(now->token!="expression"){
        cout << "getExpression error" << endl;
		return NULL;
    }
    _Expression* _expression=NULL;
    if(now->children.size()==3){
        _expression = new _Expression;
        _expression->type="compound";
        _expression->operation=now->children[1]->str;
        _expression->operationType="relop";
        _expression->operand1=getSimpleExpression(now->children[0]);
        _expression->operand2=getSimpleExpression(now->children[2]);
		_expression->lineNumber = _expression->operand1->lineNumber;
    }
    else
        _expression=getSimpleExpression(now->children[0]);
    return _expression;
}

void getExpressionList(Type *now,vector<_Expression*>& _expressionList){
    if(now->token!="expression_list"){
        cout << "getExpressionList error" << endl;
        return;
    }
    int loc=int(now->children.size()-1);
    _expressionList.push_back(getExpression(now->children[loc]));
    if(loc==2)
        getExpressionList(now->children[0],_expressionList);
    else
        reverse(_expressionList.begin(),_expressionList.end());
}

void getVariantReferenceList(Type *now,vector<_Expression*>& _expressionList){
    if(now->token!="id_varpart"){
        cout << "getVariantReferenceList error" << endl;
        return;
    }
    if(now->children.size())
        getExpressionList(now->children[1],_expressionList);
}

_VariantReference* getVariantReference(Type *now){
    if(now->token!="variable"){
        cout << "getVariantReference error" << endl;
		return NULL;
    }
    _VariantReference* _variantReference = new _VariantReference;
    _variantReference->variantId=make_pair(now->children[0]->str,now->children[0]->lineNumber);
    getVariantReferenceList(now->children[1],_variantReference->expressionList);
	if (_variantReference->expressionList.size())
		_variantReference->flag = 1;
	else
		_variantReference->flag = 0;
    return _variantReference;
}

_Statement* getElseStatement(Type *now){
    if(now->token!="else_part"){
        cout << "getElseStatement error" << endl;
        return NULL;
    }
    if(now->children.size()==0)
        return NULL;
    return getStatement(now->children[1]);
}

_Statement* getProcedureCall(Type *now) {
	if (now->token != "procedure_call") {
		cout << "getProcedureCall error" << endl;
		return NULL;
	}
	_ProcedureCall *_procedureCall = new _ProcedureCall;
	_procedureCall->lineNumber = now->children[0]->lineNumber;
	_procedureCall->type = "procedure";
	_procedureCall->procedureId = make_pair(now->children[0]->str, now->children[0]->lineNumber);
	if (now->children.size() == 4)
		getExpressionList(now->children[2], _procedureCall->actualParaList);
	return _procedureCall;
}

_Statement* getStatement(Type *now){
    if(now->token!="statement"){
        cout << "getStatement error" << endl;
        return NULL;
    }
	if (now->children.size() == 0)
		return NULL;
    if(now->children[0]->token=="variable"){
        _AssignStatement *_assignStatement = new _AssignStatement;
		_assignStatement->lineNumber = now->children[1]->lineNumber;
        _assignStatement->type="assign";
        _assignStatement->variantReference=getVariantReference(now->children[0]);
        _assignStatement->expression=getExpression(now->children[2]);
        return _assignStatement;
    }
    else if(now->children[0]->token=="procedure_call")
		return getProcedureCall(now->children[0]);
    else if(now->children[0]->token=="compound_statement")
		return getCompoundStatement(now->children[0]);
    else if(now->children[0]->token=="IF"){
        _IfStatement* _ifStatement = new _IfStatement;
		_ifStatement->lineNumber = now->children[0]->lineNumber;
        _ifStatement->type="if";
        _ifStatement->condition=getExpression(now->children[1]);
        _ifStatement->then=getStatement(now->children[3]);
        _ifStatement->els=getElseStatement(now->children[4]);
        return _ifStatement;
    }
    else if(now->children[0]->token=="FOR"){
        _ForStatement* _forStatement = new _ForStatement;
		_forStatement->lineNumber = now->children[0]->lineNumber;
        _forStatement->type="for";
        _forStatement->id=make_pair(now->children[1]->str,now->children[1]->lineNumber);
        _forStatement->start=getExpression(now->children[3]);
        _forStatement->end=getExpression(now->children[5]);
        _forStatement->_do=getStatement(now->children[7]);
        return _forStatement;
    }
    else if(now->children[0]->token=="WHILE"){
        _WhileStatement* _whileStatement = new _WhileStatement;
		_whileStatement->lineNumber = now->children[0]->lineNumber;
        _whileStatement->type="while";
        _whileStatement->condition=getExpression(now->children[1]);
        _whileStatement->_do=getStatement(now->children[3]);
        return _whileStatement;
    }
    else if(now->children[0]->token=="REPEAT"){
        _RepeatStatement* _repeatStatement = new _RepeatStatement;
		_repeatStatement->lineNumber = now->children[0]->lineNumber;
        _repeatStatement->type="repeat";
        _repeatStatement->condition=getExpression(now->children[3]);
        _repeatStatement->_do=getStatement(now->children[1]);
        return _repeatStatement;
    }
    else{
        cout << "[getStatement] statement token error" << endl;
        return NULL;
    }
}

void getStatementList(Type *now,vector<_Statement*>& _statementList){
    if(now->token!="statement_list"){
        cout << "getStatementList error" << endl;
        return;
    }
    int loc=int(now->children.size()-1);
	_Statement* statement = getStatement(now->children[loc]);
	if(statement != NULL)
		_statementList.push_back(statement);
    if(loc==2)
        getStatementList(now->children[0],_statementList);
    else
        reverse(_statementList.begin(),_statementList.end());
}

_Compound* getCompoundStatement(Type *now){
    if(now->token!="compound_statement"){
        cout << "getCompoundStatement error" << endl;
        return NULL;
    }
	_Compound *_compound = new _Compound;
	_compound->lineNumber = now->children[0]->lineNumber;
	_compound->type = "compound";
    getStatementList(now->children[1],_compound->statementList);
    return _compound;
}

void getSubprogramBody(Type *now,vector<_Constant*>& _constList,vector<_Variant*>& _variantList,_Compound* &_compound){
    if(now->token!="subprogram_body"){
        cout << "getSubprogramBody error" <<endl;
        return;
    }
    getConstList(now->children[0],_constList);
    getVariantList(now->children[1],_variantList);
	_compound = getCompoundStatement(now->children[2]);
}

_FunctionDefinition* getSubprogramDefinition(Type *now){
    if(now->token!="subprogram"){
        cout << "getSubprogramDefinition error" << endl;
        return NULL;
    }
    _FunctionDefinition *_functionDefinition=new _FunctionDefinition;
    getSubprogramHead(now->children[0],_functionDefinition->functionID,_functionDefinition->formalParaList,_functionDefinition->type);
    getSubprogramBody(now->children[2],_functionDefinition->constList,_functionDefinition->variantList,_functionDefinition->compound);
    return _functionDefinition;
}

void getSubprogramDefinitionList(Type *now,vector<_FunctionDefinition*>& _subprogramDefinitionList){
    if(now->token!="subprogram_declarations"){
        cout << "getSubprogramDefinitionList error" << endl;
        return;
    }
	if (now->children.size()) {
		_subprogramDefinitionList.push_back(getSubprogramDefinition(now->children[1]));
		getSubprogramDefinitionList(now->children[0], _subprogramDefinitionList);
	}
    else
        reverse(_subprogramDefinitionList.begin(),_subprogramDefinitionList.end());
}

_SubProgram* getProgramBody(Type *now){
    if(now->token!="program_body"){
        cout << "getProgramBody error" << endl;
        return NULL;
    }
    _SubProgram *_subProgram=new _SubProgram;
    getConstList(now->children[0],_subProgram->constList);
    getVariantList(now->children[1],_subProgram->variantList);
    getSubprogramDefinitionList(now->children[2],_subProgram->subprogramDefinitionList);
	_subProgram->compound = getCompoundStatement(now->children[3]);
    return _subProgram;
}

void getProgramHead(Type *now,pair<string,int>& _programId,vector< pair<string,int> >& _paraList){
    if(now->token!="program_head"){
        cout << "getProgramHead error" << endl;
        return; 
    }
    _programId=make_pair(now->children[1]->str,now->children[1]->lineNumber);
    getIdList(now->children[3],_paraList,true);
}

_Program* getProgram(Type *now){
    if(now->token!="programstruct"){
        cout << "getProgram error" << endl;
        return NULL;
    }
    _Program* ASTRoot=new _Program;
    getProgramHead(now->children[0],ASTRoot->programId,ASTRoot->paraList);
    ASTRoot->subProgram=getProgramBody(now->children[2]);
    return ASTRoot;
}