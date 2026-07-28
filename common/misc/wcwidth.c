/* Determine the number of screen columns needed for a character.
   Copyright (C) 2006-2007, 2010-2021 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <wchar.h>

/* Get iswprint.  */
#include <wctype.h>

#include "localcharset.h"
#include "streq.h"
#include "uniwidth.h"

/* Returns 1 if the current locale is an UTF-8 locale, 0 otherwise.  */
static inline int
is_locale_utf8 (void)
{
  const char *encoding = locale_charset ();
  return STREQ_OPT (encoding, "UTF-8", 'U', 'T', 'F', '-', '8', 0, 0, 0, 0);
}

#if GNULIB_WCHAR_SINGLE_LOCALE
/* When we know that the locale does not change, provide a speedup by
   caching the value of is_locale_utf8.  */
static int cached_is_locale_utf8 = -1;
static inline int
is_locale_utf8_cached (void)
{
  if (cached_is_locale_utf8 < 0)
    cached_is_locale_utf8 = is_locale_utf8 ();
  return cached_is_locale_utf8;
}
#else
/* By default, don't make assumptions, hence no caching.  */
# define is_locale_utf8_cached is_locale_utf8
#endif

int
wcwidth (wchar_t wc)
#undef wcwidth
{
  /* In UTF-8 locales, use a Unicode aware width function.  */
  if (is_locale_utf8_cached ())
    {
      /* We assume that in a UTF-8 locale, a wide character is the same as a
         Unicode character.  */
#ifndef _WIN32
      return uc_width (wc, "UTF-8");
#else
     /* Windows port: gnulib's uniwidth module is not vendored (only its
        header), so uc_width () cannot be linked.  What stood here returned the
        UTF-8 *encoded byte length* (1/2/3, 4 for surrogates) where a *column
        width* is required, so "éé" measured 4 columns instead of 2 and every
        caret diagnostic over non-ASCII source was mis-columned.

        This is a deliberately small stand-in, not a full Unicode width table:
        combining marks are zero-width, East Asian Wide/Fullwidth and the
        common emoji blocks are double-width, C0/C1 controls are -1 (matching
        uc_width, which mbswidth turns into "invalid"), everything else is 1.
        Rare ranges may still disagree with uc_width; vendoring uniwidth is the
        way to make this exact.

        wchar_t is 16-bit here, so a non-BMP character arrives as a surrogate
        pair.  The full code point is not knowable from the lead unit alone;
        report the pair's width on the lead (2 -- non-BMP characters reaching
        diagnostics are overwhelmingly emoji/CJK, which are Wide) and 0 on the
        trail so the pair totals 2 rather than being counted twice.  */
     uint32_t v = (uint32_t) wc;

     if (v == 0)
       return 0;
     if (v < 32 || (v >= 0x7f && v < 0xa0))
       return -1;                                   /* C0 / C1 controls */
     if (v >= 0xdc00 && v <= 0xdfff)
       return 0;                                    /* surrogate trail */
     if (v >= 0xd800 && v <= 0xdbff)
       {
         /* Surrogate lead.  The trail carries the low 10 bits, but the lead
            alone pins the code point to a 1024-wide span -- enough to tell the
            Wide blocks (emoji, CJK extensions) from narrow ones such as the
            Mathematical Alphanumerics at U+1D400, which are width 1.  Report
            the whole pair's width here and 0 on the trail.  */
         uint32_t base = 0x10000 + ((v - 0xd800) << 10);
         if ((base >= 0x1f300 && base <= 0x1f9ff)
             || (base >= 0x20000 && base <= 0x3fffd))
           return 2;
         return 1;
       }

     /* Zero-width: combining marks and format characters.  */
     if ((v >= 0x0300 && v <= 0x036f) || (v >= 0x0483 && v <= 0x0489)
         || (v >= 0x0591 && v <= 0x05bd) || v == 0x05bf
         || (v >= 0x0610 && v <= 0x061a) || (v >= 0x064b && v <= 0x065f)
         || v == 0x0670 || (v >= 0x06d6 && v <= 0x06dc)
         || (v >= 0x0e31 && v <= 0x0e3a) || (v >= 0x0e47 && v <= 0x0e4e)
         || (v >= 0x200b && v <= 0x200f) || (v >= 0x202a && v <= 0x202e)
         || (v >= 0x2060 && v <= 0x2064) || (v >= 0x20d0 && v <= 0x20f0)
         || (v >= 0xfe00 && v <= 0xfe0f)
         || (v >= 0xfe20 && v <= 0xfe2f) || v == 0xfeff)
       return 0;

     /* Double-width: East Asian Wide/Fullwidth.  */
     if ((v >= 0x1100 && v <= 0x115f) || (v >= 0x2e80 && v <= 0x303e)
         || (v >= 0x3041 && v <= 0x33ff) || (v >= 0x3400 && v <= 0x4dbf)
         || (v >= 0x4e00 && v <= 0x9fff) || (v >= 0xa000 && v <= 0xa4cf)
         || (v >= 0xac00 && v <= 0xd7a3) || (v >= 0xf900 && v <= 0xfaff)
         || (v >= 0xfe10 && v <= 0xfe19) || (v >= 0xfe30 && v <= 0xfe6f)
         || (v >= 0xff00 && v <= 0xff60) || (v >= 0xffe0 && v <= 0xffe6))
       return 2;

     return 1;
#endif
    }
  else
    {
      /* Otherwise, fall back to the system's wcwidth function.  */
#if HAVE_WCWIDTH
      return wcwidth (wc);
#else
      return wc == 0 ? 0 : iswprint (wc) ? 1 : -1;
#endif
    }
}
