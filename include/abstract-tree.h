/* abstract-tree.h */
#ifndef __TREE_H__
#define __TREE_H__
typedef enum {
    Prog,
    VarDeclListLc,
    VarDeclListGl,
    Champs,
    Condition,
    Declarateurs,
    DeclFoncts,
    DeclFonct,
    DeclVaSt,
    Parametres,
    ListTypVar,
    EnTeteFonct,
    Type,
    ListInstr,
    DeclVars,
	Affectation,
    Corps,
    Bloc,
    Instr,
    SuiteInstr,
	Void,
    Exp,
    TB,
    FB,
    M,
    E,
    T,
    F,
    Struct,
    LValue,
    Arguments,
    ListExp,
    IntLiteral,
    CharLiteral,
    Identifier,
    AddSubBin,
    AddSubUn,
    DivStar,
    Print,
    ReadE,
    ReadC,
    While,
    If,
	IfwElse,
	ElsewIf,
    Return,
	FunctCall,
    Eq,
    Or,
	And,
	Not,
    Order,
    DeclVar
  /* and allother node labels */
  /* The list must coincide with the strings in abstract-tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} Kind;

typedef struct Node {
  Kind kind;
  union {
    int integer;
    char character;
    char identifier[64];
  } u;
  struct Node *firstChild, *nextSibling;
  int lineno;
  int charno;
} Node;

Node *makeNode(Kind kind);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
#endif
