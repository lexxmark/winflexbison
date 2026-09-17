/*
 * Issue #70 regression fixture -- winflexbison's own, not from upstream flex.
 *
 * This is the half that makes the test work: a SECOND translation unit that
 * includes the generated header. The bug is a duplicate definition across two
 * objects, so it cannot show up in a single-file test no matter which options
 * are set -- it takes this file plus cxx_noyywrap_header.cc to produce the two
 * copies the linker rejects.
 *
 * Mirrors the reproducer from the issue report (main.cpp there).
 */

#include <iostream>

#include "cxx_noyywrap_header.hpp"

int
main (void)
{
    yyFlexLexer f;
    f.switch_streams(&std::cin, &std::cout);
    f.yylex();
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
