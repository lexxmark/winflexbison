/* Top level entry point of Bison.

   Copyright (C) 1984, 1986, 1989, 1992, 1995, 2000-2002, 2004-2011 Free
   Software Foundation, Inc.

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

#include <bitset_stats.h>
#include <bitset.h>
//#include <configmake.h>
#include <quotearg.h>
#include <timevar.h>

#include "LR0.h"
#include "complain.h"
#include "conflicts.h"
#include "derives.h"
#include "files.h"
#include "getargs.h"
#include "gram.h"
#include "lalr.h"
#include "ielr.h"
#include "muscle-tab.h"
#include "nullable.h"
#include "output.h"
#include "print.h"
#include "print_graph.h"
#include "print-xml.h"
#include "reader.h"
#include "reduce.h"
#include "scan-code.h"
#include "scan-gram.h"
#include "scan-skel.h"
#include "symtab.h"
#include "tables.h"
#include "uniqstr.h"

/* extracts basename from path, optionally stripping the extension "\.*"
 * (same concept as /bin/sh `basename`, but different handling of extension). */
static char *basename2 (path, strip_ext)
     char   *path;
     int strip_ext;		/* boolean */
{
	char   *b, *e = 0;

	b = path;
	for (b = path; *path; path++)
		if (*path == '/')
			b = path + 1;
		else if (*path == '\\')
			b = path + 1;
		else if (*path == '.')
			e = path;

	if (strip_ext && e && e > b)
		*e = '\0';
	return b;
}

int main (int argc, char *argv[])
{
  program_name = basename2 (argv[0], 1);

  setlocale (LC_ALL, "");
  (void) bindtextdomain (PACKAGE, LOCALEDIR);
  (void) bindtextdomain ("bison-runtime", LOCALEDIR);
  (void) textdomain (PACKAGE);

  uniqstrs_new ();
  muscle_init ();

  getargs (argc, argv);

  timevar_report = trace_flag & trace_time;
  init_timevar ();
  timevar_start (TV_TOTAL);

  if (trace_flag & trace_bitsets)
    bitset_stats_enable ();

  /* Read the input.  Copy some parts of it to FGUARD, FACTION, FTABLE
     and FATTRS.  In file reader.c.  The other parts are recorded in
     the grammar; see gram.h.  */

  timevar_push (TV_READER);
  reader ();
  timevar_pop (TV_READER);

  if (complaint_issued)
    goto finish;

  /* Find useless nonterminals and productions and reduce the grammar. */
  timevar_push (TV_REDUCE);
  reduce_grammar ();
  timevar_pop (TV_REDUCE);

  /* Record other info about the grammar.  In files derives and
     nullable.  */
  timevar_push (TV_SETS);
  derives_compute ();
  nullable_compute ();
  timevar_pop (TV_SETS);

  /* Compute LR(0) parser states.  See state.h for more info.  */
  timevar_push (TV_LR0);
  generate_states ();
  timevar_pop (TV_LR0);

  /* Add lookahead sets to parser states.  Except when LALR(1) is
     requested, split states to eliminate LR(1)-relative
     inadequacies.  */
  ielr ();

  /* Find and record any conflicts: places where one token of
     lookahead is not enough to disambiguate the parsing.  In file
     conflicts.  Also resolve s/r conflicts based on precedence
     declarations.  */
  timevar_push (TV_CONFLICTS);
  conflicts_solve ();
  if (!muscle_percent_define_flag_if ("lr.keep-unreachable-states"))
    {
      state_number *old_to_new = xnmalloc (nstates, sizeof *old_to_new);
      state_number nstates_old = nstates;
      state_remove_unreachable_states (old_to_new);
      lalr_update_state_numbers (old_to_new, nstates_old);
      conflicts_update_state_numbers (old_to_new, nstates_old);
      free (old_to_new);
    }
  conflicts_print ();
  timevar_pop (TV_CONFLICTS);

  /* Compute the parser tables.  */
  timevar_push (TV_ACTIONS);
  tables_generate ();
  timevar_pop (TV_ACTIONS);

  grammar_rules_useless_report
    (_("rule useless in parser due to conflicts"));

  /* Output file names. */
  compute_output_file_names ();

  /* Output the detailed report on the grammar.  */
  if (report_flag)
    {
      timevar_push (TV_REPORT);
      print_results ();
      timevar_pop (TV_REPORT);
    }

  /* Output the graph.  */
  if (graph_flag)
    {
      timevar_push (TV_GRAPH);
      print_graph ();
      timevar_pop (TV_GRAPH);
    }

  /* Output xml.  */
  if (xml_flag)
    {
      timevar_push (TV_XML);
      print_xml ();
      timevar_pop (TV_XML);
    }

  /* Stop if there were errors, to avoid trashing previous output
     files.  */
  if (complaint_issued)
    goto finish;

  /* Lookahead tokens are no longer needed. */
  timevar_push (TV_FREE);
  lalr_free ();
  timevar_pop (TV_FREE);

  /* Output the tables and the parser to ftable.  In file output.  */
  timevar_push (TV_PARSER);
  output ();
  timevar_pop (TV_PARSER);

  timevar_push (TV_FREE);
  nullable_free ();
  derives_free ();
  tables_free ();
  states_free ();
  reduce_free ();
  conflicts_free ();
  grammar_free ();
  output_file_names_free ();

  /* The scanner memory cannot be released right after parsing, as it
     contains things such as user actions, prologue, epilogue etc.  */
  gram_scanner_free ();
  muscle_free ();
  uniqstrs_free ();
  code_scanner_free ();
  skel_scanner_free ();
  quotearg_free ();
  timevar_pop (TV_FREE);

  if (trace_flag & trace_bitsets)
    bitset_stats_dump (stderr);

 finish:

  /* Stop timing and print the times.  */
  timevar_stop (TV_TOTAL);
  timevar_print (stderr);

  return complaint_issued ? EXIT_FAILURE : EXIT_SUCCESS;
}
