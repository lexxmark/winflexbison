/* Self-contained calculator grammar: integrated scanner + main, returns 0 on a
   clean parse and non-zero on a syntax error. This is an exit-code (flex-style)
   compile-run test: win_bison generates the parser, MSVC compiles it, and the
   exe is run with input on stdin. No golden output needed. */
%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
int  yylex(void);
void yyerror(const char *s);
%}
%define api.value.type {int}
%token NUM
%left '+' '-'
%left '*' '/'
%%
input : %empty
      | input line
      ;
line  : '\n'
      | exp '\n'        { printf("= %d\n", $1); }
      ;
exp   : NUM             { $$ = $1; }
      | exp '+' exp     { $$ = $1 + $3; }
      | exp '-' exp     { $$ = $1 - $3; }
      | exp '*' exp     { $$ = $1 * $3; }
      | exp '/' exp     { $$ = $3 ? $1 / $3 : 0; }
      | '(' exp ')'     { $$ = $2; }
      ;
%%
int yylex(void)
{
    int c;
    do { c = getchar(); } while (c == ' ' || c == '\t' || c == '\r');
    if (c == EOF) return 0;
    if (isdigit(c)) {
        int n = 0;
        do { n = n * 10 + (c - '0'); c = getchar(); } while (isdigit(c));
        if (c != EOF) ungetc(c, stdin);
        yylval = n;
        return NUM;
    }
    return c;
}

void yyerror(const char *s) { fprintf(stderr, "error: %s\n", s); }

int main(void) { return yyparse(); }
