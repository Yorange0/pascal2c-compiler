%{
#include "main.h"
#include "yacc.tab.h"
extern "C"
{
	void yyerror(const char *s);
	int yyparse();
	extern int yylex();
}

void yyerror(const char *s, YYLTYPE *loc);
void yyerror(const char *s, int line, int col);
void yyerror(const char *s, int startLine, int startCol, int endLine, int endCol);

extern int yylineno;
extern char* yytext;
extern char lineBuffer[500];
extern int yyleng;
extern int yycolumn;
extern string itos(int num);

bool haveSemanticError=false;
int rec_line,rec_pos;
char rec_buff[505];

Type* ParseTreeHead=NULL;

vector<string> syntaxErrorInformation; //存放语法错误信息

%}

%token PROGRAM
%token CONST
%token VAR
%token ARRAY
%token OF
%token PROCEDURE
%token FUNCTION
%token _BEGIN
%token END
%token IF
%token THEN
%token FOR
%token TO
%token DO
%token ELSE
%token REPEAT
%token UNTIL
%token WHILE

%token IDENTIFIER
%token UINUM
%token UFNUM
%token CHAR
%token TYPE
%token ASSIGNOP
%token RELOP
%token ADDOP
%token MULOP
%token NOT
%token RANGEDOT

%start programstruct

%left '+' '-' ADD
%left '*' '/' MUL
%right UMINUS
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%nonassoc ONE
%nonassoc TWO
%nonassoc THREE

%locations

%%
programstruct: 	program_head ';' program_body '.'{ //正常
			   		ParseTreeHead=$$=new Type;
			   		$$->token = "programstruct";
			   		$$->children.push_back($1); $$->children.push_back($2);
					$$->children.push_back($3); $$->children.push_back($4);
					if(yylex()) //多余的内容
						yyerror("redundant content at the end!", @4.last_line, @4.last_column+1);
					YYACCEPT;
			   	}|program_head error program_body '.'{ //ERROR 缺少分号 checked
			   		ParseTreeHead=$$=new Type;
			   		$$->token = "programstruct";
					yyerror("missing a semicolon here", @1.last_line, @1.last_column+1);
			   	}|program_head ';' program_body error{ //ERROR 缺少点号 checked
			   		ParseTreeHead=$$=new Type;
			   		$$->token = "programstruct";
					yyerror("missing a dot here", @3.last_line, @3.last_column+1);
			   	}|error ';' program_body '.'{ //ERROR program_head识别失败 checked
			   		ParseTreeHead=$$=new Type;
			   		$$->token = "programstruct";
					yyerror("fatal error in program head, maybe missing keyword \"program\"",@1.first_line, @1.first_column, @1.last_line, @1.last_column);
			   	}|program_head ';' error '.'{ //ERROR program_body识别失败 unchecked
			   		ParseTreeHead=$$=new Type;
			   		$$->token = "programstruct";
					yyerror("fatal error in program body");
			   	}|error program_head ';' program_body '.'{ //ERROR program_head前包含非法字符 checked
					ParseTreeHead=$$=new Type;
					$$->token = "programstruct";
					yyerror("invalid symbol before program head", @$.first_line, @$.first_column, @2.first_line, @2.first_column-1);
				}|error program_head error program_body '.'{ //ERROR program_head前包含非法记号、缺失分号 checked
					ParseTreeHead=$$=new Type;
					$$->token = "programstruct";
					yyerror("invalid token before program head, maybe missing keyword \"program\"", @$.first_line, @$.first_column, @2.first_line, @2.first_column-1);
					yyerror("missing a semicolon here", @2.last_line, @2.last_column+1);
				}|error program_head ';' program_body error{ //ERROR program_head前包含非法记号、缺失点号 checked
					ParseTreeHead=$$=new Type;
					$$->token = "programstruct";
					yyerror("invalid token before program head, maybe missing keyword \"program\"", @$.first_line, @$.first_column, @2.first_line, @2.first_column-1);
					yyerror("missing a dot here", @4.last_line, @4.last_column+1);
				}|error program_head ';' error '.'{ //ERROR program_head前包含非法记号、program_body识别失败 unchecked
					ParseTreeHead=$$=new Type;
					$$->token = "programstruct";
					yyerror("invalid token before program head, maybe missing keyword \"program\"", @$.first_line, @$.first_column, @2.first_line, @2.first_column-1);
					yyerror("fatal error in program body", @3.last_line, @3.last_column+1, @5.first_line, @5.first_column-1);
				};

program_head: 	PROGRAM IDENTIFIER '(' idlist ')'{ //正常
					$$=new Type;
					$$->token = "program_head";
					$$->children.push_back($1); $$->children.push_back($2);
					$$->children.push_back($3); $$->children.push_back($4); $$->children.push_back($5);
				}|PROGRAM error '(' idlist ')'{ //ERROR 缺少主程序名 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("missing program name here", @1.last_line, @1.last_column+1);
				}|PROGRAM IDENTIFIER error idlist ')'{ //ERROR 缺少左括号 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("missing a left bracket here", @4.first_line, @4.first_column-1);
				}|PROGRAM IDENTIFIER '(' error ')'{ //ERROR idlist识别失败 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("program identifier list missing or imcomplete", @4.first_line, @4.first_column, @4.last_line, @4.last_column);
				}|PROGRAM IDENTIFIER '(' idlist error{ //ERROR 缺少右括号 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("missing a right bracket here", @4.last_line, @4.last_column+1);
				}|PROGRAM error{ //ERROR program head checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("program head imcomplete", @1.first_line, @1.first_column, @1.last_line, @1.last_column);
				}|PROGRAM IDENTIFIER error{ //ERROR idlist缺失 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("program identifier list missing or imcomplete", @1.first_line, @1.first_column, @2.last_line, @2.last_column);
				}|PROGRAM IDENTIFIER '(' error{ //ERROR idlist缺失 checked
					$$=new Type;
					$$->token = "program_head";
					yyerror("program identifier list missing or imcomplete", @1.first_line, @1.first_column, @2.last_line, @2.last_column);
				};

program_body: 	const_declarations var_declarations subprogram_declarations compound_statement{ //正常
					$$=new Type;
					$$->token = "program_body";
					$$->children.push_back($1); $$->children.push_back($2);
					$$->children.push_back($3); $$->children.push_back($4);
				};

idlist: idlist ',' IDENTIFIER{ //正常 idlist的产生式不打算加入error
			$$=new Type;
			$$->token = "idlist";
			$$->children.push_back($1); $$->children.push_back($2); $$->children.push_back($3);
		}|IDENTIFIER{ //正常
			$$=new Type;
		   	$$->token = "idlist";
			$$->children.push_back($1);
		};

const_declarations: CONST const_declaration ';' { //正常
						$$=new Type;
						$$->token = "const_declarations";
						$$->children.push_back($1); $$->children.push_back($2); $$->children.push_back($3);
					}|{ //正常
						$$=new Type;
						$$->token = "const_declarations";
					}|CONST error ';' { //ERROR 常量定义出现错误 checked
						$$=new Type;
						$$->token = "const_declarations";
						yyerror("fatal error in const declarations", @2.first_line, @2.first_column, @2.last_line, @2.last_column);
					}|CONST const_declaration error { //ERROR 缺少分号 checked
						$$=new Type;
						$$->token = "const_declarations";
						yyerror("missing a semicolon here", @2.first_line, @2.first_column, @2.last_line, @2.last_column);
					};

const_declaration: 	const_declaration ';' IDENTIFIER '=' const_value{ //正常
						$$=new Type;
						$$->token = "const_declaration";
						$$->children.push_back($1); $$->children.push_back($2);
						$$->children.push_back($3); $$->children.push_back($4); $$->children.push_back($5);
					}|const_declaration ';' IDENTIFIER '=' error{ //常数初始化右值缺失 checked
						$$=new Type;
						$$->token = "const_declaration";
						yyerror("constant definition missing initial r-value", @4.first_line, @4.first_column, @4.last_line, @4.last_column);
					}|IDENTIFIER '=' const_value{ //正常
						$$=new Type;
						$$->token = "const_declaration";
						$$->children.push_back($1); $$->children.push_back($2); $$->children.push_back($3);
					}|IDENTIFIER '=' error{ //常数初始化右值缺失 checked
						$$=new Type;
						$$->token = "const_declaration";
						yyerror("constant definition missing initial r-value", @3.first_line, @3.first_column, @3.last_line, @3.last_column);
					}|const_declaration error IDENTIFIER '=' const_value{ //ERROR 缺少分号 checked
						$$=new Type;
						$$->token = "const_declaration";
						yyerror("missing a semicolon here", @1.first_line, @1.first_column, @1.last_line, @1.last_column+1);
					}|const_declaration ';' IDENTIFIER error const_value{ //ERROR 缺少等号（常量的初始化用的是等号，而不是赋值号） checked
						$$=new Type;
						$$->token = "const_declaration";
						yyerror("missing a equal sign here",@3.first_line, @3.first_column, @3.last_line, @3.last_column);
					}|IDENTIFIER error const_value{ //ERROR 缺少等号（常量的初始化用的是等号，而不是赋值号） checked
						$$=new Type;
						$$->token = "const_declaration";
						yyerror("missing a equal sign here", @2.first_line, @2.first_column, @2.last_line, @2.last_column);
					};

const_value: 	'+' IDENTIFIER { //正常，该非终结符的产生式不打算加入error
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|'-' IDENTIFIER { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|IDENTIFIER { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1);
				}|'+' UINUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|'-' UINUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|UINUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1);
				}|'+' UFNUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|'-' UFNUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1); $$->children.push_back($2);
				}|UFNUM { //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1);
				}|CHAR{ //正常
					$$=new Type;
					$$->token = "const_value";
					$$->children.push_back($1);
				};

var_declarations: 	VAR var_declaration ';'{ //正常
						$$=new Type;
						$$->token = "var_declarations";
						$$->children.push_back($1); $$->children.push_back($2); $$->children.push_back($3);
					}|{ //正常
						$$=new Type;
						$$->token = "var_declarations";
					}|VAR error ';'{ //ERROR 变量定义出现错误 checked
						$$=new Type;
						$$->token = "var_declarations";
						yyerror("fatal error in variant declarations", @1.first_line, @1.first_column, @1.last_line, @1.last_column);
					}|VAR var_declaration error{ //ERROR 缺少分号 checked
						$$=new Type;
						$$->token = "var_declarations";
						yyerror("missing a semicolon here", @2.last_line, @2.last_column+1);
					};

var_declaration: 	var_declaration ';' idlist ':' type { //正常
						$$=new Type;
						$$->token = "var_declaration";
						$$->children.push_back($1);$$->children.push_back($2);
						$$->children.push_back($3); $$->children.push_back($4); $$->children.push_back($5);
					}|idlist ':' type { //正常
						$$=new Type;
						$$->token ="var_declaration";
						$$->children.push_back($1);$$->children.push_back($2); $$->children.push_back($3);
					}|var_declaration error idlist ':' type { //ERROR 缺少分号 checked
						$$=new Type;
						$$->token = "var_declaration";
						yyerror("missing a semicolon here", @1.last_line, @1.last_column+1);
					}|var_declaration ';' idlist error type { //ERROR 缺少冒号 checked
						$$=new Type;
						$$->token = "var_declaration";
						yyerror("missing a colon here", @3.last_line, @3.last_column+1);
					}|var_declaration ';' idlist ':' error { //ERROR type识别失败 checked
						$$=new Type;
						$$->token = "var_declaration";
						yyerror("missing a type here", @4.last_line, @4.last_column+1);
					}|idlist ':' error { //ERROR type识别失败 checked
						$$=new Type;
						$$->token ="var_declaration";
						yyerror("missing a type here", @3.last_line, @3.last_column+1);
					}|idlist error type { //ERROR 缺少分号 checked
						$$=new Type;
						$$->token ="var_declaration";
						yyerror("missing a colon here", @1.last_line, @1.last_column+1);
					};

type: 	TYPE{ //正常
			$$=new Type;
			$$->token = "type";
			$$->children.push_back($1);
		}|ARRAY '[' period ']' OF TYPE{ //正常
			$$=new Type;
			$$->token = "type";
			$$->children.push_back($1);$$->children.push_back($2);
			$$->children.push_back($3);$$->children.push_back($4);
			$$->children.push_back($5);$$->children.push_back($6);
		}|ARRAY error period ']' OF TYPE{ //ERROR 缺少左中括号 checked
			$$=new Type;
			$$->token = "type";
			yyerror("missing a left square bracket here", @1.last_line, @1.last_column+1);
		}|ARRAY '[' period ']' error TYPE{ //ERROR 缺少OF关键字 checked
			$$=new Type;
			$$->token = "type";
			yyerror("missing keyword \"OF\" here", @4.last_line, @4.last_column+1, @6.first_line, @6.first_column-1);
		}|ARRAY '[' period ']' OF error{ //ERROR 数组元素类型识别失败 checked
			$$=new Type;
			$$->token = "type";
			yyerror("missing a base type keyword here", @5.last_line, @5.last_column+1);
		}|ARRAY error{ //ERROR 不完整的数组类型 checked
			$$=new Type;
			$$->token = "type";
			yyerror("incomplete array type", &@$);
		}|ARRAY '[' error{ //ERROR 不完整的数组类型 checked
			$$=new Type;
			$$->token = "type";
			yyerror("incomplete array type", &@$);
		}|ARRAY '[' period error{ //ERROR 不完整的数组类型 checked
			$$=new Type;
			$$->token = "type";
			yyerror("incomplete array type", &@$);
		};

period: period ',' UINUM RANGEDOT UINUM{ //正常
			$$=new Type;
			$$->token="period";
			$$->children.push_back($1);$$->children.push_back($2);
			$$->children.push_back($3);$$->children.push_back($4);$$->children.push_back($5);
		}|period error UINUM RANGEDOT UINUM{ //ERROR 缺少逗号 checked
			$$=new Type;
			$$->token="period";
			yyerror("missing a comma here", @1.last_line, @1.last_column+1);
		}|period ',' UINUM error UINUM{ //ERROR 缺少双点号 checked
			$$=new Type;
			$$->token="period";
			yyerror("missing range dot .. here", @3.last_line, @3.last_column+1);
		}|UINUM RANGEDOT UINUM{ //正常
			$$=new Type;
			$$->token="period";
			$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
		};

subprogram_declarations: 	subprogram_declarations subprogram ';'{ //正常
								$$=new Type;
								$$->token="subprogram_declarations";
								$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
							}|subprogram_declarations subprogram error{ //ERROR 缺少分号 checked
								$$=new Type;
								$$->token="subprogram_declarations";
								yyerror("missing a semicolon here", @2.last_line, @2.last_column+1);
							}|{ //正常
								$$=new Type;
								$$->token ="subprogram_declarations";
							};

subprogram: subprogram_head ';' subprogram_body{ //正常
				$$=new Type;
				$$->token="subprogram";
				$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
			}|subprogram_head error subprogram_body{ //ERROR 缺少分号 checked
				$$=new Type;
				$$->token="subprogram";
				yyerror("missing a semicolon here", @1.last_line, @1.last_column+1);
			};

subprogram_head: 	PROCEDURE IDENTIFIER formal_parameter{ //正常
						$$=new Type;
						$$->token="subprogram_head";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|FUNCTION IDENTIFIER formal_parameter ':' TYPE{ //正常
						$$=new Type;
						$$->token="subprogram_head";
						$$->children.push_back($1);$$->children.push_back($2);
						$$->children.push_back($3);$$->children.push_back($4);$$->children.push_back($5);
					}|FUNCTION error formal_parameter ':' TYPE{ //ERROR 函数名缺失 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("missing function name", @1.last_line, @1.last_column+1);
					}|FUNCTION IDENTIFIER formal_parameter error TYPE{ //ERROR 缺少冒号 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("missing a colon here", @3.last_line, @3.last_column);
					}|FUNCTION IDENTIFIER formal_parameter ':' error{ //ERROR 缺少基本类型关键字 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("missing a base type keyword here", @4.last_line, @4.last_column+1);
					}|FUNCTION IDENTIFIER formal_parameter error{ //ERROR 缺少基本类型关键字 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("missing a base type keyword here", @3.last_line, @3.last_column+1);
					}|FUNCTION error{ //ERROR 不完整的函数头 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("incomplete function head", &@$);
					}|PROCEDURE error{ //ERROR 不完整的过程头 checked
						$$=new Type;
						$$->token="subprogram_head";
						yyerror("incomplete procedure head", &@$);
					};

formal_parameter: 	'(' parameter_list ')'{ //正常
						$$=new Type;
						$$->token="formal_parameter";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|{ //正常
						$$=new Type;
						$$->token="formal_parameter";
					}|'(' error{ //ERROR 不完整的形参列表
						$$=new Type;
						$$->token="formal_parameter";
						yyerror("incomplete formal parameter list", &@$);
					}|'(' parameter_list error{ //ERROR 右括号缺失
						$$=new Type;
						$$->token="formal_parameter";
						yyerror("missing a right bracket here", @2.last_line, @2.last_column+1);
					};

parameter_list: parameter_list ';' parameter{ //正常
					$$=new Type;
					$$->token="parameter_list";
					$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
				}|parameter_list error parameter{ //ERROR 缺少分号 checked
					$$=new Type;
					$$->token="parameter_list";
					yyerror("missing a semicolon here", @1.last_line, @1.last_column+1);
				}|parameter{ //正常
					$$=new Type;
					$$->token="parameter_list";
					$$->children.push_back($1);
				};

parameter: 	var_parameter { //正常，非终结符parameter的产生式不打算加入error
				$$=new Type;
				$$->token="parameter";
				$$->children.push_back($1);
			}|value_parameter{ //正常
				$$=new Type;
				$$->token="parameter";
				$$->children.push_back($1);
			};

var_parameter: 	VAR value_parameter{ //正常
					$$=new Type;
					$$->token="var_parameter";
					$$->children.push_back($1);$$->children.push_back($2);
				}|VAR error{ //ERROR 不完整的引用参数列表 checked
					$$=new Type;
					$$->token="var_parameter";
					yyerror("incomplete refereced parameter list", &@$);
				};

value_parameter: 	idlist ':' TYPE{ //正常
						$$=new Type;
						$$->token="value_parameter";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|idlist error TYPE{ //ERROR 缺少分号 checked
						$$=new Type;
						$$->token="value_parameter";
						yyerror("missing a colon here", @1.first_line, @1.last_column+1);
					}|idlist ':' error{ //ERROR 缺少基本类型关键字 checked
						$$=new Type;
						$$->token="value_parameter";
						yyerror("missing a base type keyword here", @2.last_line, @2.last_column+1);
					}|idlist error{ //ERROR 缺少基本类型关键字 checked
						$$=new Type;
						$$->token="value_parameter";
						yyerror("missing a base type keyword here", @1.last_line, @1.last_column+1);
					};

subprogram_body: 	const_declarations var_declarations compound_statement{ //正常
						$$=new Type;
						$$->token="subprogram_body";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					};

compound_statement: _BEGIN statement_list END{ //正常
						$$=new Type;
						$$->token="compound_statement";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|_BEGIN statement_list error{ //ERROR 缺少END关键字 checked
						$$=new Type;
						$$->token="compound_statement";
						yyerror("missing keyword \"end\"", @2.last_line, @2.last_column+1);
					};

statement_list: statement_list ';' statement{ //正常
					$$=new Type;
					$$->token="statement_list";
					$$->children.push_back($1);$$->children.push_back($2); $$->children.push_back($3);
				}|statement_list error statement{ //ERROR 缺失分号 这里引发了3个规约规约冲突 checked
					$$=new Type;
					$$->token="statement_list";
					yyerror("missing a semicolon here", @1.last_line, @1.last_column+1);
				}|statement{ //正常
					$$=new Type;
					$$->token="statement_list";
					$$->children.push_back($1);
				};

statement: 	variable ASSIGNOP expression{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
			}|procedure_call{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);
			}|compound_statement{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);
			}|IF expression THEN statement else_part{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);$$->children.push_back($2);
				$$->children.push_back($3);$$->children.push_back($4);$$->children.push_back($5);
			}|IF expression error statement else_part{ //ERROR 缺少then关键字 checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing keyword \"then\"", @2.last_line, @2.last_column+1);
			}|FOR IDENTIFIER ASSIGNOP expression TO expression DO statement{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);$$->children.push_back($2);
				$$->children.push_back($3);$$->children.push_back($4);
				$$->children.push_back($5);$$->children.push_back($6);
				$$->children.push_back($7);$$->children.push_back($8);
			}|FOR IDENTIFIER error expression TO expression DO statement{ //ERROR 缺少赋值号 checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing assignop \":=\"", @2.last_line, @2.last_column+1);
			}|FOR IDENTIFIER ASSIGNOP expression error expression DO statement{ //ERROR 缺少关键字to checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing keywrod \"to\"", @4.last_line, @4.last_column+1);
			}|FOR IDENTIFIER ASSIGNOP expression TO expression error statement{ //ERROR 缺少关键字do checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing keywrod \"do\"", @6.last_line, @4.last_column+1);
			}|WHILE expression DO statement{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);$$->children.push_back($2);
				$$->children.push_back($3);$$->children.push_back($4);
			}|WHILE expression error statement{ //ERROR 缺少关键字do checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing keywrod \"do\"", @2.last_line, @2.last_column+1);
			}|REPEAT statement UNTIL expression{ //正常
				$$=new Type;
				$$->token="statement";
				$$->children.push_back($1);$$->children.push_back($2);
				$$->children.push_back($3);$$->children.push_back($4);
			}|REPEAT statement error expression{ //ERROR 缺少关键字until checked
				$$=new Type;
				$$->token="statement";
				yyerror("missing keywrod \"until\"", @4.first_line, @4.first_column);
			}|{ //正常
				$$=new Type;
				$$->token="statement";
			};

else_part: 	ELSE statement{ //正常 非终结符else_part的产生式不打算加error
				$$=new Type;
				$$->token="else_part";
				$$->children.push_back($1);$$->children.push_back($2);
			}|%prec LOWER_THAN_ELSE{ //正常
				$$=new Type;
				$$->token="else_part";
			};

variable: 	IDENTIFIER id_varpart{ //正常
				$$=new Type;
				$$->token="variable";
				$$->children.push_back($1);$$->children.push_back($2);
			};

id_varpart: '[' expression_list ']'{ //正常
				$$=new Type;
				$$->token="id_varpart";
				$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
			}|'[' error{ //ERROR 不完整的数组下标列表 checked
				$$=new Type;
				$$->token="id_varpart";
				yyerror("incomplete expression list of array subindex", &@$);
			}|'[' expression_list error{ //ERROR 缺失右中括号 checked
				$$=new Type;
				$$->token="id_varpart";
				yyerror("missing a right square bracket here", @2.last_line, @2.last_column+1);
			}|{ //正常
				$$=new Type;
				$$->token="id_varpart";
			};

procedure_call: IDENTIFIER{ //正常
				$$=new Type;
				$$->token="procedure_call";
				$$->children.push_back($1);
			}|IDENTIFIER '(' expression_list ')'{ //正常
				$$=new Type;
				$$->token="procedure_call";
				$$->children.push_back($1);$$->children.push_back($2);
				$$->children.push_back($3);$$->children.push_back($4);
			}|IDENTIFIER '(' expression_list error{ //ERROR 缺少右括号 checked
				$$=new Type;
				$$->token="procedure_call";
				yyerror("missing a right bracket here", @3.last_line, @3.last_column+1);
			};

expression_list: 	expression_list ',' expression{ //正常
						$$=new Type;
						$$->token="expression_list";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|expression{ //正常
						$$=new Type;
						$$->token="expression_list";
						$$->children.push_back($1);
					}|expression_list error expression{ //ERROR 缺少逗号 这里引发了一个移进规约冲突 checked
						$$=new Type;
						$$->token="expression_list";
						yyerror("missing a comma here", @1.last_line, @1.last_column+1);
					};

expression: simple_expression RELOP simple_expression{ //正常
				$$=new Type;
				$$->token="expression";
				$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
			}|simple_expression '=' simple_expression{ //正常
				$$=new Type;
				$$->token="expression";
				$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
			}|simple_expression{ //正常
				$$=new Type;
				$$->token="expression";
				$$->children.push_back($1);
			};

simple_expression: 	simple_expression ADDOP term{ //正常
						$$=new Type;
						$$->token="simple_expression";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|simple_expression ADDOP error term %prec ADD{//error，缺少操作数
						$$=new Type;
						$$->token="simple_expression";
						yyerror("missing operand",@2.last_line, @2.last_column+1);
					}|simple_expression '-' term{ //正常
						$$=new Type;
						$$->token="simple_expression";
						$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
					}|simple_expression '-' error term %prec ADD{//error，缺少操作数
						$$=new Type;
						$$->token="simple_expression";
						yyerror("missing operand",@2.last_line, @2.last_column+1);
					}|term{ //正常
						$$=new Type;
						$$->token="simple_expression";
						$$->children.push_back($1);
					};

term: 	term MULOP factor{ //正常
			$$=new Type;
			$$->token="term";
			$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
		}|term MULOP error factor %prec MUL{ //error,缺少操作数
			$$=new Type;
			$$->token="term";
			yyerror("missing operand",@2.last_line, @2.last_column+1);
		}|factor{ //正常
			$$=new Type;
			$$->token="term";
			$$->children.push_back($1);
		};

factor: UINUM{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);
		}|UFNUM{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);
		}|variable{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);
		}|IDENTIFIER '(' expression_list ')'{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);$$->children.push_back($2);
			$$->children.push_back($3);$$->children.push_back($4);
		}|IDENTIFIER '(' expression_list error{ //ERROR 缺少右括号 这里引发了一个移进规约冲突
			$$=new Type;
			$$->token="factor";
			yyerror("missing a right bracket here", @3.last_line, @3.last_column+1);
		}|IDENTIFIER '(' error{ //ERROR 函数调用的表达式列表缺失
			$$=new Type;
			$$->token="factor";
			yyerror("missing actual parameter list of function call", @2.last_line, @2.last_column+1);
		}|'(' expression ')'{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);$$->children.push_back($2);$$->children.push_back($3);
		}|'(' expression error{ //ERROR 缺少右括号
			$$=new Type;
			$$->token="factor";
			yyerror("missing a right bracket here", @2.last_line, @2.last_column+1);
		}|NOT factor{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);$$->children.push_back($2);
		}|'-' factor{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);$$->children.push_back($2);
		}|CHAR{ //正常
			$$=new Type;
			$$->token="factor";
			$$->children.push_back($1);
		};

%%


void yyerror(const char *s){
	haveSemanticError = true;//错误标志，含有语法错误
	string errorInformation;//定义错误信息
	errorInformation += string(s);//添加错误信息
	errorInformation += ", location: " + itos(yylineno-1) + "." + itos(yycolumn-yyleng);//添加错误位置
	syntaxErrorInformation.push_back(errorInformation);//存放错误信息
	CHECK_ERROR_BOUND
}

void yyerror(const char *s, YYLTYPE *loc){//处理单个字符的错误
	haveSemanticError = true;
	string errorInformation;
	errorInformation = "syntax error, " + string(s) + ", location: " + itos(loc->first_line) + "." + itos(loc->first_column) + "-" + itos(loc->last_line) + "." + itos(loc->last_column);
	syntaxErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void yyerror(const char *s, int line, int col){//处理一行以内的错误
	haveSemanticError = true;
	string errorInformation;
	errorInformation = "syntax error, " + string(s) + ", location: " + itos(line) + "." + itos(col);
	syntaxErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}

void yyerror(const char *s, int startLine, int startCol, int endLine, int endCol){//处理涉及多行的错误
	haveSemanticError = true;
	string errorInformation;
	errorInformation = "syntax error, " + string(s) + ", location: " + itos(startLine) + "." + itos(startCol) + "-" + itos(endLine) + "." + itos(endCol);
	syntaxErrorInformation.push_back(errorInformation);
	CHECK_ERROR_BOUND
}
