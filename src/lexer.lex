%{
#include "tree.h"
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "args.h"

void yyerror(char* msg);
int yylex(void);    

void update_cursor(void);


int lineno = 1;
int colno = 0;
int prevcolno;

%}

%option nounput
%option noinput
%option noyywrap


%x MULCOM 

separator [ \t\r\n]

%%
"/*" BEGIN MULCOM;                  { prevcolno = colno; colno += yyleng; }
<MULCOM>.                           { prevcolno++; colno++; }
<MULCOM>\n                          { lineno++; colno = 0; }
<MULCOM>"*/" BEGIN INITIAL;         { prevcolno = colno; colno += yyleng; }
"//".*                              { prevcolno = colno; colno = 0; }
if                                  { strcpy(yylval.ident, yytext); update_cursor(); return IF; }
else                                { strcpy(yylval.ident, yytext); update_cursor(); return ELSE; }
while                               { strcpy(yylval.ident, yytext); update_cursor(); return WHILE; }
return                              { strcpy(yylval.ident, yytext); update_cursor(); return RETURN; }
(int|char)                          { strcpy(yylval.ident, yytext); update_cursor(); return TYPE; }
void                                { strcpy(yylval.ident, yytext); update_cursor(); return VOID; }
&&                                  { strcpy(yylval.ident, yytext); update_cursor(); return AND; }
"||"                                { strcpy(yylval.ident, yytext); update_cursor(); return OR; }
(==|!=)                             { strcpy(yylval.ident, yytext); update_cursor(); return EQ; }
(<|<=|>|>=)                         { strcpy(yylval.ident, yytext); update_cursor(); return ORDER; }
(\+|-)                              { strcpy(yylval.ident, yytext); update_cursor(); return ADDSUB; }
(\*|\/|%)                           { strcpy(yylval.ident, yytext); update_cursor(); return DIVSTAR; }
[;,(){}[\]]                         { prevcolno = colno; colno += yyleng; return yytext[0]; }
'[^\\]'|'[\\][ntr]'                 { strcpy(yylval.carac, yytext); update_cursor(); return CHARACTER; }
0                                   { yylval.num = atoi(yytext); update_cursor(); return NUM; }
[1-9][0-9]*                         { yylval.num = atoi(yytext); update_cursor(); return NUM; }
[a-zA-Z_][a-zA-Z_0-9]*              { strcpy(yylval.ident, yytext); update_cursor(); return IDENT; }
\n                                  { prevcolno = colno; colno = 0; lineno++;  }
{separator}                         { prevcolno = colno; colno += yyleng; }
.                                   { prevcolno = colno; colno++; return yytext[0]; }
<<EOF>>                             { return 0; }
%%

// Update the cursor in the file
void update_cursor(void) {
    prevcolno = colno;
    colno += yyleng;
}
