/* Abstract ordered set data type, with out-of-memory checking.
   Copyright (C) 2009-2011 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2009.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "gl_xoset.h"

#if !HAVE_INLINE

gl_oset_t
gl_oset_create_empty (gl_oset_implementation_t implementation,
                      gl_setelement_compar_fn compar_fn,
                      gl_setelement_dispose_fn dispose_fn)
{
  gl_oset_t result =
    gl_oset_nx_create_empty (implementation, compar_fn, dispose_fn);
  if (result == NULL)
    xalloc_die ();
  return result;
}

bool
gl_oset_add (gl_oset_t set, const void *elt)
{
  int result = gl_oset_nx_add (set, elt);
  if (result < 0)
    xalloc_die ();
  return result;
}

#endif
