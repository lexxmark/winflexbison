/* Issue #44: %define lr.type canonical-lr must actually be honored (not
   silently ignored). Same self-contained calculator grammar as calc.y, with
   the table-construction algorithm overridden; pass = win_bison accepts the
   option and the generated parser still parses calc.txt correctly. */
%define lr.type canonical-lr
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
