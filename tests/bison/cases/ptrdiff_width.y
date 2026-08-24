/* Issue #95: YYPTRDIFF_T must be as wide as a pointer difference.

   MSVC defines neither __PTRDIFF_TYPE__ nor (unless the caller asked for
   /std:c11 or later) PTRDIFF_MAX, so the skeleton's width ladder used to
   land on its last resort, "long" -- 32 bits on 64-bit Windows, where
   ptrdiff_t is 64. Every x64 build of a generated parser then warned
   C4244 on "YYPTRDIFF_T yysize = yyssp - yyss + 1".

   The check below is the property behind that warning, so it holds for any
   compiler; to see the warning itself, compile the generated .c with
   cl /W3 /we4244. On Win32 both types are 32 bits and the check is
   vacuously true -- as it should be, the bug does not exist there.

   Note this file must not include <stdint.h>: the user prologue is emitted
   ahead of the skeleton's ladder, so pulling in PTRDIFF_MAX here would
   satisfy the branch above the broken one and hide what is being tested.
   <stddef.h> gives us ptrdiff_t without that side effect. */
%{
#include <stddef.h>
#include <stdio.h>
int yylex(void);
void yyerror(const char *s);
%}

%token NUM

%%
input : %empty
      | input NUM
      ;
%%

int yylex(void) { return 0; }

void yyerror(const char *s) { fprintf(stderr, "error: %s\n", s); }

int main(void)
{
    if (yyparse() != 0)
        return 1;

    if (sizeof(YYPTRDIFF_T) != sizeof(ptrdiff_t))
    {
        fprintf(stderr,
                "YYPTRDIFF_T is %d byte(s), ptrdiff_t is %d: generated"
                " parsers would truncate stack sizes (issue #95)\n",
                (int)sizeof(YYPTRDIFF_T), (int)sizeof(ptrdiff_t));
        return 1;
    }

    /* And the advertised maximum must be that type's real maximum: it is
       what the parser checks stack growth against. */
    if (YYPTRDIFF_MAXIMUM != (YYPTRDIFF_T)((~(size_t)0) >> 1))
    {
        fprintf(stderr, "YYPTRDIFF_MAXIMUM does not match the width of"
                        " YYPTRDIFF_T (issue #95)\n");
        return 1;
    }

    return 0;
}
