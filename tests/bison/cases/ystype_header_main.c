/* Issue #7 regression guard: this TU only sees the generated --defines
   header (not the .c), so it forces a real compile error if the header's
   YYSTYPE ever falls back to the yacc default `int` instead of the `char*`
   typedef declared via %code requires in ystype_header.y. */
#include "ystype_header.h"
#include <string.h>

int main(void)
{
    YYSTYPE probe = "probe";
    if (strlen(probe) == 0)
        return 1;
    return yyparse();
}
