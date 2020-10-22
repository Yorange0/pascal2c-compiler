flex lex.l
mv lex.yy.c lex.yy.cpp
bison -v -d --debug yacc.y
mv yacc.tab.c yacc.tab.cpp
g++ -o test yacc.tab.cpp lex.yy.cpp main.cpp