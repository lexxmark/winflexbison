/* GNU m4 -- A simple macro processor
   Copyright (C) 1989, 90, 91, 92, 93 Free Software Foundation, Inc.
  
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Handling of path search of included files via the builtins "include"
   and "sinclude".  */

#include "m4.h"

struct includes
{
  struct includes *next;	/* next directory to search */
  const char *dir;		/* directory */
  int len;
};

typedef struct includes includes;

static includes *dir_list;		/* the list of path directories */
static includes *dir_list_end;		/* the end of same */
static int dir_max_length;		/* length of longest directory name */


void
include_init (void)
{
  dir_list = NULL;
  dir_list_end = NULL;
  dir_max_length = 0;
}

void
include_env_init (void)
{
  char *path;
  char *path_end;

  if (no_gnu_extensions)
    return;

  path = getenv ("M4PATH");
  if (path == NULL)
    return;

  do
    {
      path_end = strchr (path, ':');
      if (path_end)
	*path_end = '\0';
      add_include_directory (path);
      path = path_end + 1;
    }
  while (path_end);
}

void
add_include_directory (const char *dir)
{
  includes *incl;

  if (no_gnu_extensions)
    return;

  if (*dir == '\0')
    dir = ".";

  incl = (includes *) xmalloc (sizeof (struct includes));
  incl->next = NULL;
  incl->len = strlen (dir);
  incl->dir = xstrdup (dir);

  if (incl->len > dir_max_length) /* remember len of longest directory */
    dir_max_length = incl->len;

  if (dir_list_end == NULL)
    dir_list = incl;
  else
    dir_list_end->next = incl;
  dir_list_end = incl;

#ifdef DEBUG_INCL
  fprintf (stderr, "add_include_directory (%s);\n", dir);
#endif
}

FILE *
path_search (const char *dir)
{
  FILE *fp;
  includes *incl;
  char *name;			/* buffer for constructed name */

  /* Look in current working directory first.  */
  fp = fopen (dir, "r");
  if (fp != NULL)
    return fp;

  /* If file not found, and filename absolute, fail.  */
  if (*dir == '/' || no_gnu_extensions)
    return NULL;

  name = (char *) xmalloc (dir_max_length + 1 + strlen (dir) + 1);

  for (incl = dir_list; incl != NULL; incl = incl->next)
    {
      strncpy (name, incl->dir, incl->len);
      name[incl->len] = '/';
      strcpy (name + incl->len + 1, dir);

#ifdef DEBUG_INCL
      fprintf (stderr, "path_search (%s) -- trying %s\n", dir, name);
#endif

      fp = fopen (name, "r");
      if (fp != NULL)
	{
	  if (debug_level & DEBUG_TRACE_PATH)
	    DEBUG_MESSAGE (("path search for `%s' found `%s'", dir, name));
	  break;
	}
    }
  xfree (name);
  return fp;
}

#ifdef DEBUG_INCL

static int
include_dump (void)
{
  includes *incl;

  fprintf (stderr, "include_dump:\n");
  for (incl = dir_list; incl != NULL; incl = incl->next)
    fprintf (stderr, "\t%s\n", incl->dir);
}

#endif /* DEBUG_INCL */
