/* Issue #64: this is the actual grammar attached to the original report
   (rpcalc___win_bison_2.5.23_crash.zip from
   https://github.com/lexxmark/winflexbison/issues/64), byte-for-byte aside
   from CRLF->LF normalization to match the other golden-cases fixtures. It's
   the first example from chapter 2 of the Bison manual with a typo: the
   `printf` action on line ~21 is missing its opening double-quote. win_bison
   2.5.23 crashed via a NULL pointer dereference while processing this file
   instead of reporting a clean diagnostic. */

%{
    #include <stdio.h>
    #include <math.h>
    int yylex(void);
    void yyerror(char const *);
%}

%define api.value.type {double}
%token NUM

%% /* Grammar rules and actions follow. */
input:
    %empty
    | input line
    ;

line:
    '\n'
    | exp '\n'      { printf(%.10g\n", $1); }
    ;

exp:
   NUM
    | exp exp '+'   { $$ = $1 + $2; }
    | exp exp '-'   { $$ = $1 - $2; }
    | exp exp '*'   { $$ = $1 * $2; }
    | exp exp '/'   { $$ = $1 / $2; }
    | exp exp '^'   { $$ = pow($1, $2); } /* exponentiation */
    | exp 'n'       { $$ = -$1; } /* unary minus */
    ;
%%

/* Lex for RPC */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int yylex(void)
{
    int c = getchar();
    /* Skip white space */
    while (c == ' ' || c == '\t') {
        c = getchar();
    }

    /* Process numbers */
    if (c == '.' || isdigit(c)) {
        ungetc(c, stdin);
        if (scanf("%lf", &yylval) != 1) {
            abort();
        }
        return NUM;
    }

    /* Return end of input */
    else if (c == EOF) {
        return YYEOF;
    }

    /* Return single char otherwise */
    else {
        return c;
    }
} /* end of function yylex */

int main(void)
{
    return yyparse();
} /* end of function main */

/* Error handler */
void yyerror(char const *s)
{
    fprintf(stderr, "%s\n", s);
} /* end of function yyerror */
