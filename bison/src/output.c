/* Output the generated parsing program for Bison.

   Copyright (C) 1984, 1986, 1989, 1992, 2000-2013 Free Software
   Foundation, Inc.

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

#include <config.h>
#include "system.h"
#include <mbstring.h>

//#include <concat-filename.h>
//#include <configmake.h>
//#include <filename.h>
#include <error.h>
#include <get-errno.h>
#include <quotearg.h>
//#include <spawn-pipe.h>
#include <timevar.h>
//#include <wait-process.h>

#include "complain.h"
#include "files.h"
#include "getargs.h"
#include "gram.h"
#include "muscle-tab.h"
#include "output.h"
#include "reader.h"
#include "scan-code.h"    /* max_left_semantic_context */
#include "scan-skel.h"
#include "symtab.h"
#include "tables.h"

# define ARRAY_CARDINALITY(Array) (sizeof (Array) / sizeof *(Array))

static struct obstack format_obstack;


/*-------------------------------------------------------------------.
| Create a function NAME which associates to the muscle NAME the     |
| result of formatting the FIRST and then TABLE_DATA[BEGIN..END[ (of |
| TYPE), and to the muscle NAME_max, the max value of the            |
| TABLE_DATA.                                                        |
`-------------------------------------------------------------------*/


#define GENERATE_MUSCLE_INSERT_TABLE(Name, Type)                        \
                                                                        \
static void                                                             \
Name (char const *name,                                                 \
      Type *table_data,                                                 \
      Type first,                                                       \
      int begin,                                                        \
      int end)                                                          \
{                                                                       \
  Type min = first;                                                     \
  Type max = first;                                                     \
  long int lmin;                                                        \
  long int lmax;                                                        \
  int i;                                                                \
  int j = 1;                                                            \
                                                                        \
  obstack_printf (&format_obstack, "%6d", first);                       \
  for (i = begin; i < end; ++i)                                         \
    {                                                                   \
      obstack_1grow (&format_obstack, ',');                             \
      if (j >= 10)                                                      \
        {                                                               \
          obstack_sgrow (&format_obstack, "\n  ");                      \
          j = 1;                                                        \
        }                                                               \
      else                                                              \
        ++j;                                                            \
      obstack_printf (&format_obstack, "%6d", table_data[i]);           \
      if (table_data[i] < min)                                          \
        min = table_data[i];                                            \
      if (max < table_data[i])                                          \
        max = table_data[i];                                            \
    }                                                                   \
  muscle_insert (name, obstack_finish0 (&format_obstack));              \
                                                                        \
  lmin = min;                                                           \
  lmax = max;                                                           \
  /* Build 'NAME_min' and 'NAME_max' in the obstack. */                 \
  obstack_printf (&format_obstack, "%s_min", name);                     \
  MUSCLE_INSERT_LONG_INT (obstack_finish0 (&format_obstack), lmin);     \
  obstack_printf (&format_obstack, "%s_max", name);                     \
  MUSCLE_INSERT_LONG_INT (obstack_finish0 (&format_obstack), lmax);     \
}

GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_unsigned_int_table, unsigned int)
GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_int_table, int)
GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_base_table, base_number)
GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_rule_number_table, rule_number)
GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_symbol_number_table, symbol_number)
GENERATE_MUSCLE_INSERT_TABLE (muscle_insert_state_number_table, state_number)

/*----------------------------------------------------------------.
| Print to OUT a representation of CP quoted and escaped for M4.  |
`----------------------------------------------------------------*/

static void
quoted_output (FILE *out, char const *cp)
{
  fprintf (out, "[[");

  for (; *cp; cp++)
    switch (*cp)
      {
      case '$': fputs ("$][", out); break;
      case '@': fputs ("@@",  out); break;
      case '[': fputs ("@{",  out); break;
      case ']': fputs ("@}",  out); break;
      default:  fputc (*cp,   out); break;
      }

  fprintf (out, "]]");
}

/*----------------------------------------------------------------.
| Print to OUT a representation of STRING quoted and escaped both |
| for C and M4.                                                   |
`----------------------------------------------------------------*/

static void
string_output (FILE *out, char const *string)
{
  quoted_output (out, quotearg_style (c_quoting_style, string));
}


/*------------------------------------------------------------------.
| Prepare the muscles related to the symbols: translate, tname, and |
| toknum.                                                           |
`------------------------------------------------------------------*/

static void
prepare_symbols (void)
{
  MUSCLE_INSERT_INT ("tokens_number", ntokens);
  MUSCLE_INSERT_INT ("nterms_number", nvars);
  MUSCLE_INSERT_INT ("symbols_number", nsyms);
  MUSCLE_INSERT_INT ("undef_token_number", undeftoken->number);
  MUSCLE_INSERT_INT ("user_token_number_max", max_user_token_number);

  muscle_insert_symbol_number_table ("translate",
                                     token_translations,
                                     token_translations[0],
                                     1, max_user_token_number + 1);

  /* tname -- token names.  */
  {
    int i;
    /* We assume that the table will be output starting at column 2. */
    int j = 2;
    struct quoting_options *qo = clone_quoting_options (0);
    set_quoting_style (qo, c_quoting_style);
    set_quoting_flags (qo, QA_SPLIT_TRIGRAPHS);
    for (i = 0; i < nsyms; i++)
      {
        char *cp = quotearg_alloc (symbols[i]->tag, -1, qo);
        /* Width of the next token, including the two quotes, the
           comma and the space.  */
        int width = strlen (cp) + 2;

        if (j + width > 75)
          {
            obstack_sgrow (&format_obstack, "\n ");
            j = 1;
          }

        if (i)
          obstack_1grow (&format_obstack, ' ');
        obstack_escape (&format_obstack, cp);
        free (cp);
        obstack_1grow (&format_obstack, ',');
        j += width;
      }
    free (qo);
    obstack_sgrow (&format_obstack, " ]b4_null[");

    /* Finish table and store. */
    muscle_insert ("tname", obstack_finish0 (&format_obstack));
  }

  /* Output YYTOKNUM. */
  {
    int i;
    int *values = xnmalloc (ntokens, sizeof *values);
    for (i = 0; i < ntokens; ++i)
      values[i] = symbols[i]->user_token_number;
    muscle_insert_int_table ("toknum", values,
                             values[0], 1, ntokens);
    free (values);
  }
}


/*----------------------------------------------------------------.
| Prepare the muscles related to the rules: r1, r2, rline, dprec, |
| merger, immediate.                                              |
`----------------------------------------------------------------*/

static void
prepare_rules (void)
{
  unsigned int *rline = xnmalloc (nrules, sizeof *rline);
  symbol_number *r1 = xnmalloc (nrules, sizeof *r1);
  unsigned int *r2 = xnmalloc (nrules, sizeof *r2);
  int *dprec = xnmalloc (nrules, sizeof *dprec);
  int *merger = xnmalloc (nrules, sizeof *merger);
  int *immediate = xnmalloc (nrules, sizeof *immediate);

  rule_number r;
  for (r = 0; r < nrules; ++r)
    {
      /* LHS of the rule R. */
      r1[r] = rules[r].lhs->number;
      /* Length of rule R's RHS. */
      r2[r] = rule_rhs_length (&rules[r]);
      /* Line where rule was defined. */
      rline[r] = rules[r].location.start.line;
      /* Dynamic precedence (GLR).  */
      dprec[r] = rules[r].dprec;
      /* Merger-function index (GLR).  */
      merger[r] = rules[r].merger;
      /* Immediate reduction flags (GLR).  */
      immediate[r] = rules[r].is_predicate;
    }

  muscle_insert_unsigned_int_table ("rline", rline, 0, 0, nrules);
  muscle_insert_symbol_number_table ("r1", r1, 0, 0, nrules);
  muscle_insert_unsigned_int_table ("r2", r2, 0, 0, nrules);
  muscle_insert_int_table ("dprec", dprec, 0, 0, nrules);
  muscle_insert_int_table ("merger", merger, 0, 0, nrules);
  muscle_insert_int_table ("immediate", immediate, 0, 0, nrules);

  MUSCLE_INSERT_INT ("rules_number", nrules);
  MUSCLE_INSERT_INT ("max_left_semantic_context", max_left_semantic_context);

  free (rline);
  free (r1);
  free (r2);
  free (dprec);
  free (merger);
  free (immediate);
}

/*--------------------------------------------.
| Prepare the muscles related to the states.  |
`--------------------------------------------*/

static void
prepare_states (void)
{
  state_number i;
  symbol_number *values = xnmalloc (nstates, sizeof *values);
  for (i = 0; i < nstates; ++i)
    values[i] = states[i]->accessing_symbol;
  muscle_insert_symbol_number_table ("stos", values,
                                     0, 1, nstates);
  free (values);

  MUSCLE_INSERT_INT ("last", high);
  MUSCLE_INSERT_INT ("final_state_number", final_state->number);
  MUSCLE_INSERT_INT ("states_number", nstates);
}


/*-------------------------------------------------------.
| Compare two symbols by type-name, and then by number.  |
`-------------------------------------------------------*/

static int
symbol_type_name_cmp (const symbol **lhs, const symbol **rhs)
{
  int res = uniqstr_cmp ((*lhs)->type_name, (*rhs)->type_name);
  if (!res)
    res = (*lhs)->number - (*rhs)->number;
  return res;
}


/*----------------------------------------------------------------.
| Return a (malloc'ed) table of the symbols sorted by type-name.  |
`----------------------------------------------------------------*/

static symbol **
symbols_by_type_name (void)
{
  typedef int (*qcmp_type) (const void *, const void *);
  symbol **res = xmemdup (symbols, nsyms * sizeof *res);
  qsort (res, nsyms, sizeof *res, (qcmp_type) &symbol_type_name_cmp);
  return res;
}


/*------------------------------------------------------------------.
| Define b4_type_names, which is a list of (lists of the numbers of |
| symbols with same type-name).                                     |
`------------------------------------------------------------------*/

static void
type_names_output (FILE *out)
{
  int i;
  symbol **syms = symbols_by_type_name ();
  fputs ("m4_define([b4_type_names],\n[", out);
  for (i = 0; i < nsyms; /* nothing */)
    {
      /* The index of the first symbol of the current type-name.  */
      int i0 = i;
      fputs (i ? ",\n[" : "[", out);
      for (; i < nsyms && syms[i]->type_name == syms[i0]->type_name; ++i)
        fprintf (out, "%s%d", i != i0 ? ", " : "", syms[i]->number);
      fputs ("]", out);
    }
  fputs ("])\n\n", out);
  free (syms);
}


/*-------------------------------------.
| The list of all the symbol numbers.  |
`-------------------------------------*/

static void
symbol_numbers_output (FILE *out)
{
  int i;
  fputs ("m4_define([b4_symbol_numbers],\n[", out);
  for (i = 0; i < nsyms; ++i)
    fprintf (out, "%s[%d]", i ? ", " : "", i);
  fputs ("])\n\n", out);
}


/*---------------------------------.
| Output the user actions to OUT.  |
`---------------------------------*/

static void
user_actions_output (FILE *out)
{
  rule_number r;

  fputs ("m4_define([b4_actions], \n[", out);
  for (r = 0; r < nrules; ++r)
    if (rules[r].action)
      {
        fprintf (out, "b4_%scase(%d, [b4_syncline(%d, ",
                 rules[r].is_predicate ? "predicate_" : "",
                 r + 1, rules[r].action_location.start.line);
        string_output (out, rules[r].action_location.start.file);
        fprintf (out, ")\n[    %s]])\n\n", rules[r].action);
      }
  fputs ("])\n\n", out);
}

/*------------------------------------.
| Output the merge functions to OUT.  |
`------------------------------------*/

static void
merger_output (FILE *out)
{
  int n;
  merger_list* p;

  fputs ("m4_define([b4_mergers], \n[[", out);
  for (n = 1, p = merge_functions; p != NULL; n += 1, p = p->next)
    {
      if (p->type[0] == '\0')
        fprintf (out, "  case %d: *yy0 = %s (*yy0, *yy1); break;\n",
                 n, p->name);
      else
        fprintf (out, "  case %d: yy0->%s = %s (*yy0, *yy1); break;\n",
                 n, p->type, p->name);
    }
  fputs ("]])\n\n", out);
}


/*---------------------------------------------.
| Prepare the muscles for symbol definitions.  |
`---------------------------------------------*/

static void
prepare_symbol_definitions (void)
{
  int i;
  for (i = 0; i < nsyms; ++i)
    {
      symbol *sym = symbols[i];
      const char *key;
      const char *value;

#define SET_KEY(Entry)                                          \
      obstack_printf (&format_obstack, "symbol(%d, %s)",        \
                      i, Entry);                                \
      key = obstack_finish0 (&format_obstack);

#define SET_KEY2(Entry, Suffix)                                 \
      obstack_printf (&format_obstack, "symbol(%d, %s_%s)",     \
                      i, Entry, Suffix);                        \
      key = obstack_finish0 (&format_obstack);

      /* Whether the symbol has an identifier.  */
      value = symbol_id_get (sym);
      SET_KEY ("has_id");
      MUSCLE_INSERT_INT (key, !!value);

      /* Its identifier.  */
      SET_KEY ("id");
      MUSCLE_INSERT_STRING (key, value ? value : "");

      /* Its tag.  Typically for documentation purpose.  */
      SET_KEY ("tag");
      MUSCLE_INSERT_STRING (key, sym->tag);

      SET_KEY ("user_number");
      MUSCLE_INSERT_INT (key, sym->user_token_number);

      SET_KEY ("is_token");
      MUSCLE_INSERT_INT (key,
                         i < ntokens && sym != errtoken && sym != undeftoken);

      SET_KEY ("number");
      MUSCLE_INSERT_INT (key, sym->number);

      SET_KEY ("has_type");
      MUSCLE_INSERT_INT (key, !!sym->type_name);

      SET_KEY ("type");
      MUSCLE_INSERT_STRING (key, sym->type_name ? sym->type_name : "");

      {
        int j;
        for (j = 0; j < CODE_PROPS_SIZE; ++j)
          {
            /* "printer", not "%printer".  */
            char const *pname = code_props_type_string (j) + 1;
            code_props const *p = symbol_code_props_get (sym, j);
            SET_KEY2 ("has", pname);
            MUSCLE_INSERT_INT (key, !!p->code);

            if (p->code)
              {
                SET_KEY2 (pname, "file");
                MUSCLE_INSERT_STRING (key, p->location.start.file);

                SET_KEY2 (pname, "line");
                MUSCLE_INSERT_INT (key, p->location.start.line);

                SET_KEY (pname);
                MUSCLE_INSERT_STRING_RAW (key, p->code);
              }
          }
      }
#undef SET_KEY2
#undef SET_KEY
    }
}


static void
prepare_actions (void)
{
  /* Figure out the actions for the specified state, indexed by
     lookahead token type.  */

  muscle_insert_rule_number_table ("defact", yydefact,
                                   yydefact[0], 1, nstates);

  /* Figure out what to do after reducing with each rule, depending on
     the saved state from before the beginning of parsing the data
     that matched this rule.  */
  muscle_insert_state_number_table ("defgoto", yydefgoto,
                                    yydefgoto[0], 1, nsyms - ntokens);


  /* Output PACT. */
  muscle_insert_base_table ("pact", base,
                             base[0], 1, nstates);
  MUSCLE_INSERT_INT ("pact_ninf", base_ninf);

  /* Output PGOTO. */
  muscle_insert_base_table ("pgoto", base,
                             base[nstates], nstates + 1, nvectors);

  muscle_insert_base_table ("table", table,
                            table[0], 1, high + 1);
  MUSCLE_INSERT_INT ("table_ninf", table_ninf);

  muscle_insert_base_table ("check", check,
                            check[0], 1, high + 1);

  /* GLR parsing slightly modifies YYTABLE and YYCHECK (and thus
     YYPACT) so that in states with unresolved conflicts, the default
     reduction is not used in the conflicted entries, so that there is
     a place to put a conflict pointer.

     This means that YYCONFLP and YYCONFL are nonsense for a non-GLR
     parser, so we could avoid accidents by not writing them out in
     that case.  Nevertheless, it seems even better to be able to use
     the GLR skeletons even without the non-deterministic tables.  */
  muscle_insert_unsigned_int_table ("conflict_list_heads", conflict_table,
                                    conflict_table[0], 1, high + 1);
  muscle_insert_unsigned_int_table ("conflicting_rules", conflict_list,
                                    0, 1, conflict_list_cnt);
}


/*--------------------------------------------.
| Output the definitions of all the muscles.  |
`--------------------------------------------*/

static void
muscles_output (FILE *out)
{
  fputs ("m4_init()\n", out);
  merger_output (out);
  symbol_numbers_output (out);
  type_names_output (out);
  user_actions_output (out);
  /* Must be last.  */
  muscles_m4_output (out);
}

/*---------------------------.
| Call the skeleton parser.  |
`---------------------------*/
#include <process.h>
#include <io.h>
static const char letters[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
   does not exist at the time of the call to mkstemp.  TMPL is
   overwritten with the result.  */
static FILE* mkstempFILE (char *tmpl, const char *mode)
{
	int len;
	char *XXXXXX;
	static unsigned long long value;
	unsigned long long random_time_bits;
	unsigned int count;
	FILE* fd = NULL;
	int r;

	/* A lower bound on the number of temporary files to attempt to
	generate.  The maximum total number of temporary file names that
	can exist for a given template is 62**6.  It should never be
	necessary to try all these combinations.  Instead if a reasonable
	number of names is tried (we define reasonable as 62**3) fail to
	give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

	/* The number of times to attempt to generate a temporary file.  To
	conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
	unsigned int attempts = TMP_MAX;
#else
	unsigned int attempts = ATTEMPTS_MIN;
#endif

	len = strlen (tmpl);
	if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
	{
		return NULL;
	}

	/* This is where the Xs start.  */
	XXXXXX = &tmpl[len - 6];

	/* Get some more or less random data but unique per process */
	{
		static unsigned long long g_value;
		g_value = _getpid();
 		g_value += 100;
		random_time_bits = (((unsigned long long)234546 << 32)
			| (unsigned long long)g_value);
	}
	value += random_time_bits ^ (unsigned long long)122434;

	for (count = 0; count < attempts; value += 7777, ++count)
	{
		unsigned long long v = value;

		/* Fill in the random bits.  */
		XXXXXX[0] = letters[v % 62];
		v /= 62;
		XXXXXX[1] = letters[v % 62];
		v /= 62;
		XXXXXX[2] = letters[v % 62];
		v /= 62;
		XXXXXX[3] = letters[v % 62];
		v /= 62;
		XXXXXX[4] = letters[v % 62];
		v /= 62;
		XXXXXX[5] = letters[v % 62];

		/* file doesn't exist */
		if (r = _access(tmpl, 0) == -1)
		{
			fd = fopen (tmpl, mode);
			if (fd)
			{
				return fd;
			}
		}
	}

	/* We got out of the loop because we ran out of combinations to try.  */
	return NULL;
}

extern int
main_m4 (int argc, char *const *argv, FILE* in, FILE* out);

static void
output_skeleton (void)
{
  FILE *m4_in = NULL;
  FILE *m4_out = NULL;
  char m4_in_file_name[] = "~m4_in_temp_file_XXXXXX";
  char m4_out_file_name[] = "~m4_out_temp_file_XXXXXX";
//  FILE *in;
//  int filter_fd[2];
  char const *argv[10];
//  pid_t pid;

  /* Compute the names of the package data dir and skeleton files.  */
  char const m4sugar[] = "m4sugar/m4sugar.m4";
  char const m4bison[] = "bison.m4";
  char *full_m4sugar;
  char *full_m4bison;
  char *full_skeleton;
  char const *p;
  char const *m4 = (p = getenv ("M4")) ? p : "M4";
  int i = 0;
  char const *pkgdatadir = compute_pkgdatadir ();
  size_t skeleton_size = strlen (skeleton) + 1;
  size_t pkgdatadirlen = strlen (pkgdatadir);
  while (pkgdatadirlen && pkgdatadir[pkgdatadirlen - 1] == '/')
    pkgdatadirlen--;
  full_skeleton = xmalloc (pkgdatadirlen + 1
			   + (skeleton_size < sizeof m4sugar
			      ? sizeof m4sugar : skeleton_size));
  strncpy (full_skeleton, pkgdatadir, pkgdatadirlen);
  full_skeleton[pkgdatadirlen] = '/';
  strcpy (full_skeleton + pkgdatadirlen + 1, m4sugar);
  full_m4sugar = xstrdup (full_skeleton);
  strcpy (full_skeleton + pkgdatadirlen + 1, m4bison);
  full_m4bison = xstrdup (full_skeleton);
  if (_mbschr (skeleton, '/'))
    strcpy (full_skeleton, skeleton);
  else
    strcpy (full_skeleton + pkgdatadirlen + 1, skeleton);

  /* Test whether m4sugar.m4 is readable, to check for proper
     installation.  A faulty installation can cause deadlock, so a
     cheap sanity check is worthwhile.  */
  xfclose (xfopen (full_m4sugar, "r"));

  /* Create an m4 subprocess connected to us via two pipes.  */

  if (trace_flag & trace_tools)
    fprintf (stderr, "running: %s %s - %s %s\n",
             m4, full_m4sugar, full_m4bison, full_skeleton);

  /* Some future version of GNU M4 (most likely 1.6) may treat the -dV in a
     position-dependent manner.  Keep it as the first argument so that all
     files are traced.

     See the thread starting at
     <http://lists.gnu.org/archive/html/bug-bison/2008-07/msg00000.html>
     for details.  */
  {
    argv[i++] = m4;

    /* When POSIXLY_CORRECT is set, GNU M4 1.6 and later disable GNU
       extensions, which Bison's skeletons depend on.  With older M4,
       it has no effect.  M4 1.4.12 added a -g/--gnu command-line
       option to make it explicit that a program wants GNU M4
       extensions even when POSIXLY_CORRECT is set.

       See the thread starting at
       <http://lists.gnu.org/archive/html/bug-bison/2008-07/msg00000.html>
       for details.  */
 //   if (*M4_GNU_OPTION)
 //     argv[i++] = M4_GNU_OPTION;

    argv[i++] = "-I";
    argv[i++] = pkgdatadir;
    if (trace_flag & trace_m4)
      argv[i++] = "-dV";
    argv[i++] = full_m4sugar;
    argv[i++] = "-";
    argv[i++] = full_m4bison;
    argv[i++] = full_skeleton;
    argv[i++] = NULL;
    aver (i <= ARRAY_CARDINALITY (argv));

    /* The ugly cast is because gnulib gets the const-ness wrong.  */
   // pid = create_pipe_bidi ("m4", m4, (char **)(void*)argv, false, true,
   //                         true, filter_fd);
  }


  if (trace_flag & trace_muscles)
    muscles_output (stderr);
  {
    m4_in = mkstempFILE(m4_in_file_name, "w+");
    if (!m4_in)
      error (EXIT_FAILURE, get_errno (),
             "fopen");
    muscles_output (m4_in);

	fflush(m4_in);
	if (fseek(m4_in, 0, SEEK_SET))
      error (EXIT_FAILURE, get_errno (),
             "fseek");
  }

  /* Read and process m4's output.  */
  {
    m4_out = mkstempFILE(m4_out_file_name, "w+");
    if (!m4_out)
      error (EXIT_FAILURE, get_errno (),
             "fopen");
  }

  if (main_m4(i-1, argv, m4_in, m4_out))
      error (EXIT_FAILURE, get_errno (),
             "m4 failed");


  free (full_m4sugar);
  free (full_m4bison);
  free (full_skeleton);

  fflush(m4_out);
  if (fseek(m4_out, 0, SEEK_SET))
    error (EXIT_FAILURE, get_errno (),
      "fseek");

  timevar_push (TV_M4);
//  in = fdopen (filter_fd[0], "r");
//  if (! in)
//    error (EXIT_FAILURE, get_errno (),
//	   "fdopen");
  scan_skel (m4_out);
    /* scan_skel should have read all of M4's output.  Otherwise, when we
       close the pipe, we risk letting M4 report a broken-pipe to the
       Bison user.  */
  aver (feof (m4_out));
  xfclose (m4_in);
  xfclose (m4_out);

  _unlink (m4_in_file_name);
  _unlink (m4_out_file_name);
//  wait_subprocess (pid, "m4", false, false, true, true, NULL);
  timevar_pop (TV_M4);
}

static void
prepare (void)
{
  /* BISON_USE_PUSH_FOR_PULL is for the test suite and should not be
     documented for the user.  */
  char const *cp = getenv ("BISON_USE_PUSH_FOR_PULL");
  bool use_push_for_pull_flag = cp && *cp && strtol (cp, 0, 10);

  /* Flags. */
  MUSCLE_INSERT_BOOL ("defines_flag", defines_flag);
  MUSCLE_INSERT_BOOL ("glr_flag", glr_parser);
  MUSCLE_INSERT_BOOL ("nondeterministic_flag", nondeterministic_parser);
  MUSCLE_INSERT_BOOL ("synclines_flag", !no_lines_flag);
  MUSCLE_INSERT_BOOL ("tag_seen_flag", tag_seen);
  MUSCLE_INSERT_BOOL ("token_table_flag", token_table_flag);
  MUSCLE_INSERT_BOOL ("use_push_for_pull_flag", use_push_for_pull_flag);
  MUSCLE_INSERT_BOOL ("yacc_flag", yacc_flag);

  /* File names.  */
  if (spec_name_prefix)
    MUSCLE_INSERT_STRING ("prefix", spec_name_prefix);

  MUSCLE_INSERT_STRING ("file_name_all_but_ext", all_but_ext);

#define DEFINE(Name) MUSCLE_INSERT_STRING (#Name, Name ? Name : "")
  DEFINE (dir_prefix);
  DEFINE (parser_file_name);
  DEFINE (spec_defines_file);
  DEFINE (spec_file_prefix);
  DEFINE (spec_graph_file);
  DEFINE (spec_name_prefix);
  DEFINE (spec_outfile);
  DEFINE (spec_verbose_file);
#undef DEFINE

  /* Find the right skeleton file, and add muscles about the skeletons.  */
  if (skeleton)
    MUSCLE_INSERT_C_STRING ("skeleton", skeleton);
  else
    skeleton = language->skeleton;

  /* About the skeletons.  */
  {
    /* b4_pkgdatadir is used inside m4_include in the skeletons, so digraphs
       would never be expanded.  Hopefully no one has M4-special characters in
       his Bison installation path.  */
    MUSCLE_INSERT_STRING_RAW ("pkgdatadir", compute_pkgdatadir ());
  }
}


/*----------------------------------------------------------.
| Output the parsing tables and the parser code to ftable.  |
`----------------------------------------------------------*/

void
output (void)
{
  obstack_init (&format_obstack);

  prepare_symbols ();
  prepare_rules ();
  prepare_states ();
  prepare_actions ();
  prepare_symbol_definitions ();

  prepare ();

  /* Process the selected skeleton file.  */
  output_skeleton ();

  obstack_free (&format_obstack, NULL);
}

extern char* local_pkgdatadir;

char const *
compute_pkgdatadir (void)
{
  char const *pkgdatadir = getenv ("BISON_PKGDATADIR");
  return pkgdatadir ? pkgdatadir : local_pkgdatadir;
}
