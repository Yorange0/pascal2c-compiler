del lex.yy.c
flex lex.l
del lex.yy.cpp
ren lex.yy.c lex.yy.cpp
del yacc.tab.c
bison -vd --debug yacc.y
del yacc.tab.cpp
ren yacc.tab.c yacc.tab.cpp
g++ lex.yy.cpp yacc.tab.cpp main.cpp -o pascal2c.exe
pascal2c.exe
pause