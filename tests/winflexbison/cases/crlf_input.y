/* A grammar stored with CRLF line endings, for winflexbison.bison_output_is_lf.
   Kept CRLF by tests/winflexbison/.gitattributes; the test verifies that before
   using it, so a normalizing checkout fails loudly instead of quietly testing
   nothing.

   The point is the copied text: a prologue, a mid-rule action and an epilogue
   are pasted into the generated parser verbatim, so if the input's CRs were not
   stripped on the way in they would surface in the output. */
%{
#include <stdio.h>
int  yylex(void);
void yyerror(const char *s);
%}

%define api.value.type {int}
%token NUM

%%

input
  : /* empty */
  | input line
  ;

line
  : NUM       { printf("%d\n", $1); }
  ;

%%

void yyerror(const char *s)
{
  fprintf(stderr, "%s\n", s);
}
