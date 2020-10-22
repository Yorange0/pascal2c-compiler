#include "main.h"
#include <fstream>
#include <sstream>

extern Type* ParseTreeHead;
extern FILE* yyin;
extern vector<string> syntaxErrorInformation;

extern "C"{
    int yyparse();
}

string process(string str);
void dfs(Type* now);
bool outputSyntaxErrorInformation();

int main()
{

#ifdef _WIN32
	string inName="PascalProgram.pas";
	string outName="preProcessed.pas";
#elif __APPLE__
	string inName="/Users/mac/yacc_and_lex_repository/lex_and_yacc/PascalProgram.pas";
	string outName="/Users/mac/yacc_and_lex_repository/lex_and_yacc/preProcessed.pas";
#endif

	ifstream fin(inName.c_str());
	ofstream fout(outName.c_str());
	string str;
	while (getline(fin,str))
		fout << endl << process(str);
	fin.close();
	fout.close();

	yydebug=1;

#ifdef _WIN32
	const char* sFile = "preProcessed.pas";
#elif __APPLE__
	const char *sFile = "/Users/mac/yacc_and_lex_repository/lex_and_yacc/preProcessed.pas";
#endif
	FILE* fp = fopen(sFile,"r");
	if(fp==NULL){
		printf("cannot open %s\n",sFile);
		return -1;
	}
	yyin=fp;

	printf("-----begin parsing %s\n",sFile);
	yyparse();
	printf("-----end parsing\n");
    if(ParseTreeHead!=NULL)
        //dfs(ParseTreeHead);

	fclose(fp);

	outputSyntaxErrorInformation();

	return 0;
}

string process(string str) {//由于PASCAL大小写不敏感，所以需要将所有字母转化为小写
	for (int i = 0; i<str.size(); i++) {
		if (str[i] >= 'A'&&str[i] <= 'Z')
			str[i] = str[i] + ('a' - 'A');
	}
	return str;
}

void dfs(Type* now){
    if(now->children.size()==0){
		if(now->str=="")
			cout << now->token << "\t->\t" << "empty" << endl;
        return;
    }
    cout << now->token << "\t->";
	for (int i = 0; i < now->children.size(); i++) {
		if (now->children[i]->children.size()==0 && now->children[i]->str != "")
			cout << "\t\"" << now->children[i]->str << "\"";
		else
			cout << "\t" << now->children[i]->token;
	}
    cout << endl;
    for(int i=0;i<now->children.size();i++)
        dfs(now->children[i]);
}

string itos(int num){
	stringstream sin;
	sin<<num;
	return sin.str();
}

bool outputSyntaxErrorInformation(){
	if(!syntaxErrorInformation.size())
		return false;
	cout << endl << "Here is the syntax error information" << endl;
	for(int i=0;i<syntaxErrorInformation.size();i++)
		cout << syntaxErrorInformation[i] << endl;
}
