/* GNU m4 -- A simple macro processor

   Copyright (C) 1989, 1990, 1991, 1992, 1993, 1994, 2003 Free
   Software Foundation, Inc.

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

/* This file handles all the low level work around the symbol table.  The
   symbol table is a simple chained hash table.  Each symbol is described
   by a struct symbol, which is placed in the hash table based upon the
   symbol name.  Symbols that hash to the same entry in the table are
   kept on a list, sorted by name.  As a special case, to facilitate the
   "pushdef" and "popdef" builtins, a symbol can be several times in the
   symbol table, one for each definition.  Since the name is the same,
   all the entries for the symbol will be on the same list, and will
   also, because the list is sorted, be adjacent.  All the entries for a
   name are simply ordered on the list by age.  The current definition
   will then always be the first found.  */

#include "m4.h"

/*----------------------------------------------------------------------.
| Initialise the symbol table, by allocating the necessary storage, and |
| zeroing all the entries.					        |
`----------------------------------------------------------------------*/

/* Pointer to symbol table.  */
symbol **symtab;

void
symtab_init (void)
{
  int i;
  symbol **s;

  s = symtab = (symbol **) xmalloc (hash_table_size * sizeof (symbol *));

  for (i = hash_table_size; --i >= 0;)
    *s++ = NULL;
}

/*--------------------------------------------------.
| Return a hashvalue for a string, from GNU-emacs.  |
`--------------------------------------------------*/

static int
hash (const char *s)
{
  register int val = 0;

  register const char *ptr = s;
  register char ch;

  while ((ch = *ptr++) != '\0')
    {
      if (ch >= 0140)
	ch -= 40;
      val = ((val << 3) + (val >> 28) + ch);
    };
  val = (val < 0) ? -val : val;
  return val % hash_table_size;
}

/*--------------------------------------------.
| Free all storage associated with a symbol.  |
`--------------------------------------------*/

static void
free_symbol (symbol *sym)
{
  if (SYMBOL_NAME (sym))
    xfree (SYMBOL_NAME (sym));
  if (SYMBOL_TYPE (sym) == TOKEN_TEXT)
    xfree (SYMBOL_TEXT (sym));
  xfree ((voidstar) sym);
}

/*------------------------------------------------------------------------.
| Search in, and manipulation of the symbol table, are all done by	  |
| lookup_symbol ().  It basically hashes NAME to a list in the symbol	  |
| table, and searched this list for the first occurence of a symbol with  |
| the name.								  |
| 									  |
| The MODE parameter determines what lookup_symbol () will do.  It can	  |
| either just do a lookup, do a lookup and insert if not present, do an	  |
| insertion even if the name is already in the list, delete the first	  |
| occurrence of the name on the list or delete all occurences of the name |
| on the list.								  |
`------------------------------------------------------------------------*/

symbol *
lookup_symbol (const char *name, symbol_lookup mode)
{
  int h, cmp = 1;
  symbol *sym, *prev;
  symbol **spp;

  h = hash (name);
  sym = symtab[h];

  for (prev = NULL; sym != NULL; prev = sym, sym = sym->next)
    {
      cmp = strcmp (SYMBOL_NAME (sym), name);
      if (cmp >= 0)
	break;
    }

  /* If just searching, return status of search.  */

  if (mode == SYMBOL_LOOKUP)
    return cmp == 0 ? sym : NULL;

  /* Symbol not found.  */

  spp = (prev != NULL) ?  &prev->next : &symtab[h];

  switch (mode)
    {

    case SYMBOL_INSERT:

      /* Return the symbol, if the name was found in the table.
	 Otherwise, just insert the name, and return the new symbol.  */

      if (cmp == 0 && sym != NULL)
	return sym;
      /* Fall through.  */

    case SYMBOL_PUSHDEF:

      /* Insert a name in the symbol table.  If there is already a symbol
	 with the name, insert this in front of it, and mark the old
	 symbol as "shadowed".  */

      sym = (symbol *) xmalloc (sizeof (symbol));
      SYMBOL_TYPE (sym) = TOKEN_VOID;
      SYMBOL_TRACED (sym) = SYMBOL_SHADOWED (sym) = FALSE;
      SYMBOL_NAME (sym) = xstrdup (name);
      SYMBOL_SHADOWED (sym) = FALSE;
      SYMBOL_MACRO_ARGS (sym) = FALSE;
      SYMBOL_BLIND_NO_ARGS (sym) = FALSE;

      SYMBOL_NEXT (sym) = *spp;
      (*spp) = sym;

      if (mode == SYMBOL_PUSHDEF && cmp == 0)
	{
	  SYMBOL_SHADOWED (SYMBOL_NEXT (sym)) = TRUE;
	  SYMBOL_TRACED (sym) = SYMBOL_TRACED (SYMBOL_NEXT (sym));
	}
      return sym;

    case SYMBOL_DELETE:

      /* Delete all occurences of symbols with NAME.  */

      if (cmp != 0 || sym == NULL)
	return NULL;
      do
	{
	  *spp = SYMBOL_NEXT (sym);
	  free_symbol (sym);
	  sym = *spp;
	}
      while (sym != NULL && strcmp (name, SYMBOL_NAME (sym)) == 0);
      return NULL;

    case SYMBOL_POPDEF:

       /* Delete the first occurence of a symbol with NAME.  */

      if (cmp != 0 || sym == NULL)
	return NULL;
      if (SYMBOL_NEXT (sym) != NULL && cmp == 0)
	SYMBOL_SHADOWED (SYMBOL_NEXT (sym)) = FALSE;
      *spp = SYMBOL_NEXT (sym);
      free_symbol (sym);
      return NULL;

    default:
      M4ERROR ((warning_status, 0,
		"INTERNAL ERROR: Illegal mode to symbol_lookup ()"));
      abort ();
	  return NULL;
    }
}

/*----------------------------------------------------------------------.
| The following function is used for the cases, where we want to do     |
| something to each and every symbol in the table.  The function        |
| hack_all_symbols () traverses the symbol table, and calls a specified |
| function FUNC for each symbol in the table.  FUNC is called with a    |
| pointer to the symbol, and the DATA argument.			        |
`----------------------------------------------------------------------*/

void
hack_all_symbols (hack_symbol *func, const char *data)
{
  int h;
  symbol *sym;

  for (h = 0; h < hash_table_size; h++)
    {
      for (sym = symtab[h]; sym != NULL; sym = SYMBOL_NEXT (sym))
	(*func) (sym, data);
    }
}

#ifdef DEBUG_SYM

static void
symtab_debug (void)
{
  token_type t;
  token_data td;
  const char *text;
  symbol *s;
  int delete;

  while ((t = next_token (&td)) != NULL)
    {
      if (t != TOKEN_WORD)
	continue;
      text = TOKEN_DATA_TEXT (&td);
      if (*text == '_')
	{
	  delete = 1;
	  text++;
	}
      else
	delete = 0;

      s = lookup_symbol (text, SYMBOL_LOOKUP);

      if (s == NULL)
	printf ("Name `%s' is unknown\n", text);

      if (delete)
	(void) lookup_symbol (text, SYMBOL_DELETE);
      else
	(void) lookup_symbol (text, SYMBOL_INSERT);
    }
  hack_all_symbols (dump_symbol);
}

static void
symtab_print_list (int i)
{
  symbol *sym;

  printf ("Symbol dump #d:\n", i);
  for (sym = symtab[i]; sym != NULL; sym = sym->next)
    printf ("\tname %s, addr 0x%x, next 0x%x, flags%s%s\n",
	   SYMBOL_NAME (sym), sym, sym->next,
	   SYMBOL_TRACED (sym) ? " traced" : "",
	   SYMBOL_SHADOWED (sym) ? " shadowed" : "");
}

#endif /* DEBUG_SYM */
