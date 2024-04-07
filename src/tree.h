#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

#define IDENT_LEN 64

typedef enum {
    If,
    Else,
    While,
    Return,
    Type,
    Void,
    And,
    Or,
    Eq,
    Negation,
    Order,
    AddSub,
    DivStar,
    Character,
    Num,
    Ident,
    Prog,
    DeclVars,
    Declarateurs,
    DeclFoncts,
    DeclFonct,
    EnTeteFonct,
    Parametres,
    ListTypVar,
    Corps,
    SuiteInstr,
    Instr,
    Exp,
    LValue,
    Arguments,
    ListExp,
    Assignation
} label_t;

typedef enum {
    NUMERIC,
    CHAR,
    IDENTIFIER,
    NONE
} ValueType;

typedef union {
    int num;
    char c;
    char ident[IDENT_LEN];
} Value;

typedef struct Node {
    bool array;
    int colno;
    int lineno;
    label_t label;
    ValueType type;
    Value val;
    struct Node *firstChild, *nextSibling;
} Node;

Node *makeNode(label_t label);

/**
 * @brief Make a node which contains a value
 * 
 * @param val 
 * @param type 
 * @return Node* 
 */
Node *makeNodeWithValue(Value val, ValueType type);
void setAsArray(Node* node);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);


#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

#endif
