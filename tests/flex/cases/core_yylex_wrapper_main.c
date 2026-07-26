/* Issue #8: a reentrant, bison-bridge scanner paired with a pure parser
   (the same combination bison_yylloc/_yylval/_nr already exercise) is where
   the size_t->int narrowing in yyuserAction()/yyfill() and the yylex
   macro-redefinition historically showed up. This target compiles that
   combination with the two specific warnings the issue named promoted to
   errors (see add_flex_bison_test(core_yylex_wrapper EXTRA_COMPILE_OPTIONS
   ...)); a clean compile is the regression guard. */
#include "core_yylex_wrapper_parser.h"
#include "core_yylex_wrapper_scanner.h"

int main(int argc, char** argv)
{
    yyscan_t scanner;
    (void)argc;
    (void)argv;
    testlex_init(&scanner);
    testset_in(stdin, scanner);
    testparse(scanner);
    testlex_destroy(scanner);
    return 0;
}
