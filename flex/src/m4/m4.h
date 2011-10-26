/* GNU m4 -- A simple macro processor

   Copyright (C) 1989, 1990, 1991, 1992, 1993, 1994, 2004 Free
   Software Foundation, Inc.

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

/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because it found this file in $srcdir).  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>

#ifdef __STDC__
# define voidstar void *
#else
# define voidstar char *
#endif

#ifdef PROTOTYPES
# define _(Args) Args
#else
# define _(Args) ()
#endif

#include <stdio.h>
#include <ctype.h>

#include "obstack.h"

/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
#define STDC_HEADERS
#if defined (HAVE_STRING_H) || defined (STDC_HEADERS)
# include <string.h>
# if !defined (STDC_HEADERS) && defined (HAVE_MEMORY_H)
#  include <memory.h>
# endif
/* This is for obstack code -- should live in obstack.h.  */
# ifndef bcopy
#  define bcopy(S, D, N) memcpy ((D), (S), (N))
# endif
#else
# include <strings.h>
# ifndef memcpy
#  define memcpy(D, S, N) bcopy((S), (D), (N))
# endif
# ifndef strchr
#  define strchr(S, C) index ((S), (C))
# endif
# ifndef strrchr
#  define strrchr(S, C) rindex ((S), (C))
# endif
# ifndef bcopy
void bcopy ();
# endif
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
#else /* not STDC_HEADERS */

voidstar malloc ();
voidstar realloc ();
char *getenv ();
double atof ();
long strtol ();

#endif /* STDC_HEADERS */

/* Some systems do not define EXIT_*, even with STDC_HEADERS.  */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

/* If FALSE is defined, we presume TRUE is defined too.  In this case,
   merely typedef boolean as being int.  Or else, define these all.  */
#ifndef FALSE
/* Do not use `enum boolean': this tag is used in SVR4 <sys/types.h>.  */
typedef enum { FALSE = 0, TRUE = 1 } boolean;
#else
typedef int boolean;
#endif

#include <io.h>
#define __STDC__ 0
//char *mktemp ();

/* Various declarations.  */

struct _string
  {
    char *string;		/* characters of the string */
    size_t length;		/* length of the string */
  };
typedef struct _string STRING;

/* Memory allocation.  */
voidstar xmalloc _((unsigned int));
voidstar xrealloc _((voidstar , unsigned int));
void xfree _((voidstar));
char *xstrdup _((const char *));
#define obstack_chunk_alloc	xmalloc
#define obstack_chunk_free	xfree

/* Other library routines.  */
void error _((int, int, const char *, ...));

/* Those must come first.  */
typedef void builtin_func ();
typedef struct token_data token_data;

/* File: m4.c  --- global definitions.  */

/* Option flags.  */
extern int sync_output;			/* -s */
extern int debug_level;			/* -d */
extern int hash_table_size;		/* -H */
extern int no_gnu_extensions;		/* -G */
extern int prefix_all_builtins;		/* -P */
extern int max_debug_argument_length;	/* -l */
extern int suppress_warnings;		/* -Q */
extern int warning_status;		/* -E */
extern int nesting_limit;		/* -L */
#ifdef ENABLE_CHANGEWORD
extern const char *user_word_regexp;	/* -W */
#endif

/* Error handling.  */
#define M4ERROR(Arglist) \
  (reference_error (), error Arglist)

void reference_error _((void));

#ifdef USE_STACKOVF
void setup_stackovf_trap _((char *const *, char *const *,
			    void (*handler) (void)));
#endif

/* File: debug.c  --- debugging and tracing function.  */

extern FILE *debug;

/* The value of debug_level is a bitmask of the following.  */

/* a: show arglist in trace output */
#define DEBUG_TRACE_ARGS 1
/* e: show expansion in trace output */
#define DEBUG_TRACE_EXPANSION 2
/* q: quote args and expansion in trace output */
#define DEBUG_TRACE_QUOTE 4
/* t: trace all macros -- overrides trace{on,off} */
#define DEBUG_TRACE_ALL 8
/* l: add line numbers to trace output */
#define DEBUG_TRACE_LINE 16
/* f: add file name to trace output */
#define DEBUG_TRACE_FILE 32
/* p: trace path search of include files */
#define DEBUG_TRACE_PATH 64
/* c: show macro call before args collection */
#define DEBUG_TRACE_CALL 128
/* i: trace changes of input files */
#define DEBUG_TRACE_INPUT 256
/* x: add call id to trace output */
#define DEBUG_TRACE_CALLID 512

/* V: very verbose --  print everything */
#define DEBUG_TRACE_VERBOSE 1023
/* default flags -- equiv: aeq */
#define DEBUG_TRACE_DEFAULT 7

#define DEBUG_PRINT1(Fmt, Arg1) \
  do								\
    {								\
      if (debug != NULL)					\
	fprintf (debug, Fmt, Arg1);				\
    }								\
  while (0)

#define DEBUG_PRINT3(Fmt, Arg1, Arg2, Arg3) \
  do								\
    {								\
      if (debug != NULL)					\
	fprintf (debug, Fmt, Arg1, Arg2, Arg3);			\
    }								\
  while (0)

#define DEBUG_MESSAGE(Fmt) \
  do								\
    {								\
      if (debug != NULL)					\
	{							\
	  debug_message_prefix ();				\
	  fprintf (debug, Fmt);					\
	  putc ('\n', debug);					\
	}							\
    }								\
  while (0)

#define DEBUG_MESSAGE1(Fmt, Arg1) \
  do								\
    {								\
      if (debug != NULL)					\
	{							\
	  debug_message_prefix ();				\
	  fprintf (debug, Fmt, Arg1);				\
	  putc ('\n', debug);					\
	}							\
    }								\
  while (0)

#define DEBUG_MESSAGE2(Fmt, Arg1, Arg2) \
  do								\
    {								\
      if (debug != NULL)					\
	{							\
	  debug_message_prefix ();				\
	  fprintf (debug, Fmt, Arg1, Arg2);			\
	  putc ('\n', debug);					\
	}							\
    }								\
  while (0)

void debug_init _((void));
int debug_decode _((const char *));
void debug_flush_files _((void));
boolean debug_set_output _((const char *));
void debug_message_prefix _((void));

void trace_prepre _((const char *, int));
void trace_pre _((const char *, int, int, token_data **));
void trace_post _((const char *, int, int, token_data **, const char *));

/* File: input.c  --- lexical definitions.  */

/* Various different token types.  */
enum token_type
{
  TOKEN_EOF,			/* end of file */
  TOKEN_STRING,			/* a quoted string */
  TOKEN_WORD,			/* an identifier */
  TOKEN_SIMPLE,			/* a single character */
  TOKEN_MACDEF			/* a macros definition (see "defn") */
};

/* The data for a token, a macro argument, and a macro definition.  */
enum token_data_type
{
  TOKEN_VOID,
  TOKEN_TEXT,
  TOKEN_FUNC
};

struct token_data
{
  enum token_data_type type;
  union
    {
      struct
	{
	  char *text;
#ifdef ENABLE_CHANGEWORD
	  char *original_text;
#endif
	}
      u_t;
      struct
	{
	  builtin_func *func;
	  boolean traced;
	}
      u_f;
    }
  u;
};

#define TOKEN_DATA_TYPE(Td)		((Td)->type)
#define TOKEN_DATA_TEXT(Td)		((Td)->u.u_t.text)
#ifdef ENABLE_CHANGEWORD
# define TOKEN_DATA_ORIG_TEXT(Td)	((Td)->u.u_t.original_text)
#endif
#define TOKEN_DATA_FUNC(Td)		((Td)->u.u_f.func)
#define TOKEN_DATA_FUNC_TRACED(Td) 	((Td)->u.u_f.traced)

typedef enum token_type token_type;
typedef enum token_data_type token_data_type;

void input_init _((void));
int peek_input _((void));
token_type next_token _((token_data *));
void skip_line _((void));

/* push back input */
void push_file _((FILE *, const char *));
void push_macro _((builtin_func *, boolean));
struct obstack *push_string_init _((void));
const char *push_string_finish _((void));
void push_wrapup _((const char *));
boolean pop_wrapup _((void));

/* current input file, and line */
extern const char *current_file;
extern int current_line;

/* left and right quote, begin and end comment */
extern STRING bcomm, ecomm;
extern STRING lquote, rquote;

#define DEF_LQUOTE "`"
#define DEF_RQUOTE "\'"
#define DEF_BCOMM "#"
#define DEF_ECOMM "\n"

void set_quotes _((const char *, const char *));
void set_comment _((const char *, const char *));
#ifdef ENABLE_CHANGEWORD
void set_word_regexp _((const char *));
#endif

/* File: output.c --- output functions.  */
extern int current_diversion;
extern int output_current_line;

void output_init _((FILE*));
void shipout_text _((struct obstack *, const char *, int));
void make_diversion _((int));
void insert_diversion _((int));
void insert_file _((FILE *));
void freeze_diversions _((FILE *));

/* File symtab.c  --- symbol table definitions.  */

/* Operation modes for lookup_symbol ().  */
enum symbol_lookup
{
  SYMBOL_LOOKUP,
  SYMBOL_INSERT,
  SYMBOL_DELETE,
  SYMBOL_PUSHDEF,
  SYMBOL_POPDEF
};

/* Symbol table entry.  */
struct symbol
{
  struct symbol *next;
  boolean traced;
  boolean shadowed;
  boolean macro_args;
  boolean blind_no_args;

  char *name;
  token_data data;
};

#define SYMBOL_NEXT(S)		((S)->next)
#define SYMBOL_TRACED(S)	((S)->traced)
#define SYMBOL_SHADOWED(S)	((S)->shadowed)
#define SYMBOL_MACRO_ARGS(S)	((S)->macro_args)
#define SYMBOL_BLIND_NO_ARGS(S)	((S)->blind_no_args)
#define SYMBOL_NAME(S)		((S)->name)
#define SYMBOL_TYPE(S)		(TOKEN_DATA_TYPE (&(S)->data))
#define SYMBOL_TEXT(S)		(TOKEN_DATA_TEXT (&(S)->data))
#define SYMBOL_FUNC(S)		(TOKEN_DATA_FUNC (&(S)->data))

typedef enum symbol_lookup symbol_lookup;
typedef struct symbol symbol;
typedef void hack_symbol ();

#define HASHMAX 509		/* default, overridden by -Hsize */

extern symbol **symtab;

void symtab_init _((void));
symbol *lookup_symbol _((const char *, symbol_lookup));
void hack_all_symbols _((hack_symbol *, const char *));

/* File: macro.c  --- macro expansion.  */

void expand_input _((void));
void call_macro _((symbol *, int, token_data **, struct obstack *));

/* File: builtin.c  --- builtins.  */

struct builtin
{
  const char *name;
  boolean gnu_extension;
  boolean groks_macro_args;
  boolean blind_if_no_args;
  builtin_func *func;
};

struct predefined
{
  const char *unix_name;
  const char *gnu_name;
  const char *func;
};

typedef struct builtin builtin;
typedef struct predefined predefined;

void builtin_init _((void));
void define_builtin _((const char *, const builtin *, symbol_lookup, boolean));
void define_user_macro _((const char *, const char *, symbol_lookup));
void undivert_all _((void));
void expand_user_macro _((struct obstack *, symbol *, int, token_data **));

const builtin *find_builtin_by_addr _((builtin_func *));
const builtin *find_builtin_by_name _((const char *));

/* File: path.c  --- path search for include files.  */

void include_init _((void));
void include_env_init _((void));
void add_include_directory _((const char *));
FILE *path_search _((const char *));

/* File: eval.c  --- expression evaluation.  */

/* eval_t and unsigned_eval_t should be at least 32 bits.  */
typedef int eval_t;
typedef unsigned int unsigned_eval_t;

boolean evaluate _((const char *, eval_t *));

/* File: format.c  --- printf like formatting.  */

void format _((struct obstack *, int, token_data **));

/* File: freeze.c --- frozen state files.  */

void produce_frozen_state _((const char *));
void reload_frozen_state _((const char *));

/* Debugging the memory allocator.  */

#ifdef WITH_DMALLOC
# define DMALLOC_FUNC_CHECK
# include <dmalloc.h>
#endif

/* Other debug stuff.  */

#ifdef DEBUG
# define DEBUG_INPUT
# define DEBUG_MACRO
# define DEBUG_SYM
# define DEBUG_INCL
#endif
