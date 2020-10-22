#include "ASTnodes.h"
#include <iostream>
using namespace std;
#define DEL(x) {\
    if(x!=NULL){\
        delete x;\
        x=NULL;\
    }\
}

/*
抽象语法树各类型节点的方法实现代码，主要涉及语义分析
*/

_Expression::_Expression(){
    variantReference=NULL;
    functionCall=NULL;
    operand1=operand2=NULL;
    totalIntValueValid=false;
}

_Expression::~_Expression(){
    DEL(variantReference)
    DEL(functionCall)
    DEL(operand1)
    DEL(operand2)
}

_FunctionCall::_FunctionCall(){
    actualParaList.clear();
}

_FunctionCall::~_FunctionCall(){
    for(int i=0;i<actualParaList.size();i++){
        DEL(actualParaList[i])
    }
}

_VariantReference::_VariantReference(){
    expressionList.clear();
	locFlag = 1;//默认是右值
}

_VariantReference::~_VariantReference(){
    for(int i=0;i<expressionList.size();i++){
        DEL(expressionList[i])
    }
}

_Compound::_Compound(){
	isReturnStatement = false;
    statementList.clear();
}

_Compound::~_Compound(){
    for(int i=0;i<statementList.size();i++){
        DEL(statementList[i])
    }
}

_RepeatStatement::_RepeatStatement(){
	isReturnStatement = false;
    condition=NULL;
    _do=NULL;
}

_RepeatStatement::~_RepeatStatement(){
    DEL(condition)
    DEL(_do)
}

_WhileStatement::_WhileStatement(){
	isReturnStatement = false;
    condition=NULL;
    _do=NULL;
}

_WhileStatement::~_WhileStatement(){
    DEL(condition)
    DEL(_do)
}

_ForStatement::_ForStatement(){
	isReturnStatement = false;
    start=NULL;
    end=NULL;
    _do=NULL;
}

_ForStatement::~_ForStatement(){
    DEL(start)
    DEL(end)
    DEL(_do)
}

_IfStatement::_IfStatement(){
	isReturnStatement = false;
    condition=NULL;
    then=NULL;
    els=NULL;
}

_IfStatement::~_IfStatement(){
    DEL(condition)
    DEL(then)
    DEL(els)
}

_AssignStatement::_AssignStatement(){
	isReturnStatement = false;
    variantReference=NULL;
    expression=NULL;
}

_AssignStatement::~_AssignStatement(){
    DEL(variantReference)
    DEL(expression)
}

_ProcedureCall::_ProcedureCall(){
	isReturnStatement = false;
    actualParaList.clear();
}

_ProcedureCall::~_ProcedureCall(){
    for(int i=0;i<actualParaList.size();i++){
        DEL(actualParaList[i])
    }
}

_Type::_Type(){
    arrayRangeList.clear();
}

_Type::_Type(pair<string,int> _type,int _flag,vector< pair<int,int> > _arrayRangeList):type(_type),flag(_flag),arrayRangeList(_arrayRangeList){

}

_FormalParameter::_FormalParameter(){
    
}

_FormalParameter::_FormalParameter(pair<string,int> _paraId,string _type,int _flag):paraId(_paraId),type(_type),flag(_flag){

}

_FunctionDefinition::_FunctionDefinition(){
    formalParaList.clear();
    constList.clear();
    variantList.clear();
    compound=NULL;
}

_FunctionDefinition::~_FunctionDefinition(){
    for(int i=0;i<formalParaList.size();i++){
        DEL(formalParaList[i])
    }
    for(int i=0;i<constList.size();i++){
        DEL(constList[i])
    }
    for(int i=0;i<variantList.size();i++){
        DEL(variantList[i])
    }
    DEL(compound)
}

_Variant::_Variant(){
    type=NULL;
}

_Variant::~_Variant(){
    DEL(type)
}

_Variant::_Variant(pair<string,int> _variantId,_Type *_type):variantId(_variantId),type(_type){

}

_SubProgram::_SubProgram(){
    constList.clear();
    variantList.clear();
    subprogramDefinitionList.clear();
    compound=NULL;
}

_SubProgram::~_SubProgram(){
    for(int i=0;i<constList.size();i++){
        DEL(constList[i])
    }
    for(int i=0;i<variantList.size();i++){
        DEL(variantList[i])
    }
    for(int i=0;i<subprogramDefinitionList.size();i++){
        DEL(subprogramDefinitionList[i])
    }
    DEL(compound)
}

_Program::_Program(){
    paraList.clear();
    subProgram=NULL;
}

_Program::~_Program(){
    DEL(subProgram)
}