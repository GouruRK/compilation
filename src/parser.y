%{
#include <stdio.h>
#include <string.h>

#include "tree.h"
#include "args.h"
#include "parser.h"

int yylex(void);
void yyerror(Node** tree, char* msg);

extern int lineno;
extern int colno;
extern int prevcolno;
extern FILE *yyin;

/**
 * @brief Set a value with the type of int
 */
Value to_int(int n);

/**
 * @brief Set a value with the type of char
 */
Value to_char(char c);

/**
 * @brief Set a value with the type of char*
 */
Value to_str(char* str);

%}
%union{
    Node *node;
    char ident[64];
    char carac;
    int num;
}

%parse-param {Node** tree}

%type <node> Prog DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr Instr Exp TB FB M E T F LValue ListExp Arguments

%type <key_word> IF WHILE RETURN ELSE
%type <ident> TYPE VOID AND OR EQ ORDER ADDSUB DIVSTAR IDENT
%type <carac> CHARACTER
%type <num> NUM  

%token IF WHILE RETURN ELSE TYPE VOID AND OR EQ ORDER ADDSUB DIVSTAR CHARACTER IDENT NUM 

%expect 1
%%
Prog:  DeclVars DeclFoncts                  { Node* prog = makeNode(Prog);
                                             addChild(prog, $1);
                                             addChild(prog, $2);
                                             *tree = prog;
                                             }
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'       { Node* t = makeNodeWithValue(to_str($2), Type);
                                              addChild(t, $3);
                                              addChild($1, t); }
    |                                       { $$ = makeNode(DeclVars); }
    ;
Declarateurs:
       Declarateurs ',' IDENT               { addSibling($$, makeNodeWithValue(to_str($3), Ident)); }  
    |  Declarateurs ',' IDENT '[' NUM ']'   { Node* t = makeNodeWithValue(to_str($3), Ident);
                                              setAsArray(t);
                                              addChild(t, makeNodeWithValue(to_int($5), Num));
                                              addSibling($$, t); }
    |  IDENT '[' NUM ']'                    { $$ = makeNodeWithValue(to_str($1), Ident);
                                              setAsArray($$);
                                              addChild($$, makeNodeWithValue(to_int($3), Num)); }
    |  IDENT                                { $$ = makeNodeWithValue(to_str($1), Ident); }
    ;
DeclFoncts:
       DeclFoncts DeclFonct                 { $$ = $1;
                                              Node* node = makeNode(DeclFonct);
                                              addChild(node, $2);
                                              addChild($$, node); }
    |  DeclFonct                            { $$ = makeNode(DeclFoncts);
                                              addChild($$, makeNode(DeclFonct));
                                              addChild(FIRSTCHILD($$), $1); }
    ;
DeclFonct:
       EnTeteFonct Corps                    { $$ = makeNode(EnTeteFonct);
                                              addChild($$, $1);
                                              addSibling($$, $2); }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'        { $$ = makeNodeWithValue(to_str($1), Type);
                                              addSibling($$, makeNodeWithValue(to_str($2), Ident));
                                              addSibling($$, $4); }
|      VOID IDENT '(' Parametres ')'        { $$ = makeNodeWithValue(to_str($1), Void);
                                              addSibling($$, makeNodeWithValue(to_str($2), Ident));
                                              addSibling($$, $4); }
    ;
Parametres:
       VOID                                 { $$ = makeNodeWithValue(to_str($1), Void); }
    |  ListTypVar                           { $$ = makeNode(ListTypVar);
                                              addChild($$, $1); }
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT            { $$ = $1;
                                              Node* t = makeNodeWithValue(to_str($3), Type);
                                              addChild(t, makeNodeWithValue(to_str($4), Ident));
                                              addSibling($$, t); }
    |  ListTypVar ',' TYPE IDENT '[' ']'    { $$ = $1;
                                              Node* t = makeNodeWithValue(to_str($3), Type);
                                              Node* ident = makeNodeWithValue(to_str($4), Ident);
                                              setAsArray(ident);
                                              addChild(t, ident);
                                              addSibling($$, t); }
    |  TYPE IDENT '[' ']'                   { $$ = makeNodeWithValue(to_str($1), Type);
                                              Node* ident = makeNodeWithValue(to_str($2), Ident);
                                              setAsArray(ident);
                                              addChild($$, ident); }
    |  TYPE IDENT                           { $$ = makeNodeWithValue(to_str($1), Type);
                                              addChild($$, makeNodeWithValue(to_str($2), Ident)); }
    ;

Corps: '{' DeclVars SuiteInstr '}'          { $$ = makeNode(Corps);
                                              addChild($$, $2);
                                              addSibling($2, $3);}
    ;
SuiteInstr:
       SuiteInstr Instr                     { $$ = $1;
                                              addChild($$, $2); }
    |                                       { $$ = makeNode(SuiteInstr); }
    ;
Instr:
       LValue '=' Exp ';'                   { $$ = makeNodeWithValue(to_char('='), Assignation);
                                              addChild($$, $1);
                                              addSibling(FIRSTCHILD($$), $3); }
    |  IF '(' Exp ')' Instr                 { $$ = makeNode(If);
                                              addChild($$, $3);
                                              addSibling($3, $5); }
    |  IF '(' Exp ')' Instr ELSE Instr      { $$ = makeNode(If);
                                              addChild($$, $3);
                                              addSibling($3, $5);
                                              Node* e = makeNode(Else);
                                              addSibling($$, e);
                                              addChild(e, $7); }
    |  WHILE '(' Exp ')' Instr              { $$ = makeNode(While);
                                              addChild($$, $3);
                                              addChild($$, $5); }
    |  IDENT '(' Arguments ')' ';'          { $$ = makeNodeWithValue(to_str($1), Ident);
                                              addChild($$, $3); }
    |  IDENT '[' Exp ']' ';'                { $$ = makeNodeWithValue(to_str($1), Ident);
                                              addChild($$, $3); }
    |  RETURN Exp ';'                       { $$ = makeNode(Return);
                                              addChild($$, $2); }
    |  RETURN ';'                           { $$ = makeNode(Return); }
    |  '{' SuiteInstr '}'                   { $$ = $2; }
    |  ';'                                  { ; }
    ;
Exp :  Exp OR TB                            { Node* n = makeNodeWithValue(to_str($2), Or);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; }
    |  TB                                   { $$ = $1; }
    ;
TB  :  TB AND FB                            { Node* n = makeNodeWithValue(to_str($2), And);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; }
    |  FB                                   { $$ = $1; }
    ;
FB  :  FB EQ M                              { Node* n = makeNodeWithValue(to_str($2), Eq);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; }
    |  M                                    { $$ = $1; }
    ;
M   :  M ORDER E                            { Node* n = makeNodeWithValue(to_str($2), Order);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; }
    |  E                                    { $$ = $1; }
    ;
E   :  E ADDSUB T                           { Node* n = makeNodeWithValue(to_str($2), AddSub);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; }
    |  T                                    { $$ = $1; }
    ;    
T   :  T DIVSTAR F                          { Node* n = makeNodeWithValue(to_str($2), DivStar);
                                              addChild(n, $1);
                                              addChild(n, $3);
                                              $$ = n; } 
    |  F                                    { $$ = $1; }
    ;
F   :  ADDSUB F                             { $$ = makeNodeWithValue(to_str($1), AddSub); 
                                              addChild($$, $2); }
    |  '!' F                                { $$ = makeNodeWithValue(to_char('!'), Negation); 
                                              addChild($$, $2); }
    |  '(' Exp ')'                          { $$ = $2; }
    |  NUM                                  { $$ = makeNodeWithValue(to_int($1), Num); }
    |  CHARACTER                            { $$ = makeNodeWithValue(to_char($1), Character); }
    |  LValue                               { $$ = $1; }
    |  IDENT '(' Arguments  ')'             { $$ = makeNodeWithValue(to_str($1), Ident);
                                              addChild($$, $3); }
    ;
LValue:
       IDENT                                { $$ = makeNodeWithValue(to_str($1), Ident); }
    |  IDENT '[' Exp ']'                    { $$ = makeNodeWithValue(to_str($1), Ident); 
                                              addChild($$, $3); }
    ;
Arguments:
       ListExp                              { $$ = makeNode(ListExp); 
                                              addChild($$, $1); }
    |                                       { $$ = makeNode(NoParametres); }
    ;
ListExp:
       ListExp ',' Exp                      { $$ = $1;
                                              addSibling($$, $3); }
    |  Exp                                  { $$ = $1; }
    ;
%%

Value to_str(char* val) {
    Value v;
    strcpy(v.ident, val);
    return v;
}

Value to_int(int n) {
    return (Value){.num = n};
}

Value to_char(char c) {
    return (Value){.c = c};
}

/**
 * @brief Print error with the line and column where the error was triggered
 */
void yyerror(Node** tree, char* msg) {
    fprintf(stderr, "%s at %d:%d\n", msg, lineno, prevcolno);
}

int parse_file(FILE* source, Node** tree) {
    yyin = source;
    return yyparse(tree);
}
