#ifndef __TRAD_H__
#define __TRAD_H__
#include "abstract-tree.h"
#include <stdio.h>

typedef void (*translation)(Node *);

void    prog(Node *pr);
void    condition(Node *cond);
void    declVarLc(Node *decv);
void    declVarGl(Node *decv);
void    divStar(Node *term);
void    addSub(Node *exp);
void    addSubUn(Node *exp);
void    function(Node *fun);
void    retour(Node *ret);
void    affectation(Node *aff);
void    lValue(Node *lvalue);
void    instr(Node *inst);
void    champs(Node *champs);
void    corps(Node *corps);
void    whileloop(Node *wh);
void    ifwelse(Node *iwe);
void    lonelyif(Node *i);
void    charLiteral(Node *c);
void    intLiteral(Node *i);
void    print(Node *pr);
void    reade(Node *re);
void    readc(Node *rc);
void    functCall(Node *fc);
void    et(Node *e);
void    ou(Node *o);
void    non(Node *n);
void    arguments(Node *arg);
void    order(Node *ord);
void    equal(Node *eq);
void    structure(Node *struc);
void    add(FILE *out);
void    sub(FILE *out);
void    divi(FILE *out);
void    modu(FILE *out);
void    mul(FILE *out);
void	pushValue(int value, FILE *out);
void	pushLValue(int offset, FILE *out);
void    startFile(FILE *out);
void    endFile(FILE *out);
void    startFunction(FILE *out);
void    endFunction(FILE *out);
void    endFile(FILE *out);
void    cmp(FILE *out);
#endif
