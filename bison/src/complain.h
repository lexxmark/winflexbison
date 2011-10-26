/* Declaration for error-reporting function for Bison.

   Copyright (C) 2000-2002, 2006, 2009-2011 Free Software Foundation,
   Inc.

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

#ifndef COMPLAIN_H_
# define COMPLAIN_H_ 1

# include "location.h"

# ifdef	__cplusplus
extern "C" {
# endif

/** Record that a warning is about to be issued, and treat it as an
    error if <tt>warnings_flag & warnings_error</tt>.  This is exported
    only for the sake of Yacc-compatible conflict reports in conflicts.c.
    All other warnings should be implemented in complain.c and should use
    the normal warning format.  */
void set_warning_issued (void);

/** Informative messages, but we proceed.  Report iff
    <tt>warnings_flag & warnings_other</tt>.  */

void warn (char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 1, 2)));

void warn_at (location loc, char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 2, 3)));

/* Generate a message aligned by an indent.
   When *indent == 0, assign message's indent to *indent,
   When *indent > 0, align the message by *indent value. */
void warn_at_indent (location loc, unsigned *indent,
                     char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 3, 4)));

/** An error, but we continue and die later.  */

void complain (char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 1, 2)));

void complain_at (location loc, char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 2, 3)));

/* Generate a message aligned by an indent.
   When *indent == 0, assign message's indent to *indent,
   When *indent > 0, align the message by *indent value. */
void complain_at_indent (location loc, unsigned *indent,
                         char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 3, 4)));

/** An incompatibility with POSIX Yacc: mapped either to warn* or
    complain* depending on yacc_flag. */

void yacc_at (location loc, char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 2, 3)));

/** A midrule-value warning.  Report iff
    <tt>warnings_flag & warnings_midrule_values</tt>.  */

void midrule_value_at (location loc, char const *format, ...);
//  __attribute__ ((__format__ (__printf__, 2, 3)));

/** A fatal error, causing immediate exit.  */

void fatal (char const *format, ...);
//  __attribute__ ((__noreturn__, __format__ (__printf__, 1, 2)));

void fatal_at (location loc, char const *format, ...);
//  __attribute__ ((__noreturn__, __format__ (__printf__, 2, 3)));

/** Whether an error was reported.  */
extern bool complaint_issued;

# ifdef	__cplusplus
}
# endif

#endif /* !COMPLAIN_H_ */
