/*
 * Issue #70 regression fixture -- winflexbison's own, not from upstream flex.
 *
 * The bug needs all three of these options at once: c++, noyywrap, and a
 * generated header. flex emits its prolog once per output stream, so the
 * yywrap definition it writes when noyywrap is set lands in BOTH this
 * scanner's .cc and the .hpp below. Any program that includes the header and
 * links the scanner then has the symbol twice -- LNK2005 under MSVC.
 *
 * Drop any one of the three options and nothing goes wrong, which is why
 * cxx_basic.ll (c++ and noyywrap, but no header and its own main) never
 * caught it.
 *
 * The scanner itself does nothing interesting on purpose: the assertion is
 * that flextest_cxx_noyywrap_header LINKS at all. Running it afterwards only
 * confirms the binary is sane.
 */

%{
#include "config.h"
%}

%option 8bit warn c++
%option nounput noinput nomain noyywrap
%option header="cxx_noyywrap_header.hpp"

%%

.|\n           { }

%%
