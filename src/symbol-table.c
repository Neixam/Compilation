/* decl-var.c */
/* Traduction descendante récursive des déclarations de variables en C */
/* Compatible avec decl-var.lex */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol-table.h"
#include "as.tab.h"
#include "traduction.h"
extern int count_line;  /* from lexical analyser */
extern int count_char;
extern int opt;
extern translation Translation[];

#define BUF_SIZE 1024
STentry *symbolTableGl;
STentry *symbolTableLc;
int STmax=MAXSYMBOLS; /* maximum size of symbol table */
int STsizeGl=0;         /* size of symbol table */
int STsizeLc=0;         /* size of symbol table */

int offset = 1;
int nbstruct = 0;
int ret_rec = 0;
int one_time = 0;
Node *racine;

int estStruct(char *type)
{
    if (!strcmp(type, "int") || !strcmp(type, "char") || !strcmp(type, "struct")
        || !strcmp(type, "void"))
        return (0);
    return (1);
}

char *recupType(char *name)
{
    int i;

    for (i = 0; i < STsizeLc; i++)
        if (!strcmp(symbolTableLc[i].name, name))
            return (symbolTableLc[i].type);
    for (i = 0; i < STsizeGl; i++)
        if (!(strcmp(symbolTableGl[i].name, name)))
            return (symbolTableGl[i].type);
    return (NULL);
}

TabParCha *recupParaChamp(char *name, char *type, int fonction)
{
    int i;

    for (i = 0; i < STsizeGl; i++)
        if (!strcmp(symbolTableGl[i].name, name) &&
            !strcmp(symbolTableGl[i].type, type) &&
            symbolTableGl[i].fonction == fonction)
            return (symbolTableGl[i].parcha);
    return (NULL);
}

TabParCha *recupChamp(char *name, char *type)
{
    int i;

    for (i = 0; i < STsizeLc; i++)
        if (!strcmp(symbolTableLc[i].name, name) &&
            !strcmp(symbolTableLc[i].type, type) &&
            symbolTableLc[i].fonction == 0)
            return (symbolTableLc[i].parcha);
    return (NULL);
}

char *recupTypeStruct(Node *Struct)
{
    TabParCha   *champs;
    int         i;

    champs = recupParaChamp(recupType(Struct->firstChild->u.identifier), "struct", 0);
    for (i = 0; i < champs->sizeTab; i++)
        if (!strcmp(champs[i].name, SECONDCHILD(Struct)->u.identifier))
            return (champs[i].type);
    fprintf(stderr, "error: the member '%s' is undifined in line %d, character %d\n",
        SECONDCHILD(Struct)->u.identifier, count_line, Struct->charno);
    exit(2);
    return (NULL);
}

void addVarGl(const char name[], char *type, int fonction){
    int count;
    for (count=0;count<STsizeGl;count++) {
        if (!strcmp(symbolTableGl[count].name,name)) {
            fprintf(stderr, "error: redefinition of variable %s near line %d, character %d\n",
            name, count_line, count_char);
            exit(2);
        }
    }
    if (++STsizeGl>STmax) {
        fprintf(stderr, "too many variables near line %d, character %d\n",
            count_line, count_char);
        exit(2);
    }
    symbolTableGl[STsizeGl - 1].offset = nbstruct;
    if (!strcmp(type, "struct"))
        nbstruct++;
    strcpy(symbolTableGl[STsizeGl-1].name, name);
    strcpy(symbolTableGl[STsizeGl-1].type, type);
    symbolTableGl[STsizeGl - 1].parcha->sizeTab = 0;
    symbolTableGl[STsizeGl - 1].fonction = fonction;
}

int recup_postruct(char *name)
{
    int     i;

    for (i = 0; i < STsizeGl; i++)
        if (!strcmp(symbolTableGl[i].name, name))
            return (symbolTableGl[i].offset);
    fprintf(stderr, "error in line %d, character %d : '%s' not defined\n",
        count_line, count_char, name);
    exit(2);
}

void addVarLc(const char name[], char *type){
    int count;
    TabParCha *cha;
    for (count=0;count<STsizeLc;count++) {
        if (!strcmp(symbolTableLc[count].name,name)) {
            fprintf(stderr, "error: redefinition of variable %s near line %d, character %d\n",
            name, count_line, count_char);
            exit(2);
        }
    }
    if (++STsizeLc>STmax) {
        fprintf(stderr, "too many variables near line %d, character %d\n", count_line, count_char);
        exit(3);
    }
    strcpy(symbolTableLc[STsizeLc-1].name, name);
    strcpy(symbolTableLc[STsizeLc-1].type, type);
    symbolTableLc[STsizeLc - 1].offset = offset * 8;
    if (estStruct(type))
    {
        cha = recupParaChamp(type, "struct", 0);
        for (count = 0; count < cha->sizeTab; count++)
        {
            strcpy(symbolTableLc[STsizeLc - 1].parcha[count].name, cha[count].name);
            strcpy(symbolTableLc[STsizeLc - 1].parcha[count].type, cha[count].type);
            symbolTableLc[STsizeLc - 1].parcha[count].sizeTab = cha->sizeTab;
            symbolTableLc[STsizeLc - 1].parcha[count].offset = offset * 8;
            if (offset > 0)
                offset++;
            else
                offset--;
        }
    }
    else
    {
        if (offset > 0)
            offset++;
        else
            offset--;
    }
}

void syntaxError(){
    fprintf(stderr, "syntax error near line %d, character %d\n", count_line, count_char);
    exit(2);
}

char *recup_type(Node *exp)
{
    if (exp->kind == LValue)
    {
        if (exp->firstChild->kind == Struct)
            return (recupTypeStruct(exp->firstChild));
        return (recupType(exp->firstChild->u.identifier));
    }
    if (exp->kind == CharLiteral)
        return ("char");
    return ("int");
}

int structInTab(STentry *tab, char *name, char *type, int size)
{
    int     i;
    
    for (i = 0; i < size; i++)
        if (!strcmp(tab[i].name, name) && !strcmp(tab[i].type, type))
            return (1);
    return (0);
}

void construct_symboltabLc(Node *tree)
{
	Node	*VarList;
	char	type[MAXTYPENAME];

	if (!tree)
		return ;
    count_line = tree->lineno;
    count_char = tree->charno;
    if (tree->kind == Parametres)
        offset = 2;
    if (tree->kind == VarDeclListLc)
        offset = -1;
	if (tree->kind == Type)
	{
        if (tree->firstChild->kind == Struct)
        {
            strcpy(type, tree->firstChild->firstChild->u.identifier);
            if (!structInTab(symbolTableGl, type, "struct", STsizeGl))
            {
                fprintf(stderr, "error: the struct '%s' undifined near line %d, character %d\n",
                    type, count_line, FIRSTCHILD(tree)->charno);
                exit(2);
            }
        }
        else
            strcpy(type, tree->u.identifier);
        for (VarList = tree->firstChild; VarList; VarList = VarList->nextSibling)
            if (VarList->kind != Struct)
                addVarLc(VarList->u.identifier, type);
        return (construct_symboltabLc(tree->nextSibling));
	}
	construct_symboltabLc(tree->firstChild);
	construct_symboltabLc(tree->nextSibling);
}

int compareType(Node *Value, char *type)
{
    int     ent;
    int     car;

    /* Implémentez un Warning pour le cast char -> int */
    if (Value->kind == IntLiteral || Value->kind == CharLiteral)
    {
        if (!(ent = strcmp(type, "int")) || !(car = strcmp(type, "char")))
        {
            if (ent == 0 && Value->kind == CharLiteral)
                fprintf(stderr, "warning in line %d, character %d : implicite cast from 'char' to 'int'\n",
                    count_line, count_char);
            else if (Value->kind == IntLiteral && ent != 0)
                fprintf(stderr, "warning in line %d, character %d : implicite cast from 'int' to 'char'\n",
                    count_line, count_char);
            return (0);
        }
        return (1);
    }
    if (Value->firstChild->kind == Struct)
        return (!recupTypeStruct(Value->firstChild) || estStruct(type));
    if (estStruct(type))
        return (!estStruct(recupType(Value->firstChild->u.identifier)));
    return (estStruct(recupType(Value->firstChild->u.identifier)));
}

int inTable(char *name, STentry *table, int size)
{
    int     i;
    
    for (i = 0; i < size; i++)
        if (!strcmp(table[i].name, name))
            return (1);
    return (0);
}

int inTableGl(char *name)
{
    return (inTable(name, symbolTableGl, STsizeGl));
}

int inTableLc(char *name)
{
    return (inTable(name, symbolTableLc, STsizeLc));
}

int inSameTable(char *name, char *type, STentry *table, int size)
{
    int     i;

    for (i = 0; i < size; i++)
        if (!strcmp(table[i].name, name) && !strcmp(table[i].type, type) && table[i].fonction)
            return (1);
    return (0);
}

int recup_offset_struct(char *name, char *cha)
{
    int         i;
    TabParCha   *champs;

    champs = recupChamp(name, recupType(name));
    for (i = 0; i < champs->sizeTab; i++)
        if (!strcmp(cha, champs[i].name))
            return (champs[i].offset);
    fprintf(stderr, "error in line %d, character %d : '%s.%s' not defined\n",
        count_line, count_char, name, cha);
    exit(2);
}

int recup_offset(char *ident)
{
    int     i;

    for (i = 0; i < STsizeLc; i++)
        if (!strcmp(symbolTableLc[i].name, ident))
            return (symbolTableLc[i].offset);
    fprintf(stderr, "error in line %d, character %d : '%s' not defined\n",
        count_line, count_char, ident);
    exit(2);
}
int typeExpression2(Node *tree, char *type)
{
    char    *tmp;
    int     ent;
    int     car;

    if (!tree)
        return (1);
    count_line = tree->lineno;
    count_char = tree->charno;
    if (tree->kind == Struct)
    {
        tmp = recupTypeStruct(tree);
        if ((ent = strcmp(tmp, "int")) && (car = strcmp(tmp, "char")))
            return (0);
        return (typeExpression2(tree->nextSibling, type));
    }
    if (tree->kind == LValue || tree->kind == IntLiteral || tree->kind == CharLiteral)
    {
        if (!type)
            return (1);
        if (tree->kind == LValue)
        {
            if (tree->firstChild->kind == Struct)
                tmp = recupTypeStruct(tree->firstChild);
            else
                tmp = recupType(tree->firstChild->u.identifier);
        }
        else if (tree->kind == IntLiteral)
            tmp = "int";
        else
            tmp = "char";
        if (strcmp(tmp, "int") && strcmp(tmp, "char"))
            return (0);
        return (typeExpression2(tree->nextSibling, type));
    }
    else if ((tree->kind <= DivStar && tree->kind >= AddSubBin) ||
             (tree->kind <= Order && tree->kind >= Eq))
    {
        if ((ent = strcmp(type, "int")) && (car = strcmp(type, "char"))) /* WARNING */
            return (0);
        return (typeExpression2(tree->firstChild, type));
    }
    else if (tree->kind == FunctCall)
    {
        tmp = recupType(tree->firstChild->u.identifier);
        if ((estStruct(tmp) && strcmp(type, tmp)) ||
            (estStruct(type) && strcmp(type, tmp))) /* WARNING */
            return (0);
        return (1);
    }
    return typeExpression2(tree->firstChild, type) && typeExpression2(tree->nextSibling, type);
}

void typeExpression(Node *tree, char *type)
{
    char    *tmp;
    int     ent;
    int     car;

    if (!tree)
        return ;
    count_line = tree->lineno;
    count_char = tree->charno;
    if (tree->kind == Struct)
    {
        tmp = recupTypeStruct(tree);
        if ((ent = strcmp(tmp, "int")) && (car = strcmp(tmp, "char")))
        {
            fprintf(stderr, "error: line %d, character %d expression wait type '%s%s' but you give a '%s%s'\n",
                count_line, FIRSTCHILD(tree)->charno, (estStruct(type)) ? "struct " : "", type,
                (estStruct(tmp)) ? "struct " : "", tmp);
            exit(2);
        }
        if (ent == 0 && !strcmp(type, "char"))
            fprintf(stderr, "warning in line %d, character %d : implicite cast from 'int' to 'char'\n",
                count_line, count_char);
        else if (!strcmp(type, "int") && ent != 0)
            fprintf(stderr, "warning in line %d, character %d : implicite cast from 'char' to 'int'\n",
                count_line, count_char);
        return (typeExpression(tree->nextSibling, type));
    }
    if (tree->kind == LValue || tree->kind == IntLiteral || tree->kind == CharLiteral)
    {
        if (!type)
            return ;
        if (tree->kind == LValue)
        {
            if (tree->firstChild->kind == Struct)
                tmp = recupTypeStruct(tree->firstChild);
            else
                tmp = recupType(tree->firstChild->u.identifier);
            if (!strcmp(tmp, "int") && !strcmp(type, "char"))
                fprintf(stderr, "warning in line %d, character %d : implicite cast from 'int' to 'char'\n",
                    count_line, count_char);
            else if (!strcmp(tmp, "char") && !strcmp(type, "int"))
                fprintf(stderr, "warning in line %d, character %d : implicite cast from 'char' to 'int'\n",
                    count_line, count_char);
        }
        else if (tree->kind == IntLiteral)
            tmp = "int";
        else
            tmp = "char";
        if (compareType(tree, type))
        {
            fprintf(stderr, "error: line %d, character %d expression wait type '%s%s' but you give a '%s%s'\n",
                count_line, count_char, (estStruct(type)) ? "struct " : "", type,
                (estStruct(tmp)) ? "struct " : "", tmp);
            exit(2);
        }
        return (typeExpression(tree->nextSibling, type));
    }
    else if ((tree->kind <= DivStar && tree->kind >= AddSubBin) ||
             (tree->kind <= Order && tree->kind >= Eq))
    {
        if ((ent = strcmp(type, "int")) && (car = strcmp(type, "char"))) /* WARNING */
        {
            fprintf(stderr, "error: line %d, character %d expression wait type '%s%s' but you give a 'int'\n",
                count_line, count_char, (estStruct(type)) ? "struct " : "", type);
            exit(2);
        }
        if (ent != 0)
            fprintf(stderr, "warning in line %d, character %d : implicite cast from 'char' to 'int'\n",
                count_line, count_char);
        return (typeExpression(tree->firstChild, type));
    }
    else if (tree->kind == FunctCall)
    {
        tmp = recupType(tree->firstChild->u.identifier);
        if ((estStruct(tmp) && strcmp(type, tmp)) ||
            (estStruct(type) && strcmp(type, tmp))) /* WARNING */
        {
            fprintf(stderr, "error: line %d, character %d expression wait type '%s%s' but you give a '%s%s'\n",
                count_line, count_char, (estStruct(type)) ? "struct " : "", type,
                (estStruct(tmp)) ? "struct " : "", tmp);
            exit(2);
        }
        return ;
    }
    typeExpression(tree->firstChild, type);
    typeExpression(tree->nextSibling, type);
}

int estValue(Kind kind)
{
    if (kind == LValue || kind == IntLiteral || kind == CharLiteral)
        return (1);
    return (0);
}

void verifySymbol(Node *tree)
{
    Node        *tmp;
    Node        *FuncName;
    char        *type;
    TabParCha   *parcha;
    int         i;

    if (!tree)
        return ;
    count_line = tree->lineno;
    count_char = tree->charno;
    if (tree->kind == LValue)
    {
        tmp = tree->firstChild;
        if (tmp->kind == Struct)
        {
            if (!(type = recupType(tmp->firstChild->u.identifier)))
            {
                fprintf(stderr, "error: the identifier '%s' is undifined in line %d, character %d\n",
                    tmp->firstChild->u.identifier, count_line, count_char);
                exit(2);
            }
            if (!(parcha = recupParaChamp(type, "struct", 0)))
            {
                fprintf(stderr, "error: the struct '%s' undifined near line %d, character %d\n",
                    type, count_line, count_char);
                exit(2);
            }
            for (i = 0; i < parcha->sizeTab; i++)
                if (!strcmp(parcha[i].name, tmp->firstChild->nextSibling->u.identifier))
                    break;
            if (i == parcha->sizeTab)
            {
                fprintf(stderr, "error: the member '%s' is undifined in line %d, character %d\n",
                    SECONDCHILD(tmp)->u.identifier, count_line, count_char);
                exit(2);
            }
        }
        else if (!inTable(tmp->u.identifier, symbolTableLc, STsizeLc))
        {
            if (!inTable(tmp->u.identifier, symbolTableGl, STsizeGl))
            {
                fprintf(stderr, "error: the identifier '%s' is undifined in line %d, character %d\n",
                    tmp->u.identifier, count_line, count_char);
                exit(2);
            }
        }
    }
    if (tree->kind == FunctCall)
    {
        FuncName = tree->firstChild;
        if (!(type = recupType(FuncName->u.identifier)) ||
            !(parcha = recupParaChamp(FuncName->u.identifier, type, 1)))
        {
            fprintf(stderr, "error: the function '%s' is undifined in line %d, character %d\n",
                FuncName->u.identifier, count_line, count_char);
            exit(2);
        }
        if ((FuncName->nextSibling->kind == Void && parcha->sizeTab != 0) ||
            (FuncName->nextSibling->kind != Void && parcha->sizeTab == 0))
        {
            fprintf(stderr, "error: bad number of argument in function '%s' in line %d, character %d\n",
                FuncName->u.identifier, count_line, count_char);
            exit(2);
        }
        for (i = 0, tmp = FuncName->nextSibling->firstChild; i < parcha->sizeTab || tmp; i++, tmp = tmp->nextSibling)
        {
            if (i >= parcha->sizeTab || !tmp)
            {
                fprintf(stderr, "error: bad number of argument in function '%s' in line %d, character %d\n",
                FuncName->u.identifier, count_line, count_char);
                exit(2);
            }
            if (estValue(tmp->kind) && compareType(tmp, parcha[i].type))
            {
                fprintf(stderr, "error in line %d, character %d: argument #%d of function %s() has type '%s%s'\n",
                    count_line, count_char, i + 1, FuncName->u.identifier,
                    (estStruct(parcha[i].type)) ? "struct " : "", parcha[i].type);
                exit(2);
            }
            else
                typeExpression(tmp->firstChild, parcha[i].type);
        }
    }
    verifySymbol(tree->firstChild);
    verifySymbol(tree->nextSibling);
}

void verifyType(Node *tree, char *typefunc)
{
    if (!tree)
        return ;
    count_line = tree->lineno;
    count_char = tree->charno;
    if (tree->kind == Return)
    {
        ret_rec = 0;
        if (tree->firstChild->kind == Void && !strcmp(typefunc, "void"))
            return ;
        return (typeExpression(tree, typefunc));
    }
    if (tree->kind == Print)
    {
        if (!typeExpression2(tree->firstChild, "int") && !typeExpression2(tree->firstChild, "char"))
        {
            fprintf(stderr, "error in line %d, character %d: argument #1 of function print() has type 'int'\n",
                count_line, count_char);
            exit(2);
        }
        return (verifyType(tree->nextSibling, typefunc));
    }
    if (tree->kind == ReadE)
    {
        if (tree->firstChild->firstChild->kind == Struct)
        {
            if (strcmp(recupTypeStruct(tree->firstChild->firstChild), "int"))
            {
                fprintf(stderr, "error in line %d, character %d: argument #1 of function reade() has type 'int'\n",
                    count_line, count_char);
                exit(2);
            }
        }
        else if (strcmp(recupType(tree->firstChild->firstChild->u.identifier), "int"))
        {
            fprintf(stderr, "error in line %d, character %d: argument #1 of function reade() has type 'int'\n",
                count_line, count_char);
            exit(2);
        }
        return (verifyType(tree->nextSibling, typefunc));
    }
    if (tree->kind == ReadC)
    {
        if (tree->firstChild->firstChild->kind == Struct)
        {
            if (strcmp(recupTypeStruct(tree->firstChild->firstChild), "char"))
            {
                fprintf(stderr, "error in line %d, character %d: argument #1 of function readc() has type 'int'\n",
                    count_line, count_char);
                exit(2);
            }
        }
        else if (strcmp(recupType(tree->firstChild->firstChild->u.identifier), "char"))
        {
            fprintf(stderr, "error in line %d, character %d: argument #1 of function readc() has type 'int'\n",
                count_line, count_char);
            exit(2);
        }
        return (verifyType(tree->nextSibling, typefunc));
    }
    if (tree->kind == Affectation)
    {
        if (tree->firstChild->firstChild->kind != Struct)
            typeExpression(tree->firstChild->nextSibling,
                recupType(tree->firstChild->firstChild->u.identifier));
        else
            typeExpression(tree->firstChild->nextSibling,
                recupTypeStruct(tree->firstChild->firstChild));
        return (verifyType(tree->nextSibling, typefunc));
    }
    if (tree->kind == Condition)
    {
        typeExpression(tree->firstChild, "int");
        return (verifyType(tree->nextSibling, typefunc));
    }
    verifyType(tree->firstChild, typefunc);
    verifyType(tree->nextSibling, typefunc);
}

void addParCha(TabParCha *tab, char *name, char *type)
{
    int i;

    for (i = 0; i < tab->sizeTab; i++)
    {
        if (!strcmp(tab[i].name, name))
        {
            fprintf(stderr, "semantic error, redefinition of variable %s near line %d\n",
            name, count_line);
            exit(2);
        }
    }
    if (++tab->sizeTab > STmax)
    {
        fprintf(stderr, "too many variables near line %d\n", count_line);
        exit(2);
    }
    strcpy(tab[tab->sizeTab - 1].name, name);
    strcpy(tab[tab->sizeTab - 1].type, type);
}

void addFonctStruct(Node *ParaCham, char *name, char *type, int fonction)
{
    Node    *VarList;

    addVarGl(name, type, fonction);
    for (VarList = ParaCham->firstChild; VarList; VarList = VarList->nextSibling)
    {
        if (VarList->kind == Void)
            return ;
        if (VarList->firstChild->kind != Struct)
            addParCha(symbolTableGl[STsizeGl - 1].parcha,
                VarList->firstChild->u.identifier, VarList->u.identifier);
        else
            addParCha(symbolTableGl[STsizeGl - 1].parcha,
                VarList->firstChild->nextSibling->u.identifier,
                VarList->firstChild->firstChild->u.identifier);
    }
}

void construct_symboltab(Node *tree)
{
    char    type[MAXTYPENAME];
    Node    *VarList;

    if (!tree)
        return ;
    count_line = tree->lineno;
    count_char = tree->charno;
	if (tree->kind == EnTeteFonct)
	{
        if (one_time == 0)
        {
            one_time = 1;
            Translation[Prog](racine);
        }
		STsizeLc = 0;
		construct_symboltabLc(tree->nextSibling);
        if (tree->firstChild->firstChild->kind != Struct)
        {
            if (opt & 2)
                printTable(STsizeLc, symbolTableLc, 
                    tree->firstChild->firstChild->u.identifier);
            addFonctStruct(tree->nextSibling,
                tree->firstChild->firstChild->u.identifier,
                tree->firstChild->u.identifier, 1);
            if ((VarList = tree->nextSibling->nextSibling->firstChild)->kind != ListInstr)
                VarList = VarList->nextSibling;
            verifySymbol(VarList);
            ret_rec = strcmp(tree->firstChild->u.identifier, "void");
            verifyType(VarList, tree->firstChild->u.identifier);
            if (ret_rec != 0)
            {
                fprintf(stderr, "error in line %d, character %d : return missing\n",
                    count_line, count_char);
                exit(2);
            }
        }
        else
        {
            if (opt & 2)
                printTable(STsizeLc, symbolTableLc,
                    tree->firstChild->firstChild->nextSibling->u.identifier);
            if (!structInTab(symbolTableGl, tree->firstChild->firstChild->firstChild->u.identifier, "struct", STsizeGl))
            {
                fprintf(stderr, "semantic error, the struct '%s' undifined near line %d, character %d\n",
                    type, count_line, count_char);
                exit(2);
            }
            addFonctStruct(tree->nextSibling,
                tree->firstChild->firstChild->nextSibling->u.identifier,
                tree->firstChild->firstChild->firstChild->u.identifier, 1);
            if ((VarList = tree->nextSibling->nextSibling->firstChild)->kind != ListInstr)
                VarList = VarList->nextSibling;
            verifySymbol(VarList);
            ret_rec = 1;
            verifyType(VarList, tree->firstChild->firstChild->firstChild->u.identifier);
            if (ret_rec != 0)
            {
                fprintf(stderr, "error in line %d, character %d : return missing\n",
                    count_line, count_char);
                exit(2);
            }
        }
        if (Translation[tree->kind])
            Translation[tree->kind](tree);
        return ;
	}
    if (tree->kind == Type)
    {
        if (tree->firstChild->kind == Struct)
        {
            if (tree->firstChild->firstChild->nextSibling)
            {
                addFonctStruct(tree->firstChild->firstChild->nextSibling->firstChild, tree->firstChild->firstChild->u.identifier, "struct", 0);
                return (construct_symboltab(tree->nextSibling));
            }
            strcpy(type, tree->firstChild->firstChild->u.identifier);
            if (!structInTab(symbolTableGl, type, "struct", STsizeGl))
            {
                fprintf(stderr, "semantic error, the struct '%s' undifined near line %d, character %d\n",
                    type, count_line, count_char);
                exit(2);
            }
        }
        else
            strcpy(type, tree->u.identifier);
        for (VarList = tree->firstChild; VarList; VarList = VarList->nextSibling)
            if (VarList->kind != Struct) 
                addVarGl(VarList->u.identifier, type, 0);
        return (construct_symboltab(tree->nextSibling));
    }
    construct_symboltab(tree->firstChild);
    construct_symboltab(tree->nextSibling);
}

void printParCha(TabParCha *parcha)
{
    int i;

    for (i = 0; i < parcha->sizeTab; i++)
        printf("\t%s%s %s\n", (estStruct(parcha[i].type)) ? "struct " : "",
            parcha[i].type, parcha[i].name);
}

void printTable(int size, STentry *tab, char *name)
{
    int i;

    printf("\n\n\n************ SYMBOL TABLE %s ***************\n\n\n", name);
    for (i = 0; i < size; i++)
    {
        printf("%s%s %s\n", (estStruct(tab[i].type)) ? "struct " : "",
            tab[i].type, tab[i].name);
        if (!strcmp(tab[i].type, "struct") || tab[i].fonction)
            printParCha(tab[i].parcha);
    }
}

void principal(Node *abstree) {
    if (opt & 1)
        printTree(abstree);
    symbolTableGl=(STentry*)malloc(STmax*sizeof(*symbolTableGl));
    symbolTableLc=(STentry*)malloc(STmax*sizeof(*symbolTableLc));
    if (!symbolTableGl || ! symbolTableLc) {
        printf("Run out of memory\n");
        exit(3);
    }
    racine = abstree;
    construct_symboltab(abstree);
    if (!inSameTable("main", "int", symbolTableGl, STsizeGl))
    {
        fprintf(stderr, "error in line %d character %d : missing main function\n",
            count_line, count_char);
        exit(2);
    }
    if (opt & 2)
        printTable(STsizeGl, symbolTableGl, "globale");
    free(symbolTableGl);
    free(symbolTableLc);
}
