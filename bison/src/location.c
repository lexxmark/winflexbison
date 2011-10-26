/* Locations for Bison

   Copyright (C) 2002, 2005-2011 Free Software Foundation, Inc.

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

#include <mbswidth.h>
#include <quotearg.h>

#include "complain.h"
#include "location.h"

location const empty_location = EMPTY_LOCATION_INIT;

/* If BUF is null, add BUFSIZE (which in this case must be less than
   INT_MAX) to COLUMN; otherwise, add mbsnwidth (BUF, BUFSIZE, 0) to
   COLUMN.  If an overflow occurs, or might occur but is undetectable,
   return INT_MAX.  Assume COLUMN is nonnegative.  */

static  int
add_column_width (int column, char const *buf, size_t bufsize)
{
  size_t width;
  unsigned int remaining_columns = INT_MAX - column;

  if (buf)
    {
      if (INT_MAX / 2 <= bufsize)
	return INT_MAX;
      width = mbsnwidth (buf, bufsize, 0);
    }
  else
    width = bufsize;

  return width <= remaining_columns ? column + width : INT_MAX;
}

/* Set *LOC and adjust scanner cursor to account for token TOKEN of
   size SIZE.  */

void
location_compute (location *loc, boundary *cur, char const *token, size_t size)
{
  int line = cur->line;
  int column = cur->column;
  char const *p0 = token;
  char const *p = token;
  char const *lim = token + size;

  loc->start = *cur;

  for (p = token; p < lim; p++)
    switch (*p)
      {
      case '\n':
	line += line < INT_MAX;
	column = 1;
	p0 = p + 1;
	break;

      case '\t':
	column = add_column_width (column, p0, p - p0);
	column = add_column_width (column, NULL, 8 - ((column - 1) & 7));
	p0 = p + 1;
	break;

      default:
	break;
      }

  cur->line = line;
  cur->column = column = add_column_width (column, p0, p - p0);

  loc->end = *cur;

  if (line == INT_MAX && loc->start.line != INT_MAX)
    warn_at (*loc, _("line number overflow"));
  if (column == INT_MAX && loc->start.column != INT_MAX)
    warn_at (*loc, _("column number overflow"));
}


/* Output to OUT the location LOC.
   Warning: it uses quotearg's slot 3.  */
unsigned
location_print (FILE *out, location loc)
{
  unsigned res = 0;
  int end_col = 0 != loc.end.column ? loc.end.column - 1 : 0;
  res += fprintf (out, "%s",
                  quotearg_n_style (3, escape_quoting_style, loc.start.file));
  if (0 <= loc.start.line)
    {
      res += fprintf(out, ":%d", loc.start.line);
      if (0 <= loc.start.column)
        res += fprintf (out, ".%d", loc.start.column);
    }
  if (loc.start.file != loc.end.file)
    {
      res += fprintf (out, "-%s",
                      quotearg_n_style (3, escape_quoting_style,
                                        loc.end.file));
      if (0 <= loc.end.line)
        {
          res += fprintf(out, ":%d", loc.end.line);
          if (0 <= end_col)
            res += fprintf (out, ".%d", end_col);
        }
    }
  else if (0 <= loc.end.line)
    {
      if (loc.start.line < loc.end.line)
        {
          res += fprintf (out, "-%d", loc.end.line);
          if (0 <= end_col)
            res += fprintf (out, ".%d", end_col);
        }
      else if (0 <= end_col && loc.start.column < end_col)
        res += fprintf (out, "-%d", end_col);
    }

  return res;
}

#include <mbstring.h>

void
boundary_set_from_string (boundary *bound, char *loc_str)
{
  /* Must search in reverse since the file name field may
   * contain `.' or `:'.  */
  char *delim = _mbsrchr (loc_str, '.');
  aver (delim);
  *delim = '\0';
  bound->column = atoi (delim+1);
  delim = _mbsrchr (loc_str, ':');
  aver (delim);
  *delim = '\0';
  bound->line = atoi (delim+1);
  bound->file = uniqstr_new (loc_str);
}
