/* abstract-tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "abstract-tree.h"
extern int count_line;       /* from lexer */
extern int count_char;       /* from lexer */
extern int last_token;       /* from lexer */

const char *StringFromKind[] = {
    "Prog",
    "VarDeclListLc",
    "VarDeclListGl",
    "Champs",
    "Condition",
    "Declarateurs",
    "DeclFoncts",
    "DeclFonct",
    "DeclVaSt",
    "Parametres",
    "ListTypVar",
    "EnTeteFonct",
    "Type",
    "ListInstr",
    "DeclVars",
	"Affectation",
    "Corps",
	"Bloc",
    "Instr",
    "SuiteInstr",
	"Void",
    "Exp",
    "TB",
    "FB",
    "M",
    "E",
    "T",
    "F",
    "Struct",
    "LValue",
    "Arguments",
    "ListExp",
    "IntLiteral",
    "CharLiteral",
    "Identifier",
    "AddSubBin",
    "AddSubUn",
    "DivStar",
    "Print",
    "ReadE",
    "ReadC",
    "While",
    "If",
	"IfwElse",
	"ElsewIf",
    "Return",
	"FunctCall",
    "Eq",
    "Or",
	"And",
	"Not",
    "Order",
    "DeclVar"
  /* and all other node labels */
  /* The list must coincide with the enum in abstract-tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

Node *makeNode(Kind kind)
{
    Node *node = malloc(sizeof(Node));
    if (!node)
    {
        printf("Run out of memory\n");
        exit(1);
    }
    node->kind = kind;
    node-> firstChild = node->nextSibling = NULL;
    node->lineno=count_line;
    node->charno = count_char - last_token;
    return node;
}

void addSibling(Node *node, Node *sibling)
{
    Node *curr = node;
    while (curr->nextSibling != NULL)
    {
        curr = curr->nextSibling;
    }
    curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child)
{
    if (parent->firstChild == NULL)
    {
        parent->firstChild = child;
    }
    else
    {
        addSibling(parent->firstChild, child);
    }
}

void deleteTree(Node *node)
{
    if (node->firstChild)
    {
        deleteTree(node->firstChild);
    }
    if (node->nextSibling)
    {
        deleteTree(node->nextSibling);
    }
    free(node);
}

void afficheChar(char c)
{
    switch (c)
    {
        case ('\n') :
            printf(": '\\n'");
            break;
        case ('\a') :
            printf(": '\\a'");
            break;
        case ('\b') :
            printf(": '\\b'");
            break;
        case ('\f') :
            printf(": '\\f'");
            break;
        case ('\r') :
            printf(": '\\r'");
            break;
        case ('\v') :
            printf(": '\\v'");
            break;
        case ('\t') :
            printf(": '\\t'");
            break;
        default :
            printf(": '%c'", c);
            break;
    }
}

void printTreeAux(Node *node)
{
    static bool rightmost[128]; // current node is rightmost sibling
    static int depth = 0;       // depth of current node

    for (int i = 1; i < depth; i++) // 2502 = vertical line
    {
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) // 2514 = up and right; 2500 = horiz; 251c = vertical and right 
    {
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }
    printf("%s", StringFromKind[node->kind]);
    switch (node->kind)
    {
        case IntLiteral: printf(": %d", node->u.integer); break;
        case CharLiteral: afficheChar(node->u.character); break;
        case Identifier: printf(": %s", node->u.identifier); break;
        case Type: printf(": %s", node->u.identifier); break;
        default: break;
    }
    printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling)
    {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTreeAux(child);
    }
    depth--;
}

void printTree(Node *node)
{
    printf("\n\n\n************ ABSTRACT TREE ***************\n\n\n");
    printTreeAux(node);
}
