/* Locations for Bison

   Copyright (C) 2002, 2005-2015, 2018-2019 Free Software Foundation,
   Inc.

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
#include <stdio.h>    /* fileno */
#include <sys/stat.h> /* fstat */

#include "complain.h"
#include "getargs.h"
#include "location.h"

location const empty_loc = EMPTY_LOCATION_INIT;

/* If BUF is null, add BUFSIZE (which in this case must be less than
   INT_MAX) to COLUMN; otherwise, add mbsnwidth (BUF, BUFSIZE, 0) to
   COLUMN.  If an overflow occurs, return INT_MAX.  */

static inline int
add_column_width (int column, char const *buf, size_t bufsize)
{
  int width
    = buf ? mbsnwidth (buf, bufsize, 0)
    : INT_MAX <= bufsize ? INT_MAX
    : bufsize;
  return column <= INT_MAX - width ? column + width : INT_MAX;
}

/* Set *LOC and adjust scanner cursor to account for token TOKEN of
   size SIZE.  */

void
location_compute (location *loc, boundary *cur, char const *token, size_t size)
{
  int line = cur->line;
  int column = cur->column;
  int byte = cur->byte;
  char const *p0 = token;
  char const *p = token;
  char const *lim = token + size;

  loc->start = *cur;

  for (p = token; p < lim; ++p)
    switch (*p)
      {
      case '\n':
        line += line < INT_MAX;
        column = 1;
        byte = 1;
        p0 = p + 1;
        break;

      case '\t':
        column = add_column_width (column, p0, p - p0);
        column = add_column_width (column, NULL, 8 - ((column - 1) & 7));
        p0 = p + 1;
        byte += byte < INT_MAX;
        break;

      default:
        byte += byte < INT_MAX;
        break;
      }

  cur->line = line;
  cur->column = column = add_column_width (column, p0, p - p0);
  cur->byte = byte;

  loc->end = *cur;

  if (line == INT_MAX && loc->start.line != INT_MAX)
    complain (loc, Wother, _("line number overflow"));
  if (column == INT_MAX && loc->start.column != INT_MAX)
    complain (loc, Wother, _("column number overflow"));
  if (byte == INT_MAX && loc->start.byte != INT_MAX)
    complain (loc, Wother, _("byte number overflow"));
}

static unsigned
boundary_print (boundary const *b, FILE *out)
{
  return fprintf (out, "%s:%d.%d@%d",
                  quotearg_n_style (3, escape_quoting_style, b->file),
                  b->line, b->column, b->byte);
}

unsigned
location_print (location loc, FILE *out)
{
  unsigned res = 0;
  if (trace_flag & trace_locations)
    {
      res += boundary_print (&loc.start, out);
      res += fprintf (out, "-");
      res += boundary_print (&loc.end, out);
    }
  else
    {
      int end_col = 0 != loc.end.column ? loc.end.column - 1 : 0;
      res += fprintf (out, "%s",
                      quotearg_n_style (3, escape_quoting_style, loc.start.file));
      if (0 <= loc.start.line)
        {
          res += fprintf (out, ":%d", loc.start.line);
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
              res += fprintf (out, ":%d", loc.end.line);
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
    }

  return res;
}


/* Persistent data used by location_caret to avoid reopening and rereading the
   same file all over for each error.  */
static struct
{
  FILE *source;
  /* The last file we tried to open.  If non NULL, but SOURCE is NULL,
     it means this file is special and should not be quoted. */
  uniqstr file;
  size_t line;
  /* Offset in SOURCE where line LINE starts.  */
  size_t offset;
} caret_info;

void
caret_free ()
{
  if (caret_info.source)
    {
      fclose (caret_info.source);
      caret_info.source = NULL;
    }
}

void
location_caret (location loc, const char *style, FILE *out)
{
  if (loc.start.column == -1 || loc.start.line == -1)
    return;
  /* If a different source than before, close and let the rest open
     the new one. */
  if (caret_info.file && caret_info.file != loc.start.file)
    {
      caret_free ();
      caret_info.file = NULL;
    }
  if (!caret_info.file)
    {
      caret_info.file = loc.start.file;
      if ((caret_info.source = fopen (caret_info.file, "r")))
        {
          /* If the file is not regular (imagine #line 1 "/dev/stdin"
             in the input file for instance), don't try to quote the
             source.  Keep caret_info.file set so that we don't try to
             open it again, but leave caret_info.source NULL so that
             we don't try to quote it. */
          struct stat buf;
          if (fstat (fileno (caret_info.source), &buf) == 0
              && buf.st_mode & S_IFREG)
            {
              caret_info.line = 1;
              caret_info.offset = 0;
            }
          else
            caret_free ();
        }
    }
  if (!caret_info.source)
    return;


  /* If the line we want to quote is seekable (the same line as the previous
     location), just seek it. If it was a previous line, we lost track of it,
     so return to the start of file.  */
  if (caret_info.line <= loc.start.line)
    fseek (caret_info.source, caret_info.offset, SEEK_SET);
  else
    {
      caret_info.line = 1;
      caret_info.offset = 0;
      fseek (caret_info.source, caret_info.offset, SEEK_SET);
    }

  /* Advance to the line's position, keeping track of the offset.  */
  while (caret_info.line < loc.start.line)
    caret_info.line += getc (caret_info.source) == '\n';
  caret_info.offset = ftell (caret_info.source);

  /* Read the actual line.  Don't update the offset, so that we keep a pointer
     to the start of the line.  */
  {
    int c = getc (caret_info.source);
    if (c != EOF)
      {
        /* Quote the file (at most the first line in the case of
           multiline locations).  */
        fprintf (out, "%5d | ", loc.start.line);
        bool single_line = loc.start.line == loc.end.line;
        /* Consider that single point location (with equal boundaries)
           actually denote the character that they follow.  */
        int byte_end = loc.end.byte +
          (single_line && loc.start.byte == loc.end.byte);
        /* Byte number.  */
        int byte = 1;
        while (c != EOF && c != '\n')
          {
            if (byte == loc.start.byte)
              begin_use_class (style, out);
            fputc (c, out);
            c = getc (caret_info.source);
            ++byte;
            if (single_line
                ? byte == byte_end
                : c == '\n' || c == EOF)
              end_use_class (style, out);
          }
        putc ('\n', out);

        {
          /* Print the carets with the same indentation as above.  */
          fprintf (out, "      | %*s", loc.start.column - 1, "");
          begin_use_class (style, out);
          putc ('^', out);
          /* Underlining a multiline location ends with the first
             line.  */
          int len = single_line
            ? loc.end.column
            : ftell (caret_info.source) - caret_info.offset;
          for (int i = loc.start.column + 1; i < len; ++i)
            putc ('~', out);
          end_use_class (style, out);
        }
        putc ('\n', out);
      }
  }
}

bool
location_empty (location loc)
{
  return !loc.start.file && !loc.start.line && !loc.start.column
    && !loc.end.file && !loc.end.line && !loc.end.column;
}

void
boundary_set_from_string (boundary *bound, char *str)
{
  /* Must search in reverse since the file name field may contain '.'
     or ':'.  */
  char *at = strrchr (str, '@');
  if (at)
    {
      *at = '\0';
      bound->byte = atoi (at+1);
    }
  {
    char *dot = strrchr (str, '.');
    aver (dot);
    *dot = '\0';
    bound->column = atoi (dot+1);
    if (!at)
      bound->byte = bound->column;
  }
  {
    char *colon = strrchr (str, ':');
    aver (colon);
    *colon = '\0';
    bound->line = atoi (colon+1);
  }
  bound->file = uniqstr_new (str);
}
