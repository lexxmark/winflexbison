/* Lists of symbols for Bison

   Copyright (C) 2002, 2005-2007, 2009-2012 Free Software Foundation,
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

#include "complain.h"
#include "symlist.h"


/*--------------------------------------.
| Create a list containing SYM at LOC.  |
`--------------------------------------*/

symbol_list *
symbol_list_sym_new (symbol *sym, location loc)
{
  symbol_list *res = xmalloc (sizeof *res);

  res->content_type = SYMLIST_SYMBOL;
  res->content.sym = sym;
  res->location = res->sym_loc = loc;

  res->midrule = NULL;
  res->midrule_parent_rule = NULL;
  res->midrule_parent_rhs_index = 0;

  code_props_none_init (&res->action_props);

  res->ruleprec = NULL;
  res->dprec = 0;
  res->merger = 0;

  res->named_ref = NULL;

  res->next = NULL;

  return res;
}


/*--------------------------------------------.
| Create a list containing TYPE_NAME at LOC.  |
`--------------------------------------------*/

symbol_list *
symbol_list_type_new (uniqstr type_name, location loc)
{
  symbol_list *res = xmalloc (sizeof *res);

  res->content_type = SYMLIST_TYPE;
  res->content.type_name = type_name;
  res->location = res->sym_loc = loc;
  res->named_ref = NULL;
  res->next = NULL;

  return res;
}


/*----------------------------------------.
| Create a list containing a <*> at LOC.  |
`----------------------------------------*/

symbol_list *
symbol_list_default_tagged_new (location loc)
{
  symbol_list *res = xmalloc (sizeof *res);

  res->content_type = SYMLIST_DEFAULT_TAGGED;
  res->location = res->sym_loc = loc;
  res->named_ref = NULL;
  res->next = NULL;

  return res;
}


/*---------------------------------------.
| Create a list containing a <> at LOC.  |
`---------------------------------------*/

symbol_list *
symbol_list_default_tagless_new (location loc)
{
  symbol_list *res = xmalloc (sizeof *res);

  res->content_type = SYMLIST_DEFAULT_TAGLESS;
  res->location = res->sym_loc = loc;
  res->named_ref = NULL;
  res->next = NULL;

  return res;
}


/*-----------------------------------------------------------------------.
| Print this list, for which every content_type must be SYMLIST_SYMBOL.  |
`-----------------------------------------------------------------------*/

void
symbol_list_syms_print (const symbol_list *l, FILE *f)
{
  for (/* Nothing. */; l && l->content.sym; l = l->next)
    {
      symbol_print (l->content.sym, f);
      fprintf (stderr, l->action_props.is_value_used ? " used" : " unused");
      if (l && l->content.sym)
	fprintf (f, ", ");
    }
}


/*---------------------------.
| Prepend NODE to the LIST.  |
`---------------------------*/

symbol_list *
symbol_list_prepend (symbol_list *list, symbol_list *node)
{
  node->next = list;
  return node;
}


/*-----------------------------------------------.
| Free the LIST, but not the items it contains.  |
`-----------------------------------------------*/

void
symbol_list_free (symbol_list *list)
{
  symbol_list *node, *next;
  for (node = list; node; node = next)
    {
      next = node->next;
      named_ref_free (node->named_ref);
      free (node);
    }
}


/*--------------------.
| Return its length.  |
`--------------------*/

int
symbol_list_length (symbol_list const *l)
{
  int res = 0;
  for (/* Nothing. */;
       l && !(l->content_type == SYMLIST_SYMBOL && l->content.sym == NULL);
       l = l->next)
    ++res;
  return res;
}


/*------------------------------.
| Get item N in symbol list L.  |
`------------------------------*/

symbol_list *
symbol_list_n_get (symbol_list *l, int n)
{
  int i;

  if (n < 0)
    return NULL;

  for (i = 0; i < n; ++i)
    {
      l = l->next;
      if (l == NULL
          || (l->content_type == SYMLIST_SYMBOL && l->content.sym == NULL))
	return NULL;
    }

  return l;
}


/*--------------------------------------------------------------.
| Get the data type (alternative in the union) of the value for |
| symbol N in symbol list L.                                    |
`--------------------------------------------------------------*/

uniqstr
symbol_list_n_type_name_get (symbol_list *l, location loc, int n)
{
  l = symbol_list_n_get (l, n);
  if (!l)
    {
      complain_at (loc, _("invalid $ value: $%d"), n);
      return NULL;
    }
  aver (l->content_type == SYMLIST_SYMBOL);
  return l->content.sym->type_name;
}

bool
symbol_list_null (symbol_list *node)
{
  return !node ||
    (node->content_type == SYMLIST_SYMBOL && !(node->content.sym));
}

void
symbol_list_destructor_set (symbol_list *node, code_props const *destructor)
{
  switch (node->content_type)
    {
      case SYMLIST_SYMBOL:
        symbol_destructor_set (node->content.sym, destructor);
        break;
      case SYMLIST_TYPE:
        semantic_type_destructor_set (
          semantic_type_get (node->content.type_name), destructor);
        break;
      case SYMLIST_DEFAULT_TAGGED:
        default_tagged_destructor_set (destructor);
        break;
      case SYMLIST_DEFAULT_TAGLESS:
        default_tagless_destructor_set (destructor);
        break;
    }
}

void
symbol_list_printer_set (symbol_list *node, code_props const *printer)
{
  switch (node->content_type)
    {
      case SYMLIST_SYMBOL:
        symbol_printer_set (node->content.sym, printer);
        break;
      case SYMLIST_TYPE:
        semantic_type_printer_set (
          semantic_type_get (node->content.type_name), printer);
        break;
      case SYMLIST_DEFAULT_TAGGED:
        default_tagged_printer_set (printer);
        break;
      case SYMLIST_DEFAULT_TAGLESS:
        default_tagless_printer_set (printer);
        break;
    }
}
