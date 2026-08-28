/* Issue #8: two warnings appeared in generated code after a version bump:
   (1) size_t->int narrowing in yyuserAction()/yyfill(), and (2) a yylex
   macro redefinition. Both historically showed up with a reentrant,
   bison-bridge flex scanner paired with a pure bison parser -- the same
   combination bison_yylloc/_yylval/_nr already exercise. This parser pairs
   with such a scanner (core_yylex_wrapper_scanner.l). */
%parse-param { void* scanner }
%lex-param   { void* scanner }

%{
#include <stdio.h>
#include "config.h"
#include "core_yylex_wrapper_parser.h"
#include "core_yylex_wrapper_scanner.h"

int yyerror(void* scanner, const char* msg);
%}

%define api.pure

%union {
    int num;
}
%token <num> NUMBER

%%

input:
    NUMBER { printf("got: %d\n", $1); }
    ;

%%

int yyerror(void* scanner, const char* msg)
{
    (void)scanner;
    fprintf(stderr, "%s\n", msg);
    return 0;
}
