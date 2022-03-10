%{
/* tpc-2020-2021.y */
/* Syntaxe du TPC pour le projet d'analyse syntaxique de 2020-2021*/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "symbol-table.h"

extern int  count_line;
extern int  count_char;
extern int  last_token;
FILE        *out = NULL;
Node        *abstree;
int         opt = 0;
extern char cnt_line[4096];
char        output_file[128];

int     yyerror(char *s)
{
    int     i;

    fprintf(stderr, "%s near line %d, character %d:\n%s\n", s, count_line,
        (count_char - last_token) + 1, cnt_line);
    for (i = 0; i < count_char - last_token; i++)
        fprintf(stderr, " ");
    fprintf(stderr, "^\n");
    return (0);
}

int     yylex(void);

%}
%union {
    int         integer;
    char        character;
    char        type[5];
    char        vide[5];
    char        identifier[64];
    char        comp[3];
    char        addsub;
    char        divstar;
    struct Node *node;
}

%expect 1
%token <identifier> IDENT
%token <type> TYPE
%token <vide> VOID
%token STRUCT
%token READE
%token READC
%token IF
%token ELSE
%token WHILE
%token RETURN
%token PRINT
%token <integer> NUM
%token <character> CHARACTER
%token <addsub> ADDSUB
%token <divstar> DIVSTAR
%token OR
%token AND
%token <comp> EQ
%token <comp> ORDER
%type <node> Prog DeclVaSt Declarateurs Champs DeclVar DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Type DeclVars Corps SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp
%%

Prog:  DeclVaSt DeclFoncts                      {
                                                  abstree = makeNode(Prog);
                                                  addChild(abstree, $1);
                                                  addSibling($1, $2);
                                                }
    ;
DeclVaSt:
       DeclVaSt Type Declarateurs ';'           {
                                                  $$ = $1; addChild($2, $3);
                                                  addChild($$, $2);
                                                }
    |  DeclVaSt STRUCT IDENT '{' Champs '}' ';' {
                                                  $$ = $1;
                                                  Node *tmp = makeNode(Type);
                                                  addChild(tmp, makeNode(Struct));
                                                  addChild(FIRSTCHILD(tmp), makeNode(Identifier));
                                                  strcpy(FIRSTCHILD(tmp)->firstChild->u.identifier, $3);
                                                  addChild(FIRSTCHILD(tmp), $5);
                                                  addChild($$, tmp);
                                                }
    |                                           {
                                                  $$ = makeNode(VarDeclListGl);
                                                }
    ;
Declarateurs:
       Declarateurs ',' IDENT                   {
                                                  $$ = $1;
                                                  Node *tmp = makeNode(Identifier);
                                                  strcpy(tmp->u.identifier, $3);
                                                  addSibling($$, tmp);
                                                }
    |  IDENT                                    {
                                                  $$ = makeNode(Identifier);
                                                  strcpy($$->u.identifier, $1);
                                                }
    ;
Champs:
       DeclVar TYPE Declarateurs ';'            {
                                                  $$ = makeNode(Champs);
                                                  Node *tmp = makeNode(Type);
                                                  strcpy(tmp->u.identifier, $2);
                                                  addChild(tmp, $3);
                                                  addChild($1, tmp);
                                                  addChild($$, $1);
                                                }
    ;
DeclVar:
        DeclVar TYPE Declarateurs ';'           {
                                                  $$ = $1;
                                                  Node *tmp = makeNode(Type);
                                                  strcpy(tmp->u.identifier, $2);
                                                  addChild(tmp, $3);
                                                  addChild($$, tmp);
                                                }
    |                                           {
                                                  $$ = makeNode(DeclVar);
                                                }
    ;
DeclFoncts:
       DeclFoncts DeclFonct                     { $$ = $1; addChild($$, $2); }
    |  DeclFonct                                {
                                                  $$ = makeNode(DeclFoncts);
                                                  addChild($$, $1);
                                                }
    ;
DeclFonct:
       EnTeteFonct Corps                        {
                                                  $$ = makeNode(DeclFonct);
                                                  addChild($$, $1);
                                                  addChild($$, $2);
                                                }
    ;
EnTeteFonct:
       Type IDENT '(' Parametres ')'            {
                                                  $$ = makeNode(EnTeteFonct);
                                                  Node *tmp = makeNode(Identifier);
                                                  strcpy(tmp->u.identifier, $2);
                                                  addChild($1, tmp);
                                                  addChild($$, $1);
                                                  addSibling($$, $4);
                                                }
    |  VOID IDENT '(' Parametres ')'            {
                                                  $$ = makeNode(EnTeteFonct);
                                                  Node *tmp = makeNode(Type);
                                                  strcpy(tmp->u.identifier, $1);
                                                  addChild($$, tmp);
                                                  tmp = makeNode(Identifier);
                                                  tmp->lineno = count_line;
                                                  strcpy(tmp->u.identifier, $2);
                                                  addChild(FIRSTCHILD($$), tmp);
                                                  addSibling($$, $4);
                                                }
    ;
Parametres:
       VOID                                     {
                                                  $$ = makeNode(Parametres);
                                                  addChild($$, makeNode(Void));
                                                }
    |  ListTypVar                               { $$ = $1; }
    ;
ListTypVar:
       ListTypVar ',' Type IDENT                {
                                                  $$ = $1;
                                                  Node *tmp = makeNode(Identifier);
                                                  strcpy(tmp->u.identifier, $4);
                                                  addChild($3, tmp);
                                                  addChild($$, $3);
                                                }
    |  Type IDENT                               {
                                                  $$ = makeNode(Parametres);
                                                  Node *tmp = makeNode(Identifier);
                                                  strcpy(tmp->u.identifier, $2);
                                                  addChild($1, tmp);
                                                  addChild($$, $1);
                                                }
    ;
Type:  TYPE                                     {
                                                  $$ = makeNode(Type);
                                                  strcpy($$->u.identifier, $1);
                                                }
    |  STRUCT IDENT                             {
                                                  $$ = makeNode(Type);
                                                  Node *tmp = makeNode(Struct);
                                                  addChild(tmp, makeNode(Identifier));
                                                  strcpy(FIRSTCHILD(tmp)->u.identifier, $2);
                                                  addChild($$, tmp);
                                                }
    ;
DeclVars:
       DeclVars Type Declarateurs ';'           {
                                                  $$ = $1; addChild($2, $3);
                                                  addChild($$, $2);
                                                }
    |                                           {
                                                  $$ = makeNode(VarDeclListLc);
                                                }
    ;
Corps: '{' DeclVars SuiteInstr '}'              {
                                                  $$ = makeNode(Corps);
                                                  if ($2->firstChild)
                                                      addChild($$, $2);
                                                  addChild($$, $3);
                                                }
    ;
SuiteInstr:
       SuiteInstr Instr                         { $$ = $1; addChild($$, $2); }
    |                                           {
                                                  $$ = makeNode(ListInstr);
                                                }
    ;
Instr:
       LValue '=' Exp ';'                       {
                                                  $$ = makeNode(Affectation);
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  READE '(' LValue ')' ';'                 {
                                                  $$ = makeNode(ReadE);
                                                  addChild($$, $3);
                                                }
    |  READC '(' LValue ')' ';'                 {
                                                  $$ = makeNode(ReadC);
                                                  addChild($$, $3);
                                                }
    |  PRINT '(' Exp ')' ';'                    {
                                                  $$ = makeNode(Print);
                                                  addChild($$, $3);
                                                }
    |  IF '(' Exp ')' Instr                     {
                                                  $$ = makeNode(If);
                                                  addChild($$, makeNode(Condition));
                                                  addChild(FIRSTCHILD($$), $3);
                                                  addChild($$, $5);
                                                }
    |  IF '(' Exp ')' Instr ELSE Instr          {
                                                  $$ = makeNode(IfwElse);
                                                  addChild($$, makeNode(Condition));
                                                  addChild(FIRSTCHILD($$), $3);
                                                  addChild($$, $5);
                                                  addSibling($$, makeNode(ElsewIf));
                                                  addChild($$->nextSibling, $7);
                                                }
    |  WHILE '(' Exp ')' Instr                  {
                                                  $$ = makeNode(While);
                                                  addChild($$, makeNode(Condition));
                                                  addChild(FIRSTCHILD($$), $3);
                                                  addChild($$, $5);
                                                }
    |  IDENT '(' Arguments  ')' ';'             {
                                                  $$ = makeNode(FunctCall);
                                                  addChild($$, makeNode(Identifier));
                                                  strcpy((FIRSTCHILD($$))->u.identifier, $1);
                                                  addChild($$, $3);
                                                }
    |  RETURN Exp ';'                           {
                                                  $$ = makeNode(Return);
                                                  addChild($$, $2);
                                                }
    |  RETURN ';'                               {
                                                  $$ = makeNode(Return);
                                                  addChild($$, makeNode(Void));
                                                }
    |  '{' SuiteInstr '}'                       {
                                                  $$ = makeNode(Bloc);
                                                  addChild($$, $2);
                                                }
    |  ';'                                      { $$ = NULL; }
    ;
Exp :  Exp OR TB                                {
                                                  $$ = makeNode(Or);
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  TB                                       { $$ = $1; }
    ;
TB  :  TB AND FB                                {
                                                  $$ = makeNode(And);
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  FB                                       { $$ = $1; }
    ;
FB  :  FB EQ M                                  {
                                                  $$ = makeNode(Eq);
                                                  strcpy($$->u.identifier, $2);
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  M                                        { $$ = $1; }
    ;
M   :  M ORDER E                                {
                                                  $$ = makeNode(Order);
                                                  strcpy($$->u.identifier, $2);
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  E                                        { $$ = $1; }
    ;
E   :  E ADDSUB T                               {
                                                  $$ = makeNode(AddSubBin);
                                                  $$->u.character = $2;
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  T                                        { $$ = $1; }
    ;    
T   :  T DIVSTAR F                              {
                                                  $$ = makeNode(DivStar);
                                                  $$->u.character = $2;
                                                  addChild($$, $1);
                                                  addChild($$, $3);
                                                }
    |  F                                        { $$ = $1; }
    ;
F   :  ADDSUB F                                 {
                                                  $$ = makeNode(AddSubUn);
                                                  $$->u.character = $1;
                                                  addChild($$, $2);
                                                }
    |  '!' F                                    {
                                                  $$ = makeNode(Not);
                                                  addChild($$, $2);
                                                }
    |  '(' Exp ')'                              { $$ = $2; }
    |  NUM                                      {
                                                  $$ = makeNode(IntLiteral);
                                                  $$->u.integer = $1;
                                                }
    |  CHARACTER                                {
                                                  $$ = makeNode(CharLiteral);
                                                  $$->u.character = $1;
                                                }
    |  LValue                                   { $$ = $1; }
    |  IDENT '(' Arguments  ')'                 {
                                                  $$ = makeNode(FunctCall);
                                                  addChild($$, makeNode(Identifier));
                                                  strcpy((FIRSTCHILD($$))->u.identifier, $1);
                                                  addChild($$, $3);
                                                }
    ;
LValue:
       IDENT                                    {
                                                  $$ = makeNode(LValue);
                                                  addChild($$, makeNode(Identifier));
                                                  strcpy((FIRSTCHILD($$))->u.identifier, $1);
                                                }
    |  IDENT '.' IDENT                          {
                                                  $$ = makeNode(LValue);
                                                  Node *tmp = makeNode(Struct);
                                                  addChild(tmp, makeNode(Identifier));
                                                  addChild(tmp, makeNode(Identifier));
                                                  strcpy(FIRSTCHILD(tmp)->u.identifier, $1);
                                                  strcpy(SECONDCHILD(tmp)->u.identifier, $3);
                                                  addChild($$, tmp);
                                                }
    ;
Arguments:
       ListExp                                  { $$ = $1; }
    |                                           {
                                                  $$ = makeNode(Void);
                                                }
    ;
ListExp:
       ListExp ',' Exp                          { $$ = $1; addChild($$, $3); }
    |  Exp                                      {
                                                  $$ = makeNode(Arguments);
                                                  addChild($$, $1);
                                                }
    ;
%%

void    afficheAide(void)
{
    printf("Usage: ./tpcc [option] file | ./tpcc [option]\n");
    printf("Options:\n");
    printf(" -h, --help\t\t\tDisplay this information.\n");
    printf(" -o <file>, --output <file>\tPlace the output in <file>.\n");
    printf(" -t, --tree\t\t\tPrint the current abstract tree.\n");
    printf(" -s, --symtabs\t\t\tPrint all current symbol tabs.\n");
}

int     main(int ac, char **av)
{
    int                 retparse;
    int                 ret;
    int                 lgind;
    int                 fd;
    const struct option long_option[] = 
    {
        {"output", 1, 0, 'o'},
        {"tree", 0, 0, 't'},
        {"symtabs", 0, 0, 's'},
        {"help", 0, 0, 'h'}
    };

    strcpy(output_file, "_anonymous.asm");
    while ((ret = getopt_long(ac, av, "o:tsh", long_option, &lgind)) != -1)
    {
        switch (ret)
        {
            case ('o') :
                strcpy(output_file, optarg);
                opt |= 4;
                break;
            case ('t') :
                opt |= 1;
                break;
            case ('s') :
                opt |= 2;
                break;
            case ('h') :
                afficheAide();
                return (0);
            case ('?') :
                return (6);
            default :
                break;
        }
    }
    if (optind < ac)
    {
        if (!(opt & 4))
        {
            strncpy(output_file, av[optind], strlen(av[optind]) - 3);
            output_file[strlen(av[optind]) - 3] = 0;
            strcat(output_file, "asm");
        }
        if ((fd = open(av[optind], O_RDONLY)) == -1)
        {
            perror("open input file");
            return (4);
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("move file descriptor");
            return (5);
        }
    }
    if (!(out = (fopen(output_file, "w"))))
    {
        perror("fopen");
        exit(3);
    }
    if ((retparse = yyparse()))
        return (1);
    close(fd);
    principal(abstree);
    deleteTree(abstree);
    fclose(out);
    return (0);
}
