/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

extern int lineno;       /* from lexer  */
extern int colno;        /* from lexer  */

static const char *StringFromLabel[] = {
    "if",
    "else",
    "while",
    "return",
    "type",
    "void",
    "and",
    "or",
    "eq",
    "negation",
    "order",
    "addsub",
    "divstar",
    "character",
    "num",
    "ident",
    "prog",
    "declVars",
    "declarateurs",
    "decl_foncts",
    "decl_fonct",
    "en_tete_fonct",
    "parametres",
    "list_typ_var",
    "corps",
    "suiteInstr",
    "instr",
    "exp",
    "lvalue",
    "arguments",
    "list_exp",
    "assignation"
};

Node *makeNode(label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(2);
    }
    node->label = label;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    node->colno = colno;
    node->array = false;
    return node;
}

Node *makeNodeWithValue(Value val, label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(2);
    }
    node->label = label;
    node->val = val;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    node->colno = colno;
    node->array = false;
    if (label == Num) {
        node->type = T_INT;
    } else if (label == Character) {
        node->type = T_CHAR;
    }
    return node;
}

void setAsArray(Node* node) {
    node->array = true;
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
            printf("'%c' (Character)", node->val.c);
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
    if (node->array) {
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
