/* GNU m4 -- A simple macro processor
Copyright (C) 1989, 90, 91, 92, 93, 94 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "m4.h"

#include "getopt.h"
#include <signal.h>

static void usage _((int));

/* Operate interactively (-e).  */
static int interactive = 0;

/* Enable sync output for /lib/cpp (-s).  */
int sync_output = 0;

/* Debug (-d[flags]).  */
int debug_level = 0;

/* Hash table size (should be a prime) (-Hsize).  */
int hash_table_size = HASHMAX;

/* Disable GNU extensions (-G).  */
int no_gnu_extensions = 0;

/* Prefix all builtin functions by `m4_'.  */
int prefix_all_builtins = 0;

/* Max length of arguments in trace output (-lsize).  */
int max_debug_argument_length = 0;

/* Suppress warnings about missing arguments.  */
int suppress_warnings = 0;

/* If not zero, then value of exit status for warning diagnostics.  */
int warning_status = 0;

/* Artificial limit for expansion_level in macro.c.  */
int nesting_limit = 250;

#ifdef ENABLE_CHANGEWORD
/* User provided regexp for describing m4 words.  */
const char *user_word_regexp = NULL;
#endif

/* Name of frozen file to digest after initialization.  */
const char *frozen_file_to_read = NULL;

/* Name of frozen file to produce near completion.  */
const char *frozen_file_to_write = NULL;

/* The name this program was run with. */
//const char *program_name;

/* If non-zero, display usage information and exit.  */
static int show_help = 0;

/* If non-zero, print the version on standard output and exit.  */
static int show_version = 0;

struct macro_definition
{
	struct macro_definition *next;
	int code;			/* D, U or t */
	const char *macro;
};
typedef struct macro_definition macro_definition;

/* Error handling functions.  */

/*-------------------------------------------------------------------------.
| Print source and line reference on standard error, as a prefix for error |
| messages.  Flush standard output first.				   |
`-------------------------------------------------------------------------*/

void
	reference_error (void)
{
	fflush (stdout);
	fprintf (stderr, "%s:%d: ", current_file, current_line);
}

#ifdef USE_STACKOVF

/*---------------------------------------.
| Tell user stack overflowed and abort.	 |
`---------------------------------------*/

static void
	stackovf_handler (void)
{
	M4ERROR ((EXIT_FAILURE, 0,
		"ERROR: Stack overflow.  (Infinite define recursion?)"));
}

#endif /* USE_STACKOV */

/* Memory allocation.  */

/*------------------------.
| Failsafe free routine.  |
`------------------------*/
void xfree (voidstar p)
{
	if (p != NULL)
		free (p);
}


/*---------------------------------------------.
| Print a usage message and exit with STATUS.  |
`---------------------------------------------*/

static void
	usage (int status)
{
	if (status != EXIT_SUCCESS)
		;//fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
	else
	{
//		printf ("Usage: %s [OPTION]... [FILE]...\n", program_name);
		fputs ("\
			   Mandatory or optional arguments to long options are mandatory or optional\n\
			   for short options too.\n\
			   \n\
			   Operation modes:\n\
			   --help                   display this help and exit\n\
			   --version                output version information and exit\n\
			   -e, --interactive            unbuffer output, ignore interrupts\n\
			   -E, --fatal-warnings         stop execution after first warning\n\
			   -Q, --quiet, --silent        suppress some warnings for builtins\n\
			   -P, --prefix-builtins        force a `m4_' prefix to all builtins\n",
			   stdout);
#ifdef ENABLE_CHANGEWORD
		fputs ("\
			   -W, --word-regexp=REGEXP     use REGEXP for macro name syntax\n",
			   stdout);
#endif
		fputs ("\
			   \n\
			   Preprocessor features:\n\
			   -I, --include=DIRECTORY      search this directory second for includes\n\
			   -D, --define=NAME[=VALUE]    enter NAME has having VALUE, or empty\n\
			   -U, --undefine=NAME          delete builtin NAME\n\
			   -s, --synclines              generate `#line NO \"FILE\"' lines\n",
			   stdout);
		fputs ("\
			   \n\
			   Limits control:\n\
			   -G, --traditional            suppress all GNU extensions\n\
			   -H, --hashsize=PRIME         set symbol lookup hash table size\n\
			   -L, --nesting-limit=NUMBER   change artificial nesting limit\n",
			   stdout);
		fputs ("\
			   \n\
			   Frozen state files:\n\
			   -F, --freeze-state=FILE      produce a frozen state on FILE at end\n\
			   -R, --reload-state=FILE      reload a frozen state from FILE at start\n",
			   stdout);
		fputs ("\
			   \n\
			   Debugging:\n\
			   -d, --debug=[FLAGS]          set debug level (no FLAGS implies `aeq')\n\
			   -t, --trace=NAME             trace NAME when it will be defined\n\
			   -l, --arglength=NUM          restrict macro tracing size\n\
			   -o, --error-output=FILE      redirect debug and trace output\n",
			   stdout);
		fputs ("\
			   \n\
			   FLAGS is any of:\n\
			   t   trace for all macro calls, not only traceon'ed\n\
			   a   show actual arguments\n\
			   e   show expansion\n\
			   q   quote values as necessary, with a or e flag\n\
			   c   show before collect, after collect and after call\n\
			   x   add a unique macro call id, useful with c flag\n\
			   f   say current input file name\n\
			   l   say current input line number\n\
			   p   show results of path searches\n\
			   i   show changes in input files\n\
			   V   shorthand for all of the above flags\n",
			   stdout);
		fputs ("\
			   \n\
			   If no FILE or if FILE is `-', standard input is read.\n",
			   stdout);
	}
	exit (status);
}

/*--------------------------------------.
| Decode options and launch execution.  |
`--------------------------------------*/

static const struct option long_options[] =
{
	{"arglength", required_argument, NULL, 'l'},
	{"debug", optional_argument, NULL, 'd'},
	{"diversions", required_argument, NULL, 'N'},
	{"error-output", required_argument, NULL, 'o'},
	{"fatal-warnings", no_argument, NULL, 'E'},
	{"freeze-state", required_argument, NULL, 'F'},
	{"hashsize", required_argument, NULL, 'H'},
	{"include", required_argument, NULL, 'I'},
	{"interactive", no_argument, NULL, 'e'},
	{"nesting-limit", required_argument, NULL, 'L'},
	{"prefix-builtins", no_argument, NULL, 'P'},
	{"quiet", no_argument, NULL, 'Q'},
	{"reload-state", required_argument, NULL, 'R'},
	{"silent", no_argument, NULL, 'Q'},
	{"synclines", no_argument, NULL, 's'},
	{"traditional", no_argument, NULL, 'G'},
	{"word-regexp", required_argument, NULL, 'W'},

	{"help", no_argument, &show_help, 1},
	{"version", no_argument, &show_version, 1},

	/* These are somewhat troublesome.  */
	{ "define", required_argument, NULL, 'D' },
	{ "undefine", required_argument, NULL, 'U' },
	{ "trace", required_argument, NULL, 't' },

	{ 0, 0, 0, 0 },
};

#ifdef ENABLE_CHANGEWORD
#define OPTSTRING "B:D:EF:GH:I:L:N:PQR:S:T:U:W:d::el:o:st:"
#else
#define OPTSTRING "B:D:EF:GH:I:L:N:PQR:S:T:U:d::el:o:st:"
#endif

int main_m4_p(FILE* in, FILE* out)
	/*
	int
	main (int argc, char *const *argv, char *const *envp)
	*/
{
	macro_definition *head;	/* head of deferred argument list */
	macro_definition *tail;
//	macro_definition *new;
//	int optchar;			/* option character */

	macro_definition *defines;
//	FILE *fp;

//	program_name = "m4";/*argv[0];*/

	include_init ();
	debug_init ();
#ifdef USE_STACKOVF
	setup_stackovf_trap (argv, envp, stackovf_handler);
#endif

	/* First, we decode the arguments, to size up tables and stuff.  */

	head = tail = NULL;

#if 0
	while (optchar = getopt_long (argc, argv, OPTSTRING, long_options, NULL),
		optchar != EOF)
		switch (optchar)
	{
		default:
			usage (EXIT_FAILURE);

		case 0:
			break;

		case 'B':			/* compatibility junk */
		case 'N':
		case 'S':
		case 'T':
			break;

		case 'D':
		case 'U':
		case 't':

			/* Arguments that cannot be handled until later are accumulated.  */

			new = (macro_definition *) xmalloc (sizeof (macro_definition));
			new->code = optchar;
			new->macro = optarg;
			new->next = NULL;

			if (head == NULL)
				head = new;
			else
				tail->next = new;
			tail = new;

			break;

		case 'E':
			warning_status = EXIT_FAILURE;
			break;

		case 'F':
			frozen_file_to_write = optarg;
			break;

		case 'G':
			no_gnu_extensions = 1;
			break;

		case 'H':
			hash_table_size = atoi (optarg);
			if (hash_table_size <= 0)
				hash_table_size = HASHMAX;
			break;

		case 'I':
			add_include_directory (optarg);
			break;

		case 'L':
			nesting_limit = atoi (optarg);
			break;

		case 'P':
			prefix_all_builtins = 1;
			break;

		case 'Q':
			suppress_warnings = 1;
			break;

		case 'R':
			frozen_file_to_read = optarg;
			break;

#ifdef ENABLE_CHANGEWORD
		case 'W':
			user_word_regexp = optarg;
			break;
#endif

		case 'd':
			debug_level = debug_decode (optarg);
			if (debug_level < 0)
			{
				error (0, 0, "bad debug flags: `%s'", optarg);
				debug_level = 0;
			}
			break;

		case 'e':
			interactive = 1;
			break;

		case 'l':
			max_debug_argument_length = atoi (optarg);
			if (max_debug_argument_length <= 0)
				max_debug_argument_length = 0;
			break;

		case 'o':
			if (!debug_set_output (optarg))
				error (0, errno, "%s", optarg);
			break;

		case 's':
			sync_output = 1;
			break;
	}
#endif

	/* -P option */
	prefix_all_builtins = 1;

	if (show_version)
	{
		//      printf ("GNU %s %s\n", PRODUCT, VERSION);
		exit (EXIT_SUCCESS);
	}

	if (show_help)
		usage (EXIT_SUCCESS);

	defines = head;

	/* Do the basic initialisations.  */

	input_init ();
	output_init (out);
	symtab_init ();
	include_env_init ();

	if (frozen_file_to_read)
		reload_frozen_state (frozen_file_to_read);
	else
		builtin_init ();

	/* Handle deferred command line macro definitions.  Must come after
	initialisation of the symbol table.  */

	while (defines != NULL)
	{
		macro_definition *next;
		char *macro_value;
		symbol *sym;

		switch (defines->code)
		{
		case 'D':
			macro_value = strchr (defines->macro, '=');
			if (macro_value == NULL)
				macro_value = "";
			else
				*macro_value++ = '\0';
			define_user_macro (defines->macro, macro_value, SYMBOL_INSERT);
			break;

		case 'U':
			lookup_symbol (defines->macro, SYMBOL_DELETE);
			break;

		case 't':
			sym = lookup_symbol (defines->macro, SYMBOL_INSERT);
			SYMBOL_TRACED (sym) = TRUE;
			break;

		default:
			M4ERROR ((warning_status, 0,
				"INTERNAL ERROR: Bad code in deferred arguments"));
			abort ();
		}

		next = defines->next;
		xfree ((voidstar) defines);
		defines = next;
	}

	/* Interactive mode means unbuffered output, and interrupts ignored.  */
#if 0
	if (interactive)
	{
		signal (SIGINT, SIG_IGN);
		setbuf (stdout, (char *) NULL);
	}

	/* Handle the various input files.  Each file is pushed on the input,
	and the input read.  Wrapup text is handled separately later.  */

	if (optind == argc)
	{
		push_file (stdin, "stdin");
		expand_input ();
	}
	else
		for (; optind < argc; optind++)
		{
			if (strcmp (argv[optind], "-") == 0)
				push_file (stdin, "stdin");
			else
			{
				fp = path_search (argv[optind]);
				if (fp == NULL)
				{
					error (0, errno, "%s", argv[optind]);
					continue;
				}
				else
					push_file (fp, argv[optind]);
			}
			expand_input ();
		}
#undef NEXTARG
#endif

	push_file (in, "stdin");
	expand_input ();
	
	/* Now handle wrapup text.  */

	while (pop_wrapup ())
		expand_input ();

//	frozen_file_to_write = "m4.out";

	if (frozen_file_to_write)
		produce_frozen_state (frozen_file_to_write);
	else
	{
		make_diversion (0);
		undivert_all ();
	}

//	exit (EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
