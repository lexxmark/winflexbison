%{
#include <stdio.h>
#include <stdlib.h>
#include "core_yylex_wrapper_parser.h"
#include "config.h"
%}

%option 8bit prefix="test"
%option reentrant bison-bridge
%option header="core_yylex_wrapper_scanner.h"
%option nomain nounput noyy_top_state noyywrap nodefault noinput warn

%%
[0-9]+ { yylval->num = atoi(yytext); return NUMBER; }
\r|\n   { }
.       { yyterminate(); }
%%
