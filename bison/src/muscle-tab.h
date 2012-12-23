/* Muscle table manager for Bison,

   Copyright (C) 2001-2003, 2006-2007, 2009-2012 Free Software
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

#ifndef MUSCLE_TAB_H_
# define MUSCLE_TAB_H_

# include <quotearg.h>

# include "location.h"

void muscle_init (void);
void muscle_insert (char const *key, char const *value);
char const *muscle_find_const (char const *key);
char *muscle_find (char const *key);
void muscle_free (void);


/* An obstack dedicated to receive muscle keys and values.  */
extern struct obstack muscle_obstack;

#define MUSCLE_INSERT_BOOL(Key, Value)                          \
  do {                                                          \
    int v = Value;                                              \
    MUSCLE_INSERT_INT (Key, v);                                 \
  } while (0)

#define MUSCLE_INSERT_INT(Key, Value)                           \
  do {                                                          \
    obstack_printf (&muscle_obstack, "%d", Value);              \
    obstack_1grow (&muscle_obstack, 0);                         \
    muscle_insert (Key, obstack_finish (&muscle_obstack));      \
  } while (0)

#define MUSCLE_INSERT_LONG_INT(Key, Value)                      \
  do {                                                          \
    obstack_printf (&muscle_obstack, "%ld", Value);             \
    obstack_1grow (&muscle_obstack, 0);                         \
    muscle_insert (Key, obstack_finish (&muscle_obstack));      \
  } while (0)

#define MUSCLE_INSERT_STRING_RAW(Key, Value)                    \
  do {                                                          \
    obstack_sgrow (&muscle_obstack, Value);                     \
    obstack_1grow (&muscle_obstack, 0);                         \
    muscle_insert (Key, obstack_finish (&muscle_obstack));      \
  } while (0)

#define MUSCLE_INSERT_STRING(Key, Value)                        \
  do {                                                          \
    obstack_escape (&muscle_obstack, Value);                    \
    obstack_1grow (&muscle_obstack, 0);                         \
    muscle_insert (Key, obstack_finish (&muscle_obstack));      \
  } while (0)

#define MUSCLE_INSERT_C_STRING(Key, Value)                      \
  do {                                                          \
    obstack_escape (&muscle_obstack,                            \
                    quotearg_style (c_quoting_style, Value));   \
    obstack_1grow (&muscle_obstack, 0);                         \
    muscle_insert (Key, obstack_finish (&muscle_obstack));      \
  } while (0)

/* Append VALUE to the current value of KEY.  If KEY did not already
   exist, create it.  Use MUSCLE_OBSTACK.  De-allocate the previously
   associated value.  Copy VALUE and SEPARATOR.  */

void muscle_grow (const char *key, const char *value, const char *separator);


/* Append VALUE to the current value of KEY, using muscle_grow.  But
   in addition, issue a synchronization line for the location LOC.  */

void muscle_code_grow (const char *key, const char *value, location loc);


/* MUSCLE is an M4 list of pairs.  Create or extend it with the pair
   (A1, A2) after escaping both values with digraphs.  Note that because the
   muscle values are output *double* quoted, one needs to strip the first level
   of quotes to reach the list itself.  */
void muscle_pair_list_grow (const char *muscle,
                            const char *a1, const char *a2);

/* Grow KEY for the occurrence of the name USER_NAME at LOC appropriately for
   use with b4_check_user_names in ../data/bison.m4.  USER_NAME is not escaped
   with digraphs, so it must not contain `[' or `]'.  */
void muscle_user_name_list_grow (char const *key, char const *user_name,
                                 location loc);

/* Indicates whether a variable's value was specified with -D/--define, with
   -F/--force-define, or in the grammar file.  */
typedef enum {
  MUSCLE_PERCENT_DEFINE_D = 0, MUSCLE_PERCENT_DEFINE_F,
  MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE
} muscle_percent_define_how;

/* Define the muscles for %define variable VARIABLE with VALUE specified
   at VARIABLE_LOC in the manner HOW unless it was specified in the
   grammar file while the previous definition for VARIABLE was specified
   with -F/--force-define.  Complain if a previous definition is being
   overridden and the new definition is specified in the grammar file.
   (These rules support the documented behavior as long as command-line
   definitions are processed before grammar file definitions.)  Record
   this as a user occurrence of VARIABLE by invoking
   muscle_user_name_list_grow.  */
void muscle_percent_define_insert (char const *variable, location variable_loc,
                                   char const *value,
                                   muscle_percent_define_how how);

/* Mimic b4_percent_define_get in ../data/bison.m4 exactly.  That is, if the
   %define variable VARIABLE is defined, return its value.  Otherwise, return
   the empty string.  Also, record Bison's usage of VARIABLE by defining
   b4_percent_define_bison_variables(VARIABLE).  The caller is responsible for
   freeing the memory of the returned string.  */
char *muscle_percent_define_get (char const *variable);

/* Mimic muscle_percent_define_get_loc in ../data/bison.m4 exactly.  That is,
   if the %define variable VARIABLE is undefined, complain fatally since that's
   a Bison error.  Otherwise, return its definition location in a form
   approriate for the first argument of warn_at, complain_at, or fatal_at.
   Don't record this as a Bison usage of VARIABLE as there's no reason to
   suspect that the user-supplied value has yet influenced the output.  */
location muscle_percent_define_get_loc (char const *variable);

/* Mimic muscle_percent_define_get_syncline in ../data/bison.m4 exactly.  That
   is, if the %define variable VARIABLE is undefined, complain fatally since
   that's a Bison error.  Otherwise, return its definition location as a
   b4_syncline invocation.  Don't record this as a Bison usage of VARIABLE as
   there's no reason to suspect that the user-supplied value has yet influenced
   the output.  */
char const *muscle_percent_define_get_syncline (char const *variable);

/* Mimic b4_percent_define_ifdef in ../data/bison.m4 exactly.  That is, if the
   %define variable VARIABLE is defined, return true.  Otherwise, return false.
   Also, record Bison's usage of VARIABLE by defining
   b4_percent_define_bison_variables(VARIABLE).  */
bool muscle_percent_define_ifdef (char const *variable);

/* Mimic b4_percent_define_flag_if in ../data/bison.m4 exactly.  That is, if
   the %define variable VARIABLE is defined to "" or "true", return true.  If
   it is defined to "false", return false.  Complain if it is undefined (a
   Bison error since the default value should have been set already) or defined
   to any other value (possibly a user error).  Also, record Bison's usage of
   VARIABLE by defining b4_percent_define_bison_variables(VARIABLE).  */
bool muscle_percent_define_flag_if (char const *variable);

/* Mimic b4_percent_define_default in ../data/bison.m4 exactly.  That is, if
   the %define variable VARIABLE is undefined, set its value to VALUE.
   Don't record this as a Bison usage of VARIABLE as there's no reason to
   suspect that the value has yet influenced the output.  */
void muscle_percent_define_default (char const *variable, char const *value);

/* Mimic b4_percent_define_check_values in ../data/bison.m4 exactly except that
   the VALUES structure is more appropriate for C.  That is, VALUES points to a
   list of strings that is partitioned into sublists by NULL's, one terminating
   each sublist.  The last sublist is followed by a second NULL.  For each
   sublist, the first string is the name of a %define variable, and all
   remaining strings in that sublist are the valid values for that variable.
   Complain if such a variable is undefined (a Bison error since the default
   value should have been set already) or defined to any other value (possibly
   a user error).  Don't record this as a Bison usage of the variable as
   there's no reason to suspect that the value has yet influenced the
   output.  */
void muscle_percent_define_check_values (char const * const *values);

/* Grow the muscle for the %code qualifier QUALIFIER appearing at
   QUALIFIER_LOC with code CODE appearing at CODE_LOC.  Record this as a
   user occurrence of QUALIFIER by invoking
   muscle_user_name_list_grow.  */
void muscle_percent_code_grow (char const *qualifier, location qualifier_loc,
                               char const *code, location code_loc);

void muscles_m4_output (FILE *out);

#endif /* not MUSCLE_TAB_H_ */
