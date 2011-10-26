/* Scan Bison Skeletons.                                       -*- C -*-

   Copyright (C) 2001-2007, 2009-2011 Free Software Foundation, Inc.

   This file is part of Bison, the GNU Compiler Compiler.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

%option nodefault noyywrap noinput nounput never-interactive debug
%option prefix="skel_" outfile="lex.yy.c"

%{
/* Work around a bug in flex 2.5.31.  See Debian bug 333231
   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef skel_wrap
#define skel_wrap() 1

#define FLEX_PREFIX(Id) skel_ ## Id
#include "flex-scanner.h"

#include <dirname.h>
#include <error.h>
#include <quotearg.h>

#include "complain.h"
#include "getargs.h"
#include "files.h"
#include "scan-skel.h"

#define YY_DECL static int skel_lex (void)
YY_DECL;

#define QPUTS(String) \
   fputs (quotearg_style (c_quoting_style, String), yyout)

static void at_directive_perform (int at_directive_argc,
                                  char *at_directive_argv[],
                                  char **outnamep, int *out_linenop);
static void fail_for_at_directive_too_many_args (char const *at_directive_name);
static void fail_for_at_directive_too_few_args (char const *at_directive_name);
static void fail_for_invalid_at (char const *at);
%}

%x SC_AT_DIRECTIVE_ARGS
%x SC_AT_DIRECTIVE_SKIP_WS

%%

%{
  int out_lineno PACIFY_CC (= 0);
  char *outname = NULL;

  /* Currently, only the @warn, @complain, @fatal, @warn_at, @complain_at, and
     @fatal_at directives take multiple arguments, and the last three already
     can't take more than 7.  at_directive_argv[0] is the directive name.  */
  #define AT_DIRECTIVE_ARGC_MAX 8
  int at_directive_argc = 0;
  char *at_directive_argv[AT_DIRECTIVE_ARGC_MAX];
%}

"@@" fputc ('@', yyout);
"@{" fputc ('[', yyout);
"@}" fputc (']', yyout);
"@`" /* Emtpy.  Used by b4_cat in ../data/bison.m4.  */
@\n  /* Likewise.  */

"@oline@"  fprintf (yyout, "%d", out_lineno + 1);
"@ofile@"  QPUTS (outname);
"@dir_prefix@" QPUTS (dir_prefix);

@[a-z_]+"(" {
  yytext[yyleng-1] = '\0';
  obstack_grow (&obstack_for_string, yytext, yyleng);
  at_directive_argv[at_directive_argc++] =
    obstack_finish (&obstack_for_string);
  BEGIN SC_AT_DIRECTIVE_ARGS;
}

  /* This pattern must not match more than the previous @ patterns. */
@[^@{}`(\n]* fail_for_invalid_at (yytext);
\n	   out_lineno++; ECHO;
[^@\n]+	   ECHO;

<INITIAL><<EOF>> {
  if (outname)
    {
      free (outname);
      xfclose (yyout);
    }
  return EOF;
}

<SC_AT_DIRECTIVE_ARGS>{
  [^@]+ { STRING_GROW; }

  "@@" { obstack_1grow (&obstack_for_string, '@'); }
  "@{" { obstack_1grow (&obstack_for_string, '['); }
  "@}" { obstack_1grow (&obstack_for_string, ']'); }
  "@`" /* Empty.  Useful for starting an argument
          that begins with whitespace. */
  @\n  /* Empty.  */

  @[,)] {
    if (at_directive_argc >= AT_DIRECTIVE_ARGC_MAX)
      fail_for_at_directive_too_many_args (at_directive_argv[0]);

    obstack_1grow (&obstack_for_string, '\0');
    at_directive_argv[at_directive_argc++] =
      obstack_finish (&obstack_for_string);

    /* Like M4, skip whitespace after a comma.  */
    if (yytext[1] == ',')
      BEGIN SC_AT_DIRECTIVE_SKIP_WS;
    else
      {
        at_directive_perform (at_directive_argc, at_directive_argv,
                              &outname, &out_lineno);
        obstack_free (&obstack_for_string, at_directive_argv[0]);
        at_directive_argc = 0;
        BEGIN INITIAL;
      }
  }

  @.? { fail_for_invalid_at (yytext); }
}

<SC_AT_DIRECTIVE_SKIP_WS>{
  [ \t\r\n]
  . { yyless (0); BEGIN SC_AT_DIRECTIVE_ARGS; }
}

<SC_AT_DIRECTIVE_ARGS,SC_AT_DIRECTIVE_SKIP_WS>{
  <<EOF>> {
    fatal (_("unclosed %s directive in skeleton"), at_directive_argv[0]);
  }
}

%%

/*------------------------.
| Scan a Bison skeleton.  |
`------------------------*/

void
scan_skel (FILE *in)
{
  static bool initialized = false;
  if (!initialized)
    {
      initialized = true;
      obstack_init (&obstack_for_string);
    }
  skel_in = in;
  skel__flex_debug = trace_flag & trace_skeleton;
  skel_lex ();
}

void
skel_scanner_free (void)
{
  obstack_free (&obstack_for_string, 0);
  /* Reclaim Flex's buffers.  */
  yylex_destroy ();
}

static void
at_directive_perform (int at_directive_argc,
                      char *at_directive_argv[],
                      char **outnamep, int *out_linenop)
{
  if (0 == strcmp (at_directive_argv[0], "@basename"))
    {
      if (at_directive_argc > 2)
        fail_for_at_directive_too_many_args (at_directive_argv[0]);
      fputs (last_component (at_directive_argv[1]), yyout);
    }
  else if (0 == strcmp (at_directive_argv[0], "@warn")
           || 0 == strcmp (at_directive_argv[0], "@complain")
           || 0 == strcmp (at_directive_argv[0], "@fatal"))
    {
      void (*func)(char const *, ...);
      switch (at_directive_argv[0][1])
        {
          case 'w': func = warn; break;
          case 'c': func = complain; break;
          case 'f': func = fatal; break;
          default: aver (false); break;
        }
      switch (at_directive_argc)
        {
          case 2:
            func (_(at_directive_argv[1]));
            break;
          case 3:
            func (_(at_directive_argv[1]), at_directive_argv[2]);
            break;
          case 4:
            func (_(at_directive_argv[1]), at_directive_argv[2],
                  at_directive_argv[3]);
            break;
          case 5:
            func (_(at_directive_argv[1]), at_directive_argv[2],
                  at_directive_argv[3], at_directive_argv[4]);
            break;
          case 6:
            func (_(at_directive_argv[1]), at_directive_argv[2],
                  at_directive_argv[3], at_directive_argv[4],
                  at_directive_argv[5]);
            break;
          default:
            fail_for_at_directive_too_many_args (at_directive_argv[0]);
            break;
        }
    }
  else if (0 == strcmp (at_directive_argv[0], "@warn_at")
           || 0 == strcmp (at_directive_argv[0], "@complain_at")
           || 0 == strcmp (at_directive_argv[0], "@fatal_at"))
    {
      void (*func)(location, char const *, ...);
      location loc;
      if (at_directive_argc < 4)
        fail_for_at_directive_too_few_args (at_directive_argv[0]);
      switch (at_directive_argv[0][1])
        {
          case 'w': func = warn_at; break;
          case 'c': func = complain_at; break;
          case 'f': func = fatal_at; break;
          default: aver (false); break;
        }
      boundary_set_from_string (&loc.start, at_directive_argv[1]);
      boundary_set_from_string (&loc.end, at_directive_argv[2]);
      switch (at_directive_argc)
        {
          case 4:
            func (loc, _(at_directive_argv[3]));
            break;
          case 5:
            func (loc, _(at_directive_argv[3]), at_directive_argv[4]);
            break;
          case 6:
            func (loc, _(at_directive_argv[3]), at_directive_argv[4],
                  at_directive_argv[5]);
            break;
          case 7:
            func (loc, _(at_directive_argv[3]), at_directive_argv[4],
                  at_directive_argv[5], at_directive_argv[6]);
            break;
          case 8:
            func (loc, _(at_directive_argv[3]), at_directive_argv[4],
                  at_directive_argv[5], at_directive_argv[6],
                  at_directive_argv[7]);
            break;
          default:
            fail_for_at_directive_too_many_args (at_directive_argv[0]);
            break;
        }
    }
  else if (0 == strcmp (at_directive_argv[0], "@output"))
    {
      if (at_directive_argc > 2)
        fail_for_at_directive_too_many_args (at_directive_argv[0]);
      if (*outnamep)
        {
          free (*outnamep);
          xfclose (yyout);
        }
      *outnamep = xstrdup (at_directive_argv[1]);
      output_file_name_check (outnamep);
      yyout = xfopen (*outnamep, "w");
      *out_linenop = 1;
    }
  else
    fail_for_invalid_at (at_directive_argv[0]);
}

static void
fail_for_at_directive_too_few_args (char const *at_directive_name)
{
  fatal (_("too few arguments for %s directive in skeleton"),
         at_directive_name);
}

static void
fail_for_at_directive_too_many_args (char const *at_directive_name)
{
  fatal (_("too many arguments for %s directive in skeleton"),
         at_directive_name);
}

static void
fail_for_invalid_at (char const *at)
{
  fatal ("invalid @ in skeleton: %s", at);
}
