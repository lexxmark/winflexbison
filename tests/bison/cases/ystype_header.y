/* Issue #7: a custom YYSTYPE typedef declared in the grammar must propagate
   into the generated --defines header, not just the .c. A regression once
   left the header's YYSTYPE defaulted to int (the yacc default) while the
   .c used the real type, causing a type mismatch across the two files.
   %code requires is the documented mechanism for declarations shared by
   both generated files, so this is what the header's YYSTYPE must reflect. */
%code requires {
typedef char* MYSTYPE;
#define YYSTYPE MYSTYPE
}
%defines

%{
#include <stdio.h>
int yylex(void);
void yyerror(const char *s);
%}

%token WORD

%%
input : %empty
      | input line
      ;
line  : WORD { printf("got: %s\n", $1); }
      ;
%%

int yylex(void)
{
    static int done = 0;
    static char word[] = "hello";
    if (done)
        return 0;
    done = 1;
    yylval = word;
    return WORD;
}

void yyerror(const char *s) { fprintf(stderr, "error: %s\n", s); }
