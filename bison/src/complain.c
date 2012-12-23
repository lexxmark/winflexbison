/* Declaration for error-reporting function for Bison.

   Copyright (C) 2000-2002, 2004-2006, 2009-2012 Free Software
   Foundation, Inc.

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

/* Based on error.c and error.h,
   written by David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include <config.h>
#include "system.h"

#include <stdarg.h>
#include <progname.h>

#include "complain.h"
#include "files.h"
#include "getargs.h"

bool complaint_issued;
static unsigned *indent_ptr = 0;



/** Report an error message.
 *
 * \param loc     the location, defaulting to the current file,
 *                or the program name.
 * \param prefix  put before the message (e.g., "warning").
 * \param message the error message, a printf format string.  Iff it
 *                ends with ": ", then no trailing newline is printed,
 *                and the caller should print the remaining
 *                newline-terminated message to stderr.
 * \param args    the arguments of the format string.
 */
static
void
error_message (location *loc,
	       const char *prefix,
	       const char *message, va_list args)
{
  unsigned pos = 0;

  if (loc)
    pos += location_print (stderr, *loc);
  else
    pos += fprintf(stderr, "%s", current_file ? current_file : program_name);
  pos += fprintf(stderr, ": ");

  if (indent_ptr)
    {
      if (!*indent_ptr)
        *indent_ptr = pos;
      else if (*indent_ptr > pos)
        fprintf (stderr, "%*s", *indent_ptr - pos, "");
      indent_ptr = 0;
    }

  if (prefix)
    fprintf (stderr, "%s: ", prefix);

  vfprintf (stderr, message, args);
  {
    size_t l = strlen (message);
    if (l < 2 || message[l - 2] != ':' || message[l - 1] != ' ')
      {
        putc ('\n', stderr);
        fflush (stderr);
        if (loc && feature_flag & feature_caret)
          location_caret (stderr, *loc);
      }
  }
  fflush (stderr);
}

/** Wrap error_message() with varargs handling. */
#define ERROR_MESSAGE(Loc, Prefix, Message)	\
{						\
  va_list args;					\
  va_start (args, Message);			\
  error_message (Loc, Prefix, Message, args);	\
  va_end (args);				\
}


/*--------------------------------.
| Report a warning, and proceed.  |
`--------------------------------*/

void
set_warning_issued (void)
{
  static bool warning_issued = false;
  if (!warning_issued && (warnings_flag & warnings_error))
    {
      fprintf (stderr, "%s: warnings being treated as errors\n", program_name);
      complaint_issued = true;
    }
  warning_issued = true;
}

void
warn_at (location loc, const char *message, ...)
{
  if (!(warnings_flag & warnings_other))
    return;
  set_warning_issued ();
  ERROR_MESSAGE (&loc, _("warning"), message);
}

void
warn_at_indent (location loc, unsigned *indent,
                const char *message, ...)
{
  if (!(warnings_flag & warnings_other))
    return;
  set_warning_issued ();
  indent_ptr = indent;
  ERROR_MESSAGE (&loc, *indent ? NULL : _("warning"), message);
}

void
warn (const char *message, ...)
{
  if (!(warnings_flag & warnings_other))
    return;
  set_warning_issued ();
  ERROR_MESSAGE (NULL, _("warning"), message);
}


/*-----------------------------------------------------------.
| An error has occurred, but we can proceed, and die later.  |
`-----------------------------------------------------------*/

void
complain_at (location loc, const char *message, ...)
{
  ERROR_MESSAGE (&loc, _("error"), message);
  complaint_issued = true;
}

void
complain_at_indent (location loc, unsigned *indent,
                    const char *message, ...)
{
  indent_ptr = indent;
  ERROR_MESSAGE (&loc, *indent ? NULL : _("error"), message);
  complaint_issued = true;
}

void
complain (const char *message, ...)
{
  ERROR_MESSAGE (NULL, _("error"), message);
  complaint_issued = true;
}


/*--------------------------------------------------------------.
| An incompatibility with POSIX Yacc: mapped either to warn* or |
| complain* depending on yacc_flag.                             |
`--------------------------------------------------------------*/

void
yacc_at (location loc, const char *message, ...)
{
  if (yacc_flag)
    {
      ERROR_MESSAGE (&loc, NULL, message);
      complaint_issued = true;
    }
  else if (warnings_flag & warnings_yacc)
    {
      set_warning_issued ();
      ERROR_MESSAGE (&loc, _("warning"), message);
    }
}

void
midrule_value_at (location loc, const char *message, ...)
{
  if (!(warnings_flag & warnings_midrule_values))
    return;
  set_warning_issued ();
  ERROR_MESSAGE (&loc, _("warning"), message);
}

/*-------------------------------------------------.
| A severe error has occurred, we cannot proceed.  |
`-------------------------------------------------*/

void
fatal_at (location loc, const char *message, ...)
{
  ERROR_MESSAGE (&loc, _("fatal error"), message);
  exit (EXIT_FAILURE);
}

void
fatal (const char *message, ...)
{
  ERROR_MESSAGE (NULL, _("fatal error"), message);
  exit (EXIT_FAILURE);
}
