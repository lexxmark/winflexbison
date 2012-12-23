/* Muscle table manager for Bison.

   Copyright (C) 2001-2012 Free Software Foundation, Inc.

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

#include <hash.h>

#include "complain.h"
#include "files.h"
#include "getargs.h"
#include "muscle-tab.h"
#include "quote.h"

/* A key-value pair, along with storage that can be reclaimed when
   this pair is no longer needed.  */
typedef struct
{
  char const *key;
  char const *value;
  char *storage;
} muscle_entry;

/* An obstack used to create some entries.  */
struct obstack muscle_obstack;

/* Initial capacity of muscles hash table.  */
#define HT_INITIAL_CAPACITY 257

static struct hash_table *muscle_table = NULL;

static bool
hash_compare_muscles (void const *x, void const *y)
{
  muscle_entry const *m1 = x;
  muscle_entry const *m2 = y;
  return strcmp (m1->key, m2->key) == 0;
}

static size_t
hash_muscle (const void *x, size_t tablesize)
{
  muscle_entry const *m = x;
  return hash_string (m->key, tablesize);
}

/*-----------------------------------------------------------------.
| Create the MUSCLE_TABLE, and initialize it with default values.  |
| Also set up the MUSCLE_OBSTACK.                                  |
`-----------------------------------------------------------------*/

static void
muscle_entry_free (void *entry)
{
  muscle_entry *mentry = entry;
  free (mentry->storage);
  free (mentry);
}

void
muscle_init (void)
{
  /* Initialize the muscle obstack.  */
  obstack_init (&muscle_obstack);

  muscle_table = hash_initialize (HT_INITIAL_CAPACITY, NULL, hash_muscle,
				  hash_compare_muscles, muscle_entry_free);

  /* Version and input file.  */
  MUSCLE_INSERT_STRING ("version", VERSION);
}


/*------------------------------------------------------------.
| Free all the memory consumed by the muscle machinery only.  |
`------------------------------------------------------------*/

void
muscle_free (void)
{
  hash_free (muscle_table);
  obstack_free (&muscle_obstack, NULL);
}



/*------------------------------------------------------------.
| Insert (KEY, VALUE).  If KEY already existed, overwrite the |
| previous value.                                             |
`------------------------------------------------------------*/

void
muscle_insert (char const *key, char const *value)
{
  muscle_entry probe;
  muscle_entry *entry;

  probe.key = key;
  entry = hash_lookup (muscle_table, &probe);

  if (!entry)
    {
      /* First insertion in the hash. */
      entry = xmalloc (sizeof *entry);
      entry->key = key;
      if (!hash_insert (muscle_table, entry))
        xalloc_die ();
    }
  else
    free (entry->storage);
  entry->value = value;
  entry->storage = NULL;
}


/*-------------------------------------------------------------------.
| Append VALUE to the current value of KEY.  If KEY did not already  |
| exist, create it.  Use MUSCLE_OBSTACK.  De-allocate the previously |
| associated value.  Copy VALUE and SEPARATOR.                       |
`-------------------------------------------------------------------*/

void
muscle_grow (const char *key, const char *val, const char *separator)
{
  muscle_entry probe;
  muscle_entry *entry = NULL;

  probe.key = key;
  entry = hash_lookup (muscle_table, &probe);

  if (!entry)
    {
      /* First insertion in the hash. */
      entry = xmalloc (sizeof *entry);
      entry->key = key;
      if (!hash_insert (muscle_table, entry))
        xalloc_die ();
      entry->value = entry->storage = xstrdup (val);
    }
  else
    {
      /* Grow the current value. */
      char *new_val;
      obstack_sgrow (&muscle_obstack, entry->value);
      free (entry->storage);
      obstack_sgrow (&muscle_obstack, separator);
      obstack_sgrow (&muscle_obstack, val);
      obstack_1grow (&muscle_obstack, 0);
      new_val = obstack_finish (&muscle_obstack);
      entry->value = entry->storage = xstrdup (new_val);
      obstack_free (&muscle_obstack, new_val);
    }
}

/*------------------------------------------------------------------.
| Using muscle_grow, append a synchronization line for the location |
| LOC to the current value of KEY.                                  |
`------------------------------------------------------------------*/

static void
muscle_syncline_grow (char const *key, location loc)
{
  char *extension = NULL;
  obstack_printf (&muscle_obstack, "]b4_syncline(%d, ", loc.start.line);
  obstack_quote (&muscle_obstack,
                 quotearg_style (c_quoting_style, loc.start.file));
  obstack_sgrow (&muscle_obstack, ")[");
  obstack_1grow (&muscle_obstack, 0);
  extension = obstack_finish (&muscle_obstack);
  muscle_grow (key, extension, "");
  obstack_free (&muscle_obstack, extension);
}

/*------------------------------------------------------------------.
| Append VALUE to the current value of KEY, using muscle_grow.  But |
| in addition, issue a synchronization line for the location LOC    |
| using muscle_syncline_grow.                                       |
`------------------------------------------------------------------*/

void
muscle_code_grow (const char *key, const char *val, location loc)
{
  muscle_syncline_grow (key, loc);
  muscle_grow (key, val, "\n");
}


void muscle_pair_list_grow (const char *muscle,
			    const char *a1, const char *a2)
{
  char *pair;
  obstack_sgrow (&muscle_obstack, "[");
  obstack_quote (&muscle_obstack, a1);
  obstack_sgrow (&muscle_obstack, ", ");
  obstack_quote (&muscle_obstack, a2);
  obstack_sgrow (&muscle_obstack, "]");
  obstack_1grow (&muscle_obstack, 0);
  pair = obstack_finish (&muscle_obstack);
  muscle_grow (muscle, pair, ",\n");
  obstack_free (&muscle_obstack, pair);
}


/*----------------------------------------------------------------------------.
| Find the value of muscle KEY.  Unlike MUSCLE_FIND, this is always reliable  |
| to determine whether KEY has a value.                                       |
`----------------------------------------------------------------------------*/

char const *
muscle_find_const (char const *key)
{
  muscle_entry probe;
  muscle_entry *result = NULL;

  probe.key = key;
  result = hash_lookup (muscle_table, &probe);
  if (result)
    return result->value;
  return NULL;
}


/*----------------------------------------------------------------------------.
| Find the value of muscle KEY.  Abort if muscle_insert was invoked more      |
| recently than muscle_grow for KEY since muscle_find can't return a          |
| char const *.                                                               |
`----------------------------------------------------------------------------*/

char *
muscle_find (char const *key)
{
  muscle_entry probe;
  muscle_entry *result = NULL;

  probe.key = key;
  result = hash_lookup (muscle_table, &probe);
  if (result)
    {
      aver (result->value == result->storage);
      return result->storage;
    }
  return NULL;
}


/* In the format `file_name:line.column', append BOUND to MUSCLE.  Use
   digraphs for special characters in the file name.  */

static void
muscle_boundary_grow (char const *key, boundary bound)
{
  char *extension;
  obstack_sgrow  (&muscle_obstack, "[[");
  obstack_escape (&muscle_obstack, bound.file);
  obstack_1grow  (&muscle_obstack, ':');
  obstack_printf (&muscle_obstack, "%d", bound.line);
  obstack_1grow  (&muscle_obstack, '.');
  obstack_printf (&muscle_obstack, "%d", bound.column);
  obstack_sgrow  (&muscle_obstack, "]]");
  obstack_1grow  (&muscle_obstack, '\0');
  extension = obstack_finish (&muscle_obstack);
  muscle_grow (key, extension, "");
  obstack_free (&muscle_obstack, extension);
}


/* In the format `[[file_name:line.column]], [[file_name:line.column]]',
   append LOC to MUSCLE.  Use digraphs for special characters in each
   file name.  */

static void
muscle_location_grow (char const *key, location loc)
{
  muscle_boundary_grow (key, loc.start);
  muscle_grow (key, "", ", ");
  muscle_boundary_grow (key, loc.end);
}

#define COMMON_DECODE(Value)                                    \
  case '$':                                                     \
    aver (*++(Value) == ']');                                   \
    aver (*++(Value) == '[');                                   \
    obstack_sgrow (&muscle_obstack, "$");                       \
    break;                                                      \
  case '@':                                                     \
    switch (*++(Value))                                         \
      {                                                         \
        case '@': obstack_sgrow (&muscle_obstack, "@" ); break; \
        case '{': obstack_sgrow (&muscle_obstack, "[" ); break; \
        case '}': obstack_sgrow (&muscle_obstack, "]" ); break; \
        default: aver (false); break;                           \
      }                                                         \
    break;                                                      \
  default:                                                      \
    obstack_1grow (&muscle_obstack, *(Value));                  \
    break;

/* Reverse of obstack_escape.  */
static char *
string_decode (char const *key)
{
  char const *value;
  char *value_decoded;
  char *result;

  value = muscle_find_const (key);
  if (!value)
    return NULL;
  do {
    switch (*value)
      {
        COMMON_DECODE (value)
        case '[':
        case ']':
          aver (false);
          break;
      }
  } while (*value++);
  value_decoded = obstack_finish (&muscle_obstack);
  result = xstrdup (value_decoded);
  obstack_free (&muscle_obstack, value_decoded);
  return result;
}

/* Reverse of muscle_location_grow.  */
static location
location_decode (char const *key)
{
  location loc;
  char const *value = muscle_find_const (key);
  aver (value);
  aver (*value == '[');
  aver (*++value == '[');
  while (*++value)
    switch (*value)
      {
        COMMON_DECODE (value)
        case '[':
          aver (false);
          break;
        case ']':
          {
            char *boundary_str;
            aver (*++value == ']');
            obstack_1grow (&muscle_obstack, '\0');
            boundary_str = obstack_finish (&muscle_obstack);
            switch (*++value)
              {
                case ',':
                  boundary_set_from_string (&loc.start, boundary_str);
                  obstack_free (&muscle_obstack, boundary_str);
                  aver (*++value == ' ');
                  aver (*++value == '[');
                  aver (*++value == '[');
                  break;
                case '\0':
                  boundary_set_from_string (&loc.end, boundary_str);
                  obstack_free (&muscle_obstack, boundary_str);
                  return loc;
                  break;
                default:
                  aver (false);
                  break;
              }
          }
          break;
      }
  aver (false);
  return loc;
}

void
muscle_user_name_list_grow (char const *key, char const *user_name,
                            location loc)
{
  muscle_grow (key, "[[[[", ",");
  muscle_grow (key, user_name, "");
  muscle_grow (key, "]], ", "");
  muscle_location_grow (key, loc);
  muscle_grow (key, "]]", "");
}

# define ARRAY_CARDINALITY(Array) (sizeof (Array) / sizeof *(Array))
#define STREQ(s1, s2) ((strcmp (s1, s2) == 0))

/** If the \a variable name is obsolete, return the name to use,
 * otherwise \a variable. */
static
char const *
muscle_percent_variable_update (char const *variable)
{
  typedef struct
  {
    const char *obsolete;
    const char *updated;
  } conversion_type;
  const conversion_type conversion[] =
    {
      { "api.push_pull", "api.push-pull", },
      { "location_type", "api.location.type", },
      { "lr.keep_unreachable_states", "lr.keep-unreachable-states", },
    };
  char const *res = variable;
  int i;
  for (i = 0; i < ARRAY_CARDINALITY (conversion); ++i)
    if (STREQ (conversion[i].obsolete, variable))
      {
        res = conversion[i].updated;
        break;
      }
  return res;
}

void
muscle_percent_define_insert (char const *var, location variable_loc,
                              char const *value,
                              muscle_percent_define_how how)
{
  /* Backward compatibility.  */
  char const *variable = muscle_percent_variable_update (var);
  char const *name = UNIQSTR_CONCAT ("percent_define(", variable, ")");
  char const *loc_name = UNIQSTR_CONCAT ("percent_define_loc(", variable, ")");
  char const *syncline_name =
    UNIQSTR_CONCAT ("percent_define_syncline(", variable, ")");
  char const *how_name = UNIQSTR_CONCAT ("percent_define_how(", variable, ")");

  /* Command-line options are processed before the grammar file.  */
  if (how == MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE
      && muscle_find_const (name))
    {
      unsigned i = 0;
      muscle_percent_define_how how_old =
        atoi (muscle_find_const (how_name));
      if (how_old == MUSCLE_PERCENT_DEFINE_F)
        return;
      complain_at_indent (variable_loc, &i,
                          _("%%define variable %s redefined"), quote (variable));
      i += SUB_INDENT;
      complain_at_indent (muscle_percent_define_get_loc (variable), &i,
                          _("previous definition"));
    }

  MUSCLE_INSERT_STRING (name, value);
  muscle_insert (loc_name, "");
  muscle_location_grow (loc_name, variable_loc);
  muscle_insert (syncline_name, "");
  muscle_syncline_grow (syncline_name, variable_loc);
  muscle_user_name_list_grow ("percent_define_user_variables", variable,
                              variable_loc);
  MUSCLE_INSERT_INT (how_name, how);
}

char *
muscle_percent_define_get (char const *variable)
{
  char const *name;
  char const *usage_name;
  char *value;

  name = UNIQSTR_CONCAT ("percent_define(", variable, ")");
  usage_name = UNIQSTR_CONCAT ("percent_define_bison_variables(",
                               variable, ")");

  muscle_insert (usage_name, "");
  value = string_decode (name);
  if (!value)
    value = xstrdup ("");
  return value;
}

location
muscle_percent_define_get_loc (char const *variable)
{
  char const *loc_name;
  loc_name = UNIQSTR_CONCAT ("percent_define_loc(", variable, ")");
  if (!muscle_find_const (loc_name))
    fatal(_("%s: undefined %%define variable %s"),
          "muscle_percent_define_get_loc", quote (variable));
  return location_decode (loc_name);
}

char const *
muscle_percent_define_get_syncline (char const *variable)
{
  char const *syncline_name;
  char const *syncline;
  syncline_name =
    UNIQSTR_CONCAT ("percent_define_syncline(", variable, ")");
  syncline = muscle_find_const (syncline_name);
  if (!syncline)
    fatal(_("%s: undefined %%define variable %s"),
          "muscle_percent_define_get_syncline", quote (variable));
  return syncline;
}

bool
muscle_percent_define_ifdef (char const *variable)
{
  char const *name;
  char const *usage_name;
  char const *value;

  name = UNIQSTR_CONCAT ("percent_define(", variable, ")");
  usage_name =
    UNIQSTR_CONCAT ("percent_define_bison_variables(", variable, ")");

  value = muscle_find_const (name);
  if (value)
    {
      muscle_insert (usage_name, "");
      return true;
    }

  return false;
}

bool
muscle_percent_define_flag_if (char const *variable)
{
  char const *invalid_boolean_name;
  bool result = false;

  invalid_boolean_name =
    UNIQSTR_CONCAT ("percent_define_invalid_boolean(", variable, ")");

  if (muscle_percent_define_ifdef (variable))
    {
      char *value = muscle_percent_define_get (variable);
      if (value[0] == '\0' || 0 == strcmp (value, "true"))
        result = true;
      else if (0 == strcmp (value, "false"))
        result = false;
      else if (!muscle_find_const (invalid_boolean_name))
        {
          muscle_insert (invalid_boolean_name, "");
          complain_at(muscle_percent_define_get_loc (variable),
                      _("invalid value for %%define Boolean variable %s"),
                      quote (variable));
        }
      free (value);
    }
  else
    fatal(_("%s: undefined %%define variable %s"),
          "muscle_percent_define_flag", quote (variable));

  return result;
}

void
muscle_percent_define_default (char const *variable, char const *value)
{
  char const *name;
  char const *loc_name;
  char const *syncline_name;
  name = UNIQSTR_CONCAT ("percent_define(", variable, ")");
  loc_name = UNIQSTR_CONCAT ("percent_define_loc(", variable, ")");
  syncline_name =
    UNIQSTR_CONCAT ("percent_define_syncline(", variable, ")");
  if (!muscle_find_const (name))
    {
      location loc;
      MUSCLE_INSERT_STRING (name, value);
      loc.start.file = loc.end.file = "<default value>";
      loc.start.line = loc.end.line = -1;
      loc.start.column = loc.end.column = -1;
      muscle_insert (loc_name, "");
      muscle_location_grow (loc_name, loc);
      muscle_insert (syncline_name, "");
    }
}

void
muscle_percent_define_check_values (char const * const *values)
{
  for (; *values; ++values)
    {
      char const * const *variablep = values;
      char const *name;
      char *value;

      name = UNIQSTR_CONCAT ("percent_define(", *variablep, ")");

      value = string_decode (name);
      if (value)
        {
          for (++values; *values; ++values)
            {
              if (0 == strcmp (value, *values))
                break;
            }
          if (!*values)
            {
              unsigned i = 0;
              location loc = muscle_percent_define_get_loc (*variablep);
              complain_at_indent (loc, &i,
                                _("invalid value for %%define variable %s: %s"),
                                  quote (*variablep), quote_n (1, value));
              i += SUB_INDENT;
              for (values = variablep + 1; *values; ++values)
                complain_at_indent (loc, &i, _("accepted value: %s"),
                                    quote (*values));
            }
          else
            {
              while (*values)
                ++values;
            }
          free (value);
        }
      else
        fatal (_("%s: undefined %%define variable %s"),
               "muscle_percent_define_check_values", quote (*variablep));
    }
}

void
muscle_percent_code_grow (char const *qualifier, location qualifier_loc,
                          char const *code, location code_loc)
{
  char const *name;
  name = UNIQSTR_CONCAT ("percent_code(", qualifier, ")");
  muscle_code_grow (name, code, code_loc);
  muscle_user_name_list_grow ("percent_code_user_qualifiers", qualifier,
                               qualifier_loc);
}


/*------------------------------------------------.
| Output the definition of ENTRY as a m4_define.  |
`------------------------------------------------*/

static  bool
muscle_m4_output (muscle_entry *entry, FILE *out)
{
  fprintf (out, "m4_define([b4_%s],\n", entry->key);
  fprintf (out, "[[%s]])\n\n\n", entry->value);
  return true;
}

static bool
muscle_m4_output_processor (void *entry, void *out)
{
  return muscle_m4_output (entry, out);
}


/*----------------------------------------------------------------.
| Output the definition of all the current muscles into a list of |
| m4_defines.                                                     |
`----------------------------------------------------------------*/

void
muscles_m4_output (FILE *out)
{
  hash_do_for_each (muscle_table, muscle_m4_output_processor, out);
}
