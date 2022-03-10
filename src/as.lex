%{
#include "as.tab.h"
#include <string.h>
int     count_line = 1;
int     count_char = 0;
int     last_token;
int     line_flag = 0;
char    cnt_line[4096] = "";
%}

%option noinput
%option nounput
%x COML COMB STR
%%
^.*                             {
                                    if (!line_flag)
                                    {
                                        strcpy(cnt_line, yytext);
                                        line_flag++;
                                    }
                                    REJECT;
                                }
[ \t]                           { count_char++; }
int|char                        { strcpy(yylval.type, yytext); last_token = yyleng; count_char+= yyleng; return TYPE; }
void                            { strcpy(yylval.vide, yytext); last_token = yyleng; count_char+= yyleng; return VOID; }
struct                          { last_token = yyleng; count_char+= yyleng; return STRUCT; }
print                           { last_token = yyleng; count_char+= yyleng; return PRINT; }
readc                           { last_token = yyleng; count_char+= yyleng; return READC; }
reade                           { last_token = yyleng; count_char+= yyleng; return READE; }
return                          { last_token = yyleng; count_char+= yyleng; return RETURN; }
if                              { last_token = yyleng; count_char+= yyleng; return IF; }
else                            { last_token = yyleng; count_char+= yyleng; return ELSE; }
"=="|"!="                       { strcpy(yylval.comp, yytext); last_token = yyleng; count_char+= yyleng; return EQ; }
while                           { last_token = yyleng; count_char+= yyleng; return WHILE; }
"<="|">="|"<"|">"               { strcpy(yylval.comp, yytext); last_token = yyleng; count_char+= yyleng; return ORDER; }
[/][/]                          { count_char+= yyleng; BEGIN COML; }
[/][*]                          { count_char+= yyleng; BEGIN COMB; }
["]                             { count_char+= yyleng; BEGIN STR; }
[0-9]+                          { yylval.integer = atoi(yytext); last_token = yyleng; count_char+= yyleng; return NUM; }
['].[']|[']"\\"[0abtnvfr'][']    {
                                    if (yyleng == 3)
                                        yylval.character = yytext[1];
                                    else
                                    {
                                        switch (yytext[2])
                                        {
                                            case ('n') : yylval.character = '\n';
                                                break;
                                            case ('a') : yylval.character = '\a';
                                                break;
                                            case ('r') : yylval.character = '\r';
                                                break;
                                            case ('t') : yylval.character = '\t';
                                                break;
                                            case ('f') : yylval.character = '\f';
                                                break;
                                            case ('v') : yylval.character = '\v';
                                                break;
                                            case ('b') : yylval.character = '\b';
                                                break;
                                            case ('\'') : yylval.character = '\'';
                                                break;
                                            case ('\\') : yylval.character = '\\';
                                                break;
                                            case ('\"') : yylval.character = '\"';
                                                break;
                                            default : yylval.character = '\0';
                                                break;
                                        }
                                    }
                                    last_token = yyleng;
                                    count_char+= yyleng;
                                    return CHARACTER;
                                }
[&][&]                          { last_token = yyleng; count_char+= yyleng; return AND; }
[\|][\|]                        { last_token = yyleng; count_char+= yyleng; return OR; }
[*]|[/]|[%]                     { yylval.divstar = yytext[0]; last_token = yyleng; count_char+= yyleng; return DIVSTAR; }
[+]|-                           { yylval.addsub = yytext[0]; last_token = yyleng; count_char+= yyleng; return ADDSUB; }
[a-zA-Z_][a-zA-Z0-9_]*          { strcpy(yylval.identifier, yytext); last_token = yyleng; count_char+= yyleng; return IDENT; }
.                               { last_token = yyleng; count_char++; return (yytext[0]); }
\n                              { count_char = 0; count_line++; line_flag = 0; }

<COML>.                         { count_char++; }
<COML>\n                        { count_char = 0; count_line++; line_flag = 0; BEGIN INITIAL; }

<COMB>.                         { count_char++; }
<COMB>\n                        { count_char = 0; count_line++; line_flag = 0; }
<COMB>[*][/]                    { count_char+= yyleng; BEGIN INITIAL; }

<STR>\n                         { count_char = 0; count_line++; line_flag = 0; }
<STR>[^"]                       { count_char++; }
<STR>["]                        { count_char++; BEGIN INITIAL; return CHARACTER; }
%%
