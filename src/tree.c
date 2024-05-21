#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

extern int lineno;       // from lexer
extern int colno;        // from lexer

static const char *StringFromLabel[] = {
    [If] = "if",
    [Else] = "else",
    [While] = "while",
    [Return] = "return",
    [Type] = "type",
    [Void] = "void",
    [And] = "and",
    [Or] = "or",
    [Eq] = "eq",
    [Negation] = "negation",
    [Order] = "order",
    [AddSub] = "addsub",
    [DivStar] = "divstar",
    [Character] = "character",
    [Num] = "num",
    [Ident] = "ident",
    [Prog] = "prog",
    [DeclVars] = "declVars",
    [Declarateurs] = "declarateurs",
    [DeclFoncts] = "decl_foncts",
    [DeclFonct] = "decl_fonct",
    [EnTeteFonct] = "en_tete_fonct",
    [Parametres] = "parametres",
    [NoParametres] = "no_parametres",
    [ListTypVar] = "list_typ_var",
    [Corps] = "corps",
    [SuiteInstr] = "suiteInstr",
    [Instr] = "instr",
    [Exp] = "exp",
    [LValue] = "lvalue",
    [Arguments] = "arguments",
    [ListExp] = "list_exp",
    [Assignation] = "assignation", 
    [EmptyInstr] = "empty_instr"
};

Node *makeNode(label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(3);
    }
    node->label = label;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    node->colno = colno;
    node->type = T_NONE;
    return node;
}

Node *makeNodeWithValue(Value val, label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(3);
    }
    node->label = label;
    node->val = val;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    node->colno = colno;
    node->type = T_NONE;
    return node;
}

void setAsArray(Node* node) {
    node->type = T_ARRAY;
}

void addSibling(Node *node, Node *sibling) {
    Node *curr = node;
    while (curr->nextSibling != NULL) {
        curr = curr->nextSibling;
    }
    curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    }
    else {
        addSibling(parent->firstChild, child);
    }
}

void deleteTree(Node *node) {
    if (!node) {
        return;
    }
    if (node->firstChild) {
        deleteTree(node->firstChild);
    }
    if (node->nextSibling) {
        deleteTree(node->nextSibling);
    }
    free(node);
}

/**
 * @brief Fonction display the value of a node
 * 
 * @param node 
 */
static void printNode(Node* node) {
    switch (node->label) {
        case Num:
            printf("%d (Num)", node->val.num);
            break;
        case Character:
            printf("%s (Character)", node->val.ident);
            break;
        case Ident: case Type: case Or: case And:
        case Eq: case Order: case DivStar: case AddSub:
        case Negation: case Assignation:
            printf("%s (%s)", node->val.ident, StringFromLabel[node->label]);
            break;
        default:
            printf("%s", StringFromLabel[node->label]);
            break;
    }
    if (is_array(node->type)) {
        puts("[]");
    } else {
        putchar('\n');
    }
}

void printTree(Node *node) {
    static bool rightmost[128]; // tells if node is rightmost sibling
    static int depth = 0;       // depth of current node
    for (int i = 1; i < depth; i++) { // 2502 = vertical line
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }
    printNode(node);
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
