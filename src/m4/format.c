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

/* printf like formatting for m4.  */

#include "m4.h"

#ifdef HAVE_EFGCVT

/* Various constants for floating point formatting.  */
#define MAXFIELD	128	/* size of buffer for formatted text */
/* The following two are hardware dependant.  */
#define ECVTMAX		18	/* max number of significant digits for %e */
#define FCVTMAX		(18+38+4) /* max number of significant digits for %f */

/* Externs used herein.  */
#if HAVE_EFGCVT <= 1
extern char *ecvt (), *fcvt (), *gcvt ();
#endif

#ifndef STDC_HEADERS
extern int atoi ();
extern long atol ();
extern double atof ();
#endif /* STDC_HEADERS */

#define min(a, b)	((a) < (b) ? (a) : (b))

static char const digits[] = "0123456789abcdef";
static char const Digits[] = "0123456789ABCDEF";

/* STR has dimension MAXFIELD (?).  */

static char *
ulong_to_str (register unsigned long val, char *str, int base,
	      const char *digits)
{
  register char *s = &str[MAXFIELD];

  *--s = '\0';
  do
    {
      *--s = digits[val % base];
      val /= base;
    }
  while (val > 0);

  return s;
}

/*-----------------------------------------.
| Clear trailing zeroes, return argument.  |
`-----------------------------------------*/

static char *
clr0 (char *s)
{
  register char *t;

  for (t = s + strlen (s); *--t == '0' && t > s;)
    *t = '\0';
  return s;
}

#endif /* HAVE_EFGCVT */

/* Simple varargs substitute.  */

#define ARG_INT(argc, argv) \
	((argc == 0) ? 0 : \
	 (--argc, argv++, atoi (TOKEN_DATA_TEXT (argv[-1]))))

#define ARG_UINT(argc, argv) \
	((argc == 0) ? 0 : \
	 (--argc, argv++, (unsigned int) atoi (TOKEN_DATA_TEXT (argv[-1]))))

#define ARG_LONG(argc, argv) \
	((argc == 0) ? 0 : \
	 (--argc, argv++, atol (TOKEN_DATA_TEXT (argv[-1]))))

#define ARG_ULONG(argc, argv) \
	((argc == 0) ? 0 : \
	 (--argc, argv++, (unsigned long) atol (TOKEN_DATA_TEXT (argv[-1]))))

#define ARG_STR(argc, argv) \
	((argc == 0) ? "" : \
	 (--argc, argv++, TOKEN_DATA_TEXT (argv[-1])))

#define ARG_DOUBLE(argc, argv) \
	((argc == 0) ? 0 : \
	 (--argc, argv++, atof (TOKEN_DATA_TEXT (argv[-1]))))


/*------------------------------------------------------------------------.
| The main formatting function.  Output is placed on the obstack OBS, the |
| first argument in ARGV is the formatting string, and the rest is	  |
| arguments for the string.						  |
`------------------------------------------------------------------------*/

void
format (struct obstack *obs, int argc, token_data **argv)
{
#ifdef HAVE_EFGCVT

  const char *fmt;		/* format control string */
  int c;			/* a simple character */
  char fc;			/* format code */

  /* Flags.  */
  char flags;			/* 1 iff treating flags */
  char ljust;			/* left justification */
  char mandsign;		/* mandatory sign */
  char noplus;			/* use space if no sign */
  char alternate;		/* use alternate form */
  char zeropad;			/* do zero padding */
  char plus;			/* plus-sign, according to mandatory and noplus */

  /* Precision specifiers.  */
  int width;			/* minimum field width */
  int prec;			/* precision */
  int maxch;			/* maximum no. of chars to print */
  char lflag;			/* long flag */
  char hflag;			/* short flag */

  /* Different parts of each specification.  */
  char sign;			/* wanted sign, iff any */
  int ppad;			/* pre-prefix zero padding */
  const char *prefix;		/* value prefix */
  int lpad;			/* zero padding on the left */
  register char *s;		/* ptr to formatted text */
  int rpad;			/* zero padding on the rigth*/
  const char *suffix;		/* value suffix */

  /* Buffer and stuff.  */
  char str[MAXFIELD];		/* buffer for formatted text */
  int length;			/* length of str */
  int padding;			/* padding at the left or rigth */
  register int i;		/* an index */

/* Length of trailing string in str.  */
#define LENGTH(s)	(&str[MAXFIELD-1] - (s))
#define HAS_SIGN	(sign != '\0')

  fmt = ARG_STR (argc, argv);
  for (;;)
    {
      while ((c = *fmt++) != '%')
	{
	  if (c == 0)
	    return;
	  obstack_1grow (obs, c);
	}
      if (*fmt == '%')
	{
	  obstack_1grow (obs, '%');
	  fmt++;
	  continue;
	}

      /* Parse flags.  */
      flags = 1;
      ljust = mandsign = noplus = alternate = zeropad = 0;
      do
	{
	  switch (*fmt)
	    {
	    case '-':		/* left justification */
	      ljust = 1;
	      break;

	    case '+':		/* mandatory sign */
	      mandsign = 1;
	      break;

	    case ' ':		/* space instead of positive sign */
	      noplus = 1;
	      break;

	    case '0':		/* zero padding */
	      zeropad = 1;
	      break;

	    case '#':		/* alternate output */
	      alternate = 1;
	      break;

	    default:
	      flags = 0;
	      break;
	    }
	}
      while (flags && fmt++);

      plus = '\0';		/* what to use as a plus ??? */
      if (mandsign)
	plus = '+';
      else if (noplus)
	plus = ' ';

      if (ljust)
	zeropad = 0;

      /* Minimum field width.  */
      width = -1;
      if (*fmt == '*')
	{
	  width = ARG_INT (argc, argv);
	  fmt++;
	}
      else if (isdigit (*fmt))
	{
	  width = 0;
	  do
	    {
	      width = width * 10 + *fmt++ - '0';
	    }
	  while (isdigit (*fmt));
	}

      /* Maximum precision.  */
      prec = -1;
      if (*fmt == '.')
	{
	  if (*(++fmt) == '*')
	    {
	      prec = ARG_INT (argc, argv);
	      ++fmt;
	    }
	  else if (isdigit (*fmt))
	    {
	      prec = 0;
	      do
		{
		  prec = prec * 10 + *fmt++ - '0';
		}
	      while (isdigit (*fmt))
		;
	    }
	}

      /* Length modifiers.  */
      lflag = (*fmt == 'l');
      hflag = (*fmt == 'h');
      if (lflag || hflag)
	fmt++;

      sign = '\0';
      ppad = lpad = rpad = 0;
      maxch = -1;
      prefix = suffix = "";

      switch (fc = *fmt++)
	{

	case '\0':
	  return;

	case 'c':
	  c = ARG_INT (argc, argv);
	  str[0] = (unsigned char) c;
	  str[1] = '\0';
	  s = str;
	  break;

	case 's':
	  s = ARG_STR (argc, argv);
	  maxch = prec;
	  break;

	case 'd':
	case 'i':
	  if (lflag)
	    {
	      long val = ARG_LONG (argc, argv);
	      if (val < 0)
		{
		  val = -val;	/* does not work for MINLONG */
		  sign = '-';
		}
	      else
		sign = plus;
	      s = ulong_to_str ((unsigned long) val, str, 10, digits);
	    }
	  else
	    {
	      int val = ARG_INT (argc, argv);
	      if (hflag)
		val = (short) val;
	      if (val < 0)
		{
		  val = -val;	/* does not work for MININT */
		  sign = '-';
		}
	      else
		sign = plus;
	      s = ulong_to_str ((unsigned long) val, str, 10, digits);
	    }
	  if (zeropad)
	    lpad = width - LENGTH (s) - HAS_SIGN;
	  break;

	case 'o':
	  if (lflag)
	    {
	      unsigned long val = ARG_ULONG (argc, argv);
	      s = ulong_to_str ((unsigned long) val, str, 8, digits);
	    }
	  else
	    {
	      unsigned int val = ARG_UINT (argc, argv);
	      if (hflag)
		val = (unsigned short) val;
	      s = ulong_to_str ((unsigned long) val, str, 8, digits);
	    }
	  if (alternate)
	    prefix = "0";
	  if (zeropad)
	    lpad = width - LENGTH (s) - alternate;
	  break;

	case 'x':
	case 'X':
	  if (lflag)
	    {
	      unsigned long val = ARG_ULONG (argc, argv);
	      s = ulong_to_str ((unsigned long) val, str, 16,
			       (fc == 'x') ? digits : Digits);
	    }
	  else
	    {
	      unsigned int val = ARG_UINT (argc, argv);
	      if (hflag)
		val = (unsigned short) val;
	      s = ulong_to_str ((unsigned long) val, str, 16,
			       (fc == 'x') ? digits : Digits);
	    }
	  if (alternate)
	    prefix = (fc == 'X') ? "0X" : "0x";
	  if (zeropad)
	    lpad = width - LENGTH (s) - 2*alternate;
	  break;

	case 'u':
	  if (lflag)
	    {
	      unsigned long val = ARG_ULONG (argc, argv);
	      s = ulong_to_str ((unsigned long) val, str, 10, digits);
	    }
	  else
	    {
	      unsigned int val = ARG_UINT (argc, argv);
	      if (hflag)
		val = (unsigned short) val;
	      s = ulong_to_str ((unsigned long) val, str, 10, digits);
	    }
	  if (zeropad)
	    lpad = width - LENGTH (s);
	  break;

	case 'e':
	case 'E':
	  {
	    char *t;
	    int sgn, decpt, exp, n;
	    double val = ARG_DOUBLE (argc, argv);

	    if (prec < 0)
	      prec = 6;
	    t = clr0 (ecvt (val, min (prec + 1, ECVTMAX), &decpt, &sgn));
	    sign = sgn ? '-' : plus;

	    n = prec;
	    s = str;
	    exp = (t[0] == '0' && t[1] == '\0') ? 0 : decpt - 1;

	    *s++ = *t++;
	    if (n > 0 || alternate)
	      *s++ = '.';
	    while (*t != '\0' && --n >= 0)
	      *s++ = *t++;
	    *s = '\0';
	    rpad = n;

	    sgn = 0;
	    if (exp < 0)
	      {
		exp = -exp;
		sgn = 1;
	      }
	    t = ulong_to_str ((unsigned long) exp, str, 10, digits);
	    if (exp < 10)
	      *--t = '0';	/* always at least two digits */
	    *--t = sgn ? '-' : '+';
	    *--t = fc;

	    if (zeropad)
	      {
		lpad = width - HAS_SIGN - (s - str) - LENGTH (t);
		if (rpad > 0)
		  lpad -= rpad;
	      }

	    suffix = t;
	    s = str;
	  }
	  break;

	case 'f':
	  {
	    const char *t;
	    int sgn, decpt, n;
	    double val = ARG_DOUBLE (argc, argv);

	    if (prec < 0)
	      prec = 6;

	    /* FIXME: For the following line, Dave Anglin reports
	       ``warning: passing arg 1 of `clr0' discards `const' from
	       pointer target type''.  I suspect fcvt might be declared
	       as returning const on some systems.  Pouah!  I should
	       revise this whole module, one of these days...  */

	    t = clr0 (fcvt (val, min (prec, FCVTMAX), &decpt, &sgn));

	    sign = sgn ? '-' : plus;

	    n = prec;
	    s = str;

	    if (decpt <= 0)
	      {
		prefix = (n > 0 || alternate) ? "0." : "0";
		lpad = min (-decpt, prec);
		n -= lpad;
	      }
	    else
	      {
		while (--decpt >= 0)
		  *s++ = *t++;
		if (n > 0 || alternate)
		  *s++ = '.';
	      }
	    while (*t && --n >= 0)
	      *s++ = *t++;

	    *s = '\0';
	    rpad = n;

	    if (zeropad)
	      ppad = width - HAS_SIGN - (prefix[1] ? 2 : 1) - lpad -
		(s - str) - rpad;

	    s = str;
	  }
	  break;

	default:
	  continue;
	}

      if (lpad < 0)
	lpad = 0;
      if (rpad < 0)
	rpad = 0;
      if (width < 0)
	width = 0;

      i = strlen (s);
      if (maxch <= 0 || maxch > i)
	maxch = i;

      length = (HAS_SIGN + ppad + strlen (prefix) + lpad + maxch
		+ rpad + strlen (suffix));
      padding = 0;
      if (width != 0)
	{
	  padding = width - length;
	}

      if (ljust == 0)		/* left padding */
	for (i = padding; --i >= 0;)
	  obstack_1grow (obs, ' ');
      if (HAS_SIGN)		/* sign */
	obstack_1grow (obs, sign);
      for (i = ppad; --i >= 0;)	/* pre-prefix zero padding */
	obstack_1grow (obs, '0');
      for (; *prefix; ++prefix)	/* prefix */
	obstack_1grow (obs, *prefix);
      for (i = lpad; --i >= 0;)	/* left zero padding */
	obstack_1grow (obs, '0');
      for (i = maxch; --i >= 0; ++s) /* actual text */
	obstack_1grow (obs, *s);
      for (i = rpad; --i >= 0;)	/* right zero padding */
	obstack_1grow (obs, '0');
      for (; *suffix; ++suffix)	/* suffix */
	obstack_1grow (obs, *suffix);
      if (ljust != 0)		/* right padding */
	for (i = padding; --i >= 0;)
	  obstack_1grow (obs, ' ');
    }

#else /* not HAVE_EFGCVT */

  char *fmt;			/* format control string */
  const char *fstart;		/* beginning of current format spec */
  int c;			/* a simple character */

  /* Flags.  */
  char flags;			/* 1 iff treating flags */

  /* Precision specifiers.  */
  int width;			/* minimum field width */
  int prec;			/* precision */
  char lflag;			/* long flag */
  char hflag;			/* short flag */

  /* Buffer and stuff.  */
  char str[256];		/* buffer for formatted text */
  enum {INT, UINT, LONG, ULONG, DOUBLE, STR} datatype;

  fmt = ARG_STR (argc, argv);
  for (;;)
    {
      while ((c = *fmt++) != '%')
	{
	  if (c == 0)
	    return;
	  obstack_1grow (obs, c);
	}

      fstart = fmt - 1;

      if (*fmt == '%')
	{
	  obstack_1grow (obs, '%');
	  fmt++;
	  continue;
	}

      /* Parse flags.  */
      flags = 1;
      do
	{
	  switch (*fmt)
	    {
	    case '-':		/* left justification */
	    case '+':		/* mandatory sign */
	    case ' ':		/* space instead of positive sign */
	    case '0':		/* zero padding */
	    case '#':		/* alternate output */
	      break;

	    default:
	      flags = 0;
	      break;
	    }
	}
      while (flags && fmt++);

      /* Minimum field width.  */
      width = -1;
      if (*fmt == '*')
	{
	  width = ARG_INT (argc, argv);
	  fmt++;
	}
      else if (isdigit (*fmt))
	{
	  do
	    {
	      fmt++;
	    }
	  while (isdigit (*fmt));
	}

      /* Maximum precision.  */
      prec = -1;
      if (*fmt == '.')
	{
	  if (*(++fmt) == '*')
	    {
	      prec = ARG_INT (argc, argv);
	      ++fmt;
	    }
	  else if (isdigit (*fmt))
	    {
	      do
		{
		  fmt++;
		}
	      while (isdigit (*fmt));
	    }
	}

      /* Length modifiers.  */
      lflag = (*fmt == 'l');
      hflag = (*fmt == 'h');
      if (lflag || hflag)
	fmt++;

      switch (*fmt++)
	{

	case '\0':
	  return;

	case 'c':
	  datatype = INT;
	  break;

	case 's':
	  datatype = STR;
	  break;

	case 'd':
	case 'i':
	  if (lflag)
	    {
	      datatype = LONG;
	    }
	  else
	    {
	      datatype = INT;
	    }
	  break;

	case 'o':
	case 'x':
	case 'X':
	case 'u':
	  if (lflag)
	    {
	      datatype = ULONG;
	    }
	  else
	    {
	      datatype = UINT;
	    }
	  break;

	case 'e':
	case 'E':
	case 'f':
	  datatype = DOUBLE;
	  break;

	default:
	  continue;
	}

      c = *fmt;
      *fmt = '\0';

      switch(datatype) 
	{
	case INT:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_INT(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_INT(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_INT(argc, argv));
	  else
	    sprintf (str, fstart, ARG_INT(argc, argv));
	  break;

	case UINT:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_UINT(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_UINT(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_UINT(argc, argv));
	  else
	    sprintf (str, fstart, ARG_UINT(argc, argv));
	  break;

	case LONG:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_LONG(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_LONG(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_LONG(argc, argv));
	  else
	    sprintf (str, fstart, ARG_LONG(argc, argv));
	  break;

	case ULONG:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_ULONG(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_ULONG(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_ULONG(argc, argv));
	  else
	    sprintf (str, fstart, ARG_ULONG(argc, argv));
	  break;

	case DOUBLE:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_DOUBLE(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_DOUBLE(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_DOUBLE(argc, argv));
	  else
	    sprintf (str, fstart, ARG_DOUBLE(argc, argv));
	  break;

	case STR:
	  if (width != -1 && prec != -1)
	    sprintf (str, fstart, width, prec, ARG_STR(argc, argv));
	  else if (width != -1)
	    sprintf (str, fstart, width, ARG_STR(argc, argv));
	  else if (prec != -1)
	    sprintf (str, fstart, prec, ARG_STR(argc, argv));
	  else
	    sprintf (str, fstart, ARG_STR(argc, argv));
	  break;
	}

      *fmt = c;

      obstack_grow (obs, str, strlen (str));
    }

#endif /* not HAVE_EFGCVT */
}
