/* decl-var.h */
/* Analyse descendante récursive des déclarations de variables en C */
#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#include "abstract-tree.h"
#define MAXNAME 1024
#define MAXSYMBOLS 256
#define MAXTYPENAME 64
typedef struct
{
    char name[MAXNAME];
    char type[MAXTYPENAME];
    int offset;
    int sizeTab;
} TabParCha;

typedef struct {
    char name[MAXNAME];
    char type[MAXTYPENAME];
    int fonction;
    int offset;
    TabParCha parcha[MAXSYMBOLS];
} STentry;

#define INTEGER 0
#define REAL 1

#define CHAR 255
#define INT 256

char *recup_type(Node *exp);
char *recupType(char *name);

TabParCha *recupChamp(char *name, char *type);
TabParCha *recupParaChamp(char *name, char *type, int fonction);

void addVar(const char name[], int type);
void syntaxError();
void principal(Node *abstree);
void printTable(int size, STentry *tab, char *name);

int recup_offset(char *ident);
int recup_offset_struct(char *name, char *cha);
int recup_postruct(char *name);
int inTableLc(char *name);
int inTableGl(char *name);
int estStruct(char *type);

#endif
