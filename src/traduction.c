#include <stdio.h>
#include <string.h>
#include "traduction.h"
#include "symbol-table.h"

extern FILE *out;
extern int  nbstruct;
extern char *StringFromKind[];
int         nb_if = 0;
int         gl = 0;
int         loortworwh = 0;
int         align;
int         isarg = 0;

translation Translation[] =
{
    prog,
    declVarLc,
    declVarGl,
    champs,
    condition,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    function,
    NULL,
    instr,
    NULL,
	affectation,
    corps,
    corps,
    NULL,
    NULL,
	NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    structure,
    lValue,
    arguments,
    NULL,
    intLiteral,
    charLiteral,
    NULL,
    addSub,
    addSubUn,
    divStar,
    print,
    reade,
    readc,
    whileloop,
    lonelyif,
	ifwelse,
	NULL,
    retour,
	functCall,
    equal,
    ou,
	et,
	non,
    order,
    NULL
};

void    prog(Node *pr)
{
    Translation[pr->firstChild->kind](pr->firstChild);
    startFile(out);
}

void    condition(Node *cond)
{
    Translation[cond->firstChild->kind](cond->firstChild);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tje %s%d\n",
        (loortworwh == 1) ? "elwif" : (loortworwh == 2) ? "endwhile" : "endloif",
        nb_if);
}

int     alignement(int al)
{
    if (al % 2 == 0)
        return (al * 8);
    return ((al + 1) * 8);
}

void    declVarLc(Node *decv)
{
    Node    *tmp;
    Node    *tmp2;

    gl = 2;
    align = 0;
    for (tmp = decv->firstChild; tmp; tmp = tmp->nextSibling)
    {
        if (tmp->firstChild->kind == Struct)
            Translation[tmp->firstChild->kind](tmp->firstChild);
        else
        {
            for (tmp2 = tmp->firstChild; tmp2; tmp2 = tmp2->nextSibling)
                align++;
        }
    }
    if (align)
        fprintf(out, "\tsub rsp, %d\n", alignement(align));
    gl = 0;
}

void    declVarGl(Node *decv)
{
    Node    *tmp;
    Node    *tmp2;

    fprintf(out, "extern printf\n");
    fprintf(out, "extern scanf\n");
    fprintf(out, "section .bss\n");
    gl = 1;
    for (tmp = decv->firstChild; tmp; tmp = tmp->nextSibling)
    {
        if (tmp->firstChild->kind == Struct)
            Translation[tmp->firstChild->kind](tmp->firstChild);
        else
        {
            for (tmp2 = tmp->firstChild; tmp2; tmp2 = tmp2->nextSibling)
                fprintf(out, "\t%s: resq 1\n", tmp2->u.identifier);
        }
    }
    fprintf(out, "\n");
    gl = 0;
}

void    divStar(Node *term)
{
    Translation[term->firstChild->kind](term->firstChild);
    Translation[term->firstChild->nextSibling->kind](term->firstChild->nextSibling);
    (term->u.character == '*') ? mul(out) : (term->u.character == '/') ? divi(out) : modu(out);
}

void    addSub(Node *exp)
{
    Translation[exp->firstChild->kind](exp->firstChild);
    Translation[exp->firstChild->nextSibling->kind](exp->firstChild->nextSibling);
    (exp->u.character == '+') ? add(out) : sub(out);
}

void    addSubUn(Node *exp)
{
    Translation[exp->firstChild->kind](exp->firstChild);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tneg rax\n");
    fprintf(out, "\tpush rax\n");
}

void    function(Node *fun)
{
    Node *tmp;

    if (fun->firstChild->firstChild->kind != Struct)
        fprintf(out, "%s:\n", fun->firstChild->firstChild->u.identifier);
    else
        fprintf(out, "%s:\n",
            fun->firstChild->firstChild->nextSibling->u.identifier);
    startFunction(out);
    for (tmp = fun->nextSibling; tmp; tmp = tmp->nextSibling)
        if (Translation[tmp->kind])
            Translation[tmp->kind](tmp);
    endFunction(out);
    fprintf(out, "\tret\n");
    fprintf(out, "\n");
}

void    retour(Node *ret)
{
    TabParCha   *cha;
    int         i;

    if (ret->firstChild->kind == Void)
    {
        endFunction(out);
        fprintf(out, "\tret\n");
        return ;
    }
    Translation[ret->firstChild->kind](ret->firstChild);
    if (ret->firstChild->kind == LValue &&
        ret->firstChild->firstChild->kind != Struct &&
        estStruct(recupType(ret->firstChild->firstChild->u.identifier)))
    {
        if (inTableLc(ret->firstChild->firstChild->u.identifier))
            cha = recupChamp(ret->firstChild->firstChild->u.identifier,
                recupType(ret->firstChild->firstChild->u.identifier));
        else
            cha = recupParaChamp(recupType(ret->firstChild->firstChild->u.identifier),
                "struct", 0);
        for (i = cha->sizeTab - 1; i >= 0; i--)
        {
            fprintf(out, "\tpop rax\n");
            fprintf(out, "\tmov QWORD [r15 + %d], rax\n", i * 8);
        }
    }
    else
        fprintf(out, "\tpop rax\n");
    endFunction(out);
    fprintf(out, "\tret\n");
}

void    affectation(Node *aff)
{
    int         offset;
    int         i;
    TabParCha   *cha;

    Translation[aff->firstChild->nextSibling->kind](aff->firstChild->nextSibling);
    if (aff->firstChild->firstChild->kind == Struct)
    {
        fprintf(out, "\tpop rax\n");
        if (inTableLc(aff->firstChild->firstChild->firstChild->u.identifier))
        {
            offset = recup_offset_struct(
                aff->firstChild->firstChild->firstChild->u.identifier,
                aff->firstChild->firstChild->firstChild->nextSibling->u.identifier);
            fprintf(out, "\tmov QWORD [rbp %s %d], rax\n",
                (offset < 0) ? "" : "+", offset);
        }
        else
            fprintf(out, "\tmov QWORD [%s.%s], rax\n",
                aff->firstChild->firstChild->firstChild->u.identifier,
                aff->firstChild->firstChild->firstChild->nextSibling->u.identifier);
        return ;
    }
    if (estStruct(recupType(aff->firstChild->firstChild->u.identifier)))
    {
        if (inTableLc(aff->firstChild->firstChild->u.identifier))
        {
            cha = recupChamp(aff->firstChild->firstChild->u.identifier,
                recupType(aff->firstChild->firstChild->u.identifier));
            for (i = cha->sizeTab - 1; i >= 0; i--)
            {
                fprintf(out, "\tpop rax\n");
                fprintf(out, "\tmov QWORD [rbp %s %d], rax\n",
                    (cha[i].offset) ? "" : "+", cha[i].offset);
            }
        }
        else
        {
            cha = recupParaChamp(
                recupType(aff->firstChild->firstChild->u.identifier),
                "struct", 0);
            for (i = cha->sizeTab - 1; i >= 0; i--)
            {
                fprintf(out, "\tpop rax\n");
                fprintf(out, "\tmov QWORD [%s.%s], rax\n",
                    aff->firstChild->firstChild->u.identifier, cha[i].name);
            }
        }
        return ;
    }
    fprintf(out, "\tpop rax\n");
    if (inTableLc(aff->firstChild->firstChild->u.identifier))
    {
        offset = recup_offset(aff->firstChild->firstChild->u.identifier);
        fprintf(out, "\tmov QWORD [rbp %s %d], rax\n", (offset < 0) ? "" : "+",
            offset);
        return ;
    }
    fprintf(out, "\tmov QWORD [%s], rax\n",
        aff->firstChild->firstChild->u.identifier);
}

void    lValue(Node *lvalue)
{
    TabParCha   *cha;
    int         i;

    if (lvalue->firstChild->kind == Struct)
        return (Translation[lvalue->firstChild->kind](lvalue->firstChild));
    if (estStruct(recupType(lvalue->firstChild->u.identifier)))
    {
        if (inTableLc(lvalue->firstChild->u.identifier))
        {
            cha = recupChamp(lvalue->firstChild->u.identifier,
                recupType(lvalue->firstChild->u.identifier));
            if (isarg)
            {
                for (i = cha->sizeTab - 1; i >= 0; i--)
                    pushLValue(cha[i].offset, out);
            }
            else
            {
                for (i = 0; i < cha->sizeTab; i++)
                    pushLValue(cha[i].offset, out);
            }
        }
        else
        {
            cha = recupParaChamp(recupType(lvalue->firstChild->u.identifier),
                "struct", 0);
            if (isarg)
            {
                for (i = cha->sizeTab - 1; i >= 0; i--)
                    fprintf(out, "\tpush QWORD [%s.%s]\n",
                        lvalue->firstChild->u.identifier, cha[i].name);
            }
            else
            {
                for (i = 0; i < cha->sizeTab; i++)
                    fprintf(out, "\tpush QWORD [%s.%s]\n",
                        lvalue->firstChild->u.identifier, cha[i].name);
            }
        }
        return ;
    }
    if (inTableLc(lvalue->firstChild->u.identifier))
        pushLValue(recup_offset(lvalue->firstChild->u.identifier), out);
    else
        fprintf(out, "\tpush QWORD [%s]\n", lvalue->firstChild->u.identifier);
}

void    instr(Node *inst)
{
    Node    *tmp;

    for (tmp = inst->firstChild; tmp; tmp = tmp->nextSibling)
    {
        if (Translation[tmp->kind])
            Translation[tmp->kind](tmp);
    }
}

void    champs(Node *champs)
{
    return ;
}

void    corps(Node *corps)
{
    Node    *tmp;

    for (tmp = corps->firstChild; tmp; tmp = tmp->nextSibling)
        if (Translation[tmp->kind])
            Translation[tmp->kind](tmp);
}

void    whileloop(Node *wh)
{
    int     hight;

    hight = nb_if;
    loortworwh = 2;
    fprintf(out, "while%d:\n", hight);
    Translation[wh->firstChild->kind](wh->firstChild);
    nb_if++;
    Translation[wh->firstChild->nextSibling->kind](wh->firstChild->nextSibling);
    fprintf(out, "\tjmp while%d\n", hight);
    fprintf(out, "endwhile%d:\n", hight);
}

void    ifwelse(Node *iwe)
{
    int     hight;

    hight = nb_if;
    loortworwh = 1;
    Translation[iwe->firstChild->kind](iwe->firstChild);
    fprintf(out, "ifwel%d:\n", hight);
    nb_if++;
    Translation[iwe->firstChild->nextSibling->kind](iwe->firstChild->nextSibling);
    fprintf(out, "\tjmp endif%d\n", hight);
    fprintf(out, "elwif%d:\n", hight);
    Translation[iwe->nextSibling->firstChild->kind](iwe->nextSibling->firstChild);
    fprintf(out, "endif%d:\n", hight);
}

void    lonelyif(Node *i)
{
    int     hight;

    hight = nb_if;
    loortworwh = 0;
    Translation[i->firstChild->kind](i->firstChild);
    fprintf(out, "loif%d:\n", hight);
    nb_if++;
    Translation[i->firstChild->nextSibling->kind](i->firstChild->nextSibling);
    fprintf(out, "endloif%d:\n", hight);
}

void    charLiteral(Node *c)
{
    pushValue(c->u.character, out);
}

void    intLiteral(Node *i)
{
    pushValue(i->u.integer, out);
}

void    print(Node *pr)
{
    char    *type;

    if (Translation[pr->firstChild->kind])
        Translation[pr->firstChild->kind](pr->firstChild);
    fprintf(out, "\tpop rsi\n");
    type = recup_type(pr->firstChild);
    if (!strcmp(type, "int"))
        fprintf(out, "\tmov rdi, fmt_d_pr\n");
    else
        fprintf(out, "\tmov rdi, fmt_c_pr\n");
    fprintf(out, "\tcall printf\n");
}

void    aux_args(Node *args)
{
    if (!args)
        return ;
    aux_args(args->nextSibling);
    Translation[args->kind](args);
}

void    arguments(Node *arg)
{
    isarg = 1;
    aux_args(arg->firstChild);
    isarg = 0;
}

void    reade(Node *re)
{
    int     offset;

    fprintf(out, "\tsub rsp, 16\n");
    fprintf(out, "\tlea rsi, [rsp]\n");
    fprintf(out, "\tmov rdi, fmt_d_sc\n");
    fprintf(out, "\tcall scanf\n");
    fprintf(out, "\tmov rax, QWORD [rsp]\n");
    if (re->firstChild->firstChild->kind == Struct)
    {
        if (inTableLc(re->firstChild->firstChild->firstChild->u.identifier))
        {
            offset = recup_offset_struct(
                re->firstChild->firstChild->firstChild->u.identifier,
                re->firstChild->firstChild->firstChild->nextSibling->u.identifier);
            fprintf(out, "\tmov QWORD [rbp %s %d], rax\n",
                (offset < 0) ? "" : "+", offset);
        }
        else
            fprintf(out, "\tmov QWORD [%s.%s], rax\n",
                re->firstChild->firstChild->firstChild->u.identifier,
                re->firstChild->firstChild->firstChild->nextSibling->u.identifier);
        return ;
    }
    if (inTableLc(re->firstChild->firstChild->u.identifier))
    {
        offset = recup_offset(re->firstChild->firstChild->u.identifier);
        fprintf(out, "\tmov QWORD [rbp %s %d], rax\n", (offset < 0) ? "" : "+",
            offset);
        return ;
    }
    fprintf(out, "\tmov QWORD [%s], rax\n", re->firstChild->firstChild->u.identifier);
}

void    readc(Node *rc)
{
    int     offset;

    fprintf(out, "\tsub rsp, 16\n");
    fprintf(out, "\tlea rsi, [rsp]\n");
    fprintf(out, "\tmov rdi, fmt_c_sc\n");
    fprintf(out, "\tcall scanf\n");
    fprintf(out, "\tmov rax, QWORD [char]\n");
    if (rc->firstChild->firstChild->kind == Struct)
    {
        if (inTableLc(rc->firstChild->firstChild->firstChild->u.identifier))
        {
            offset = recup_offset_struct(
                rc->firstChild->firstChild->firstChild->u.identifier,
                rc->firstChild->firstChild->firstChild->nextSibling->u.identifier);
            fprintf(out, "\tmov QWORD [rbp %s %d], rax\n",
                (offset < 0) ? "" : "+", offset);
        }
        else
            fprintf(out, "\tmov QWORD [%s.%s], rax\n",
                rc->firstChild->firstChild->firstChild->u.identifier,
                rc->firstChild->firstChild->firstChild->nextSibling->u.identifier);
        return ;
    }
    if (inTableLc(rc->firstChild->firstChild->u.identifier))
    {
        offset = recup_offset(rc->firstChild->firstChild->u.identifier);
        fprintf(out, "\tmov QWORD [rbp %s %d], rax\n", (offset < 0) ? "" : "+",
            offset);
        return ;
    }
    fprintf(out, "\tmov QWORD [%s], rax\n", rc->firstChild->u.identifier);
}

int     recupArg(char *fname)
{
    TabParCha   *par;
    TabParCha   *cha;
    int         ret;
    int         i;

    par = recupParaChamp(fname, recupType(fname), 1);
    for (i = 0, ret = 0; i < par->sizeTab; i++)
    {
        if (estStruct(par[i].type))
        {
            cha = recupParaChamp(par[i].type, "struct", 0);
            ret += cha->sizeTab;
        }
        else
            ret++;
    }
    return (ret);
}

void    functCall(Node *fc)
{
    Node        *tmp;
    TabParCha   *cha;
    int         nb_arg;
    int         i;

    if (estStruct(recupType(fc->firstChild->u.identifier)))
        fprintf(out, "\tlea r15, [struct%d]\n",
            recup_postruct(recupType(fc->firstChild->u.identifier)));
    nb_arg = recupArg(fc->firstChild->u.identifier);
    if (nb_arg % 2 == 1)
        fprintf(out, "\tpush QWORD 0\n");
    if (Translation[fc->firstChild->nextSibling->kind])
        Translation[fc->firstChild->nextSibling->kind](fc->firstChild->nextSibling);
    fprintf(out, "\tcall %s\n", fc->firstChild->u.identifier);
    for (tmp = fc->firstChild->nextSibling->firstChild; tmp; tmp = tmp->nextSibling)
    {
        if (tmp->kind == LValue &&
            tmp->firstChild->kind != Struct &&
            estStruct(recupType(tmp->firstChild->u.identifier)))
        {
            cha = recupParaChamp(recupType(tmp->firstChild->u.identifier),
                "struct", 0);
            for (i = 0; i < cha->sizeTab; i++)
                fprintf(out, "\tpop rbx\n");
        }
        else
            fprintf(out, "\tpop rbx\n");
    }
    if (nb_arg % 2 == 1)
        fprintf(out, "\tpop rbx\n");
    if (!strcmp(recupType(fc->firstChild->u.identifier), "void"))
        return ;
    else if (!estStruct(recupType(fc->firstChild->u.identifier)))
        fprintf(out, "\tpush rax\n");
    else
    {
        cha = recupParaChamp(recupType(fc->firstChild->u.identifier),
            "struct", 0);
        for (i = 0; i < cha->sizeTab; i++)
            fprintf(out, "\tpush QWORD [r15 + %d]\n", i * 8);
    }
}

void    et(Node *e)
{
    Translation[e->firstChild->kind](e->firstChild);
    Translation[e->firstChild->nextSibling->kind](e->firstChild->nextSibling);
    fprintf(out, "\tmov rax, 0\n");
    fprintf(out, "\tpop r9\n");
    fprintf(out, "\tpop r10\n");
    fprintf(out, "\tcmp r9, 0\n");
    fprintf(out, "\tsetne al\n");
    fprintf(out, "\tcmp r10, 0\n");
    fprintf(out, "\tsetne dl\n");
    fprintf(out, "\tand al, dl\n");
    fprintf(out, "\tpush rax\n");
}

void    ou(Node *o)
{
    Translation[o->firstChild->kind](o->firstChild);
    Translation[o->firstChild->nextSibling->kind](o->firstChild->nextSibling);
    fprintf(out, "\tmov rax, 0\n");
    fprintf(out, "\tpop r9\n");
    fprintf(out, "\tpop r10\n");
    fprintf(out, "\tcmp r9, 0\n");
    fprintf(out, "\tsetne al\n");
    fprintf(out, "\tcmp r10, 0\n");
    fprintf(out, "\tsetne dl\n");
    fprintf(out, "\tor al, dl\n");
    fprintf(out, "\tpush rax\n");
}

void    non(Node *n)
{
    Translation[n->firstChild->kind](n->firstChild);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tmov rbx, 0\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tsete bl\n");
    fprintf(out, "\tpush rbx\n");
}

void    order(Node *ord)
{
    Translation[ord->firstChild->kind](ord->firstChild);
    Translation[ord->firstChild->nextSibling->kind](ord->firstChild->nextSibling);
    fprintf(out, "\tmov rdx, 0\n");
    cmp(out);
    if (ord->u.identifier[0] == '<')
        fprintf(out, "\tsetl");
    else
        fprintf(out, "\tsetg");
    if (ord->u.identifier[1] == '=')
        fprintf(out, "e");
    fprintf(out, " dl\n");
    fprintf(out, "\tpush rdx\n");
}

void    equal(Node *eq)
{
    Translation[eq->firstChild->kind](eq->firstChild);
    Translation[eq->firstChild->nextSibling->kind](eq->firstChild->nextSibling);
    fprintf(out, "\tmov rdx, 0\n");
    cmp(out);
    if (!strcmp(eq->u.identifier, "=="))
        fprintf(out, "\tsete dl\n");
    else
        fprintf(out, "\tsetne dl\n");
    fprintf(out, "\tpush rdx\n");
}

void    structure(Node *struc)
{
    TabParCha   *cha;
    Node        *tmp;
    int         i;

    if (gl == 1)
    {
        if (!struc->firstChild->nextSibling)
        {
            cha = recupParaChamp(struc->firstChild->u.identifier,
                "struct", 0);
            for (tmp = struc->nextSibling; tmp; tmp = tmp->nextSibling)
                for (i = 0; i < cha[0].sizeTab; i++)
                    fprintf(out, "\t%s.%s: resq 1\n", tmp->u.identifier,
                        cha[i].name);
        }
        else
        {
            cha = recupParaChamp(struc->firstChild->u.identifier,
                "struct", 0);
            fprintf(out, "\tstruct%d: resq %d\n",
                recup_postruct(struc->firstChild->u.identifier), alignement(cha->sizeTab));
        }
    }
    else if (gl == 2)
    {
        cha = recupParaChamp(struc->firstChild->u.identifier,
            "struct", 0);
        for (tmp = struc->nextSibling; tmp; tmp = tmp->nextSibling)
            align += cha->sizeTab;
    }
    else
    {
        if (inTableLc(struc->firstChild->u.identifier))
        {
            cha = recupChamp(struc->firstChild->u.identifier,
                recupType(struc->firstChild->u.identifier));
            for (i = 0; i < cha->sizeTab; i++)
                if (!strcmp(struc->firstChild->nextSibling->u.identifier, cha[i].name))
                {
                    pushLValue(cha[i].offset, out);
                    break;
                }
        }
        else
        {
            fprintf(out, "\tpush QWORD [%s.%s]\n", struc->firstChild->u.identifier,
                struc->firstChild->nextSibling->u.identifier);
        }
    }
}

void    cmp(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, rbx\n");
}

void    add(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
	fprintf(out, "\tpop rax\n");
	fprintf(out, "\tadd rax, rbx\n");
	fprintf(out, "\tpush rax\n");
}

void    sub(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
	fprintf(out, "\tpop rax\n");
	fprintf(out, "\tsub rax, rbx\n");
	fprintf(out, "\tpush rax\n");
}

void    mul(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
	fprintf(out, "\tpop rax\n");
	fprintf(out, "\timul rax, rbx\n");
	fprintf(out, "\tpush rax\n");
}

void    modu(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
	fprintf(out, "\tpop rax\n");
	fprintf(out, "\tmov rdx, 0\n");
	fprintf(out, "\tidiv rbx\n");
	fprintf(out, "\tpush rdx\n");
}

void    divi(FILE *out)
{
    fprintf(out, "\tpop rbx\n");
	fprintf(out, "\tpop rax\n");
	fprintf(out, "\tpush rdx\n");
	fprintf(out, "\tmov rdx, 0\n");
	fprintf(out, "\tidiv rbx\n");
	fprintf(out, "\tpop rdx\n");
	fprintf(out, "\tpush rax\n");
}

void	pushValue(int value, FILE *out)
{
	fprintf(out, "\tpush QWORD %d\n", value);
}

void	pushLValue(int offset, FILE *out)
{
	fprintf(out, "\tmov rax, QWORD [rbp %s %d]\n", (offset < 0) ? "" : "+", offset);
	fprintf(out, "\tpush rax\n");
}

void	startFile(FILE *out)
{
	fprintf(out, "section .data\n");
    fprintf(out, "\tfmt_c_pr: db \"%%c\", 0\n");
    fprintf(out, "\tfmt_d_pr: db \"%%d\", 0\n");
    fprintf(out, "\tfmt_c_sc: db \" %%c\", 0\n");
    fprintf(out, "\tfmt_d_sc: db \"%%d\", 0\n\n");
	fprintf(out, "section .text\n");
	fprintf(out, "\tglobal main\n\n");
}

void    startFunction(FILE *out)
{
    fprintf(out, "\tpush rbp\n");
    fprintf(out, "\tmov rbp, rsp\n");
}

void    endFunction(FILE *out)
{
    fprintf(out, "\tmov rsp, rbp\n");
    fprintf(out, "\tpop rbp\n");
}
