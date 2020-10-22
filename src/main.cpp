/*
主函数
*/
#include "main.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <windows.h>
class _Program;//暂时只需要前向声明（后面也尽量保持这种状态）

extern Type* ParseTreeHead;
extern FILE* yyin;
extern _Program* getProgram(Type *now);
extern int yydebug;
extern bool haveSemanticError;
extern void SemanticAnalyse(_Program *ASTRoot);
extern vector<string> lexicalErrorInformation;
extern vector<string> syntaxErrorInformation;
extern vector<string> semanticErrorInformation;
extern vector<string> semanticWarningInformation;
extern void codeGenerate(_Program *ASTRoot, string outName);

extern "C"{
    int yyparse();
}

int errorCount = 0;
int errorBound = int(1e9 + 7); //默认错误上限，INF

map<string, string> argumentsExplanation;

void preProcess(string inName);
void argumentsExplanationInit();
void outputArgumentsExplanation();
string char2str(char* chs);
bool chs2int(char* chs, int &num);
void dfs(Type* now);
void getRunArguments(int argc, char **argv, string &inName, string &outName, string &compilerName, string &exeName, int &errorBound, bool &willCompile, bool &willExecute);
void outputErrors();

// -inname [file name] 表示指定输入文件名，默认为"PascalProgram.pas"
// -outname [file name] 表示指定输出文件名，默认为"CProgram.c"
// -compiler [compiler name] 表示指定c编译器名，默认为"gcc",并将c程序编译成可执行文件，默认不编译
// -exename [exe name] 表示指定可执行文件名，默认为"CProcess.exe",如果未出现-compiler参数，则默认用"gcc"进行编译
// -execute 表示自动执行生成的可执行文件，如果未出现-e、-exename参数，则均按照默认方式进行操作
// -errorbound [n] 表示指定错误上限，默认无上限，即编译器发现了指定个数的错误后，立即停止运行
// -developer表示输出开发者信息
// -version表示输出版本信息
// -help表示输出所有命令行参数的帮助信息

int main(int argc, char **argv){
	argumentsExplanationInit();
	string inName = "PascalProgram.pas"; //默认输入文件名
	string outName = "CProgram.c"; //默认输出文件名
	string compilerName = "gcc"; //默认C编译器
	string exeName = "CProcess.exe"; //默认的编译C程序后获得的可执行文件名
	bool willCompile = false; //默认不编译C程序
	bool willExecute = false; //默认不执行编译C程序后获得的可执行程序

	//处理命令行参数
	getRunArguments(argc, argv, inName, outName, compilerName, exeName, errorBound, willCompile, willExecute);
	
	//yydebug=1;//语法分析DEBUG信息开关

	FILE *fp = NULL;
	fp = fopen(inName.c_str(), "r");
	if (fp == NULL) {
		cout << "Cannot open PASCAL-S file " << inName.c_str() << " , please check it." << endl;
		exit(0);
	}
	fclose(fp);

	cout << "Now start pre process..." << endl;
	preProcess(inName); //预处理

	//通过将打开文件指针赋值为yyin，给词法分析程序提供输入
#ifdef _WIN32
	const char* sFile = "preProcessed.pas";
#elif __APPLE__
	const char *sFile = "preProcessed.pas";
#endif
	fp = fopen(sFile,"r");
	if(fp==NULL){
		printf("Cannot open %s\n",sFile);
		return -1;
	}
	yyin=fp;
	cout << "Now start lex and syntax analyse..." << endl;
	haveSemanticError = false;
	yyparse();//调用语法分析程序
	fclose(fp);

	//dfs(ParseTreeHead); //遍历普通语法分析树

	bool canContinueToSemanticAnalyse = true;

	if (lexicalErrorInformation.size()) //如果有词法错误
		canContinueToSemanticAnalyse = false;
	else
		cout << "Lex analyse succeed!!!" << endl << endl; //没有词法错误

	if (haveSemanticError) //如果有语法错误
		canContinueToSemanticAnalyse = false;
	else if (canContinueToSemanticAnalyse)
		cout << "Syntax analyse succeed!!!" << endl << endl;

	if (!canContinueToSemanticAnalyse) { //如果有词法或语法错误
		outputErrors();
		system("pause");
		return 0;
	}

	//开始语义分析
	cout << "Now start semantic analysing..." << endl;
	_Program* ASTRoot=getProgram(ParseTreeHead);
	SemanticAnalyse(ASTRoot);//语义分析

	outputErrors();

	if (semanticErrorInformation.size()) { //如果有语义错误
		system("pause");
		return 0;
	}
	
	cout << "Semantic analyse succeed!!!" << endl << endl;

	//代码生成
	cout << "Now start generating the C Program code..." << endl;
	codeGenerate(ASTRoot, outName);
	cout << "Code Generate succeed!!!" << endl;
	cout << "Please check C code in " << outName << endl << endl;

	string cmd;

	if (willCompile) {
		cout << "Now compile the C program..." << endl;
		cmd = compilerName + " " + outName + " -o " + exeName;
		cout << cmd << endl;
		system(cmd.c_str());
		cout << endl;
	}

	if (willExecute) {
		cout << "Now execute the C process..." << endl;
		cmd = exeName;
		cout << cmd << endl;
		system(cmd.c_str());
	}

	//system("pause");
	return 0;
}

void argumentsExplanationInit() {
	argumentsExplanation["-inname"] = "-inname [file name]:\t\tdesignate the name of input pascal program, default is \"PascalProgram.pas\".";
	argumentsExplanation["-outname"] = "-outname [file name]:\t\tdesignate the name of output C program, default is \"CProgram.c\".";
	argumentsExplanation["-compiler"] = "-complier [complier name]:\tdesignate the name of C compiler, default is \"gcc\".";
	argumentsExplanation["-exename"] = "-exename [exe name]:\t\tdesignate the name of exe file, default is \"CProcess.exe\".";
	argumentsExplanation["-execute"] = "-execute:\t\t\tautomatically run the exe file.";
	argumentsExplanation["-errorbound"] = "-errorbound [n]:\t\tdesignate the up bound of error number as n, if the pascal2c compiler finds n errors, the compile process will abort, default is INF.";
	argumentsExplanation["-developer"] = "-developer:\t\t\tinformation about developers.";
	argumentsExplanation["-version"] = "-version:\t\t\tinformation about version of pascal2c compiler.";
	argumentsExplanation["-help"] = "-help:\t\t\t\toutput all the explanation abount command line arguments.";
}

void outputArgumentsExplanation() {
	for (auto it = argumentsExplanation.begin(); it != argumentsExplanation.end(); it++) {
		cout << it->second << endl;
	}
}

string char2str(char* chs) {
	string res;
	for (; *chs != 0; chs++)
		res += *chs;
	return res;
}

bool chs2int(char* chs, int &num) {
	int tmp = num;
	num = 0;
	for (; *chs != '\0'; chs++) {
		if (*chs >= '0'&&*chs <= '9') {
			num *= 10;
			num += *chs - '0';
		}
		else {
			num = tmp;
			return false;
		}
	}
	return true;
}

//普通语法树的debug信息（遍历输出）
void dfs(Type* now) {
	if (now->children.size() == 0) {
		if (now->str == "")
			cout << now->token << "\t->\t" << "empty" << endl;
		return;
	}
	cout << now->token << "\t->";
	for (int i = 0; i < now->children.size(); i++) {
		if (now->children[i]->children.size() == 0 && now->children[i]->str != "")
			cout << "\t\"" << now->children[i]->str << "\"";
		else
			cout << "\t" << now->children[i]->token;
	}
	cout << endl;
	for (int i = 0; i<now->children.size(); i++)
		dfs(now->children[i]);
}

void getRunArguments(int argc, char **argv, string &inName, string &outName, string &compilerName, string &exeName, int &errorBound, bool &willCompile, bool &willExecute){
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-inname") == 0) {
			if (i + 1 >= argc || argumentsExplanation.count(argv[i + 1]))
				continue;
			inName = char2str(argv[i + 1]);
			FILE* fp;
			fp = fopen(inName.c_str(), "r");
			if (fp == NULL) {
				cout << "Can not found the pascal program input file " << inName << endl;
				exit(0);
			}
			fclose(fp);
			i++;
		}
		else if (strcmp(argv[i], "-outname") == 0) {
			if (i + 1 >= argc || argumentsExplanation.count(argv[i + 1]))
				continue;
			outName = char2str(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-compiler") == 0) {
			willCompile = true;
			if (i + 1 >= argc || argumentsExplanation.count(argv[i + 1]))
				continue;
			compilerName = char2str(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-exename") == 0) {
			if (i + 1 >= argc || argumentsExplanation.count(argv[i + 1]))
				continue;
			exeName = char2str(argv[i + 1]);
			i++;
			willCompile = true;
		}
		else if (strcmp(argv[i], "-execute") == 0) {
			willCompile = true;
			willExecute = true;
		}
		else if (strcmp(argv[i], "-errorbound") == 0) {
			if (i + 1 >= argc || argumentsExplanation.count(argv[i + 1]))
				continue;
			if (!chs2int(argv[i + 1], errorBound)) {
				cout << "-errorbound need an integer arguments" << endl;
				exit(0);
			}
			i++;
		}
		else if (strcmp(argv[i], "-developer") == 0) {
			cout << "MilesGO BUPT" << endl;
			cout << "SamanthaYue BUPT" << endl;
			cout << "bupt_paul BUPT" << endl;
			cout << "wklhtcone BUPT" << endl;
			cout << "persistence69 BUPT" << endl;
			exit(0);
		}
		else if (strcmp(argv[i], "-version") == 0) {
			cout << "Version 1.0" << endl;
			exit(0);
		}
		else if (strcmp(argv[i], "-help") == 0) {
			outputArgumentsExplanation();
			exit(0);
		}
		else {
			cout << "Argument " << i << ": " << argv[i] << " is invalid." << endl;
			exit(0);
		}
	}
}

void preProcess(string inName){
	ifstream fin(inName);
	ofstream fout("preProcessed.pas");
	string str;
	while (getline(fin,str)){
		for(int i=0;i<str.size();i++){
			if(str[i]>='A'&&str[i]<='Z')
				str[i]+='a'-'A';
		}
		fout << endl << str;
	}
	fin.close();
	fout.close();
}

void outputErrors() {
	if (lexicalErrorInformation.size()) { //如果有词法错误
		cout << "************************Here are the lexical errors***********************" << endl;
		for (int i = 0; i < lexicalErrorInformation.size(); i++)
			cout << lexicalErrorInformation[i] << endl;
		cout << "********************Please correct your lexical errors********************" << endl << endl;
	}
	if (syntaxErrorInformation.size()) { //如果有语法错误
		cout << "************************Here are the syntax errors***********************" << endl;
		for (int i = 0; i < syntaxErrorInformation.size(); i++)
			cout << syntaxErrorInformation[i] << endl;
		cout << "********************Please correct your syntax errors********************" << endl << endl;
	}
	if (semanticWarningInformation.size()) { //如果有语义警告
		cout << "*****************************Here are the semantic warnings****************************" << endl;
		for (int i = 0; i < semanticWarningInformation.size(); i++)
			cout << semanticWarningInformation[i] << endl;
		cout << "********************Please pay attention to these semantic warnings********************" << endl << endl;
	}
	if (semanticErrorInformation.size()) { //如果有语义错误
		cout << "************************Here are the semantic errors***********************" << endl;
		for (int i = 0; i < semanticErrorInformation.size(); i++)
			cout << semanticErrorInformation[i] << endl;
		cout << "********************Please correct your semantic errors********************" << endl;
	}
}