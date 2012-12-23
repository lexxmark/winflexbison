/* Definitions for symtab.c and callers, part of Bison.

   Copyright (C) 1984, 1989, 1992, 2000-2002, 2004-2007, 2009-2012 Free
   Software Foundation, Inc.

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

/**
 * \file symtab.h
 * \brief Manipulating ::symbol.
 */

#ifndef SYMTAB_H_
# define SYMTAB_H_

# include "assoc.h"
# include "location.h"
# include "scan-code.h"
# include "uniqstr.h"

/*----------.
| Symbols.  |
`----------*/

/** Symbol classes.  */
typedef enum
{
  unknown_sym,          /**< Undefined.  */
  token_sym,		/**< Terminal. */
  nterm_sym		/**< Non-terminal. */
} symbol_class;


/** Internal token numbers. */
typedef int symbol_number;
#define SYMBOL_NUMBER_MAXIMUM INT_MAX


typedef struct symbol symbol;

/* When extending this structure, be sure to complete
   symbol_check_alias_consistency.  */
struct symbol
{
  /** The key, name of the symbol.  */
  uniqstr tag;
  /** The location of its first occurrence.  */
  location location;

  /** Its \c \%type.

      Beware that this is the type_name as was entered by the user,
      including silly things such as "]" if she entered "%token <]> t".
      Therefore, when outputting type_name to M4, be sure to escape it
      into "@}".  See quoted_output for instance.  */
  uniqstr type_name;

  /** Its \c \%type's location.  */
  location type_location;

  /** Any \c \%destructor declared specifically for this symbol.

      Access this field only through <tt>symbol</tt>'s interface
      functions.  For example, if <tt>symbol::destructor = NULL</tt>, a
      default \c \%destructor or a per-type \c \%destructor might be
      appropriate, and \c symbol_destructor_get will compute the
      correct one.  */
  code_props destructor;

  /** Any \c \%printer declared specifically for this symbol.

      Access this field only through <tt>symbol</tt>'s interface functions.
      \sa symbol::destructor  */
  code_props printer;

  symbol_number number;
  location prec_location;
  int prec;
  assoc assoc;
  int user_token_number;

  /* Points to the other in the symbol-string pair for an alias.
     Special value USER_NUMBER_HAS_STRING_ALIAS in the symbol half of the
     symbol-string pair for an alias.  */
  symbol *alias;
  symbol_class class;
  bool declared;
};

/** Undefined user number.  */
#define USER_NUMBER_UNDEFINED -1

/* `symbol->user_token_number == USER_NUMBER_HAS_STRING_ALIAS' means
   this symbol has a literal string alias.  For instance, `%token foo
   "foo"' has `"foo"' numbered regularly, and `foo' numbered as
   USER_NUMBER_HAS_STRING_ALIAS.  */
#define USER_NUMBER_HAS_STRING_ALIAS -9991

/* Undefined internal token number.  */
#define NUMBER_UNDEFINED (-1)

/** Print a symbol (for debugging). */
void symbol_print (symbol *s, FILE *f);

/** Fetch (or create) the symbol associated to KEY.  */
symbol *symbol_from_uniqstr (const uniqstr key, location loc);

/** Fetch (or create) the symbol associated to KEY.  */
symbol *symbol_get (const char *key, location loc);

/** Generate a dummy nonterminal.

   Its name cannot conflict with the user's names.  */
symbol *dummy_symbol_get (location loc);

/** Is this a dummy nonterminal?  */
bool symbol_is_dummy (const symbol *sym);

/**
 * Make \c str the literal string alias of \c sym.  Copy token number,
 * symbol number, and type from \c sym to \c str.
 */
void symbol_make_alias (symbol *sym, symbol *str, location loc);

/** Set the \c type_name associated with \c sym.

    Do nothing if passed 0 as \c type_name.  */
void symbol_type_set (symbol *sym, uniqstr type_name, location loc);

/** Set the \c destructor associated with \c sym.  */
void symbol_destructor_set (symbol *sym, code_props const *destructor);

/** Get the computed \c \%destructor for \c sym, which was initialized with
    \c code_props_none_init if there's no \c \%destructor.  */
code_props const *symbol_destructor_get (symbol const *sym);

/** Set the \c printer associated with \c sym.  */
void symbol_printer_set (symbol *sym, code_props const *printer);

/** Get the computed \c \%printer for \c sym, which was initialized with
    \c code_props_none_init if there's no \c \%printer.  */
code_props const *symbol_printer_get (symbol const *sym);

/* Set the \c precedence associated with \c sym.

   Ensure that \a symbol is a terminal.
   Do nothing if invoked with \c undef_assoc as \c assoc.  */
void symbol_precedence_set (symbol *sym, int prec, assoc a, location loc);

/** Set the \c class associated with \c sym.  */
void symbol_class_set (symbol *sym, symbol_class class, location loc,
		       bool declaring);

/** Set the \c user_token_number associated with \c sym.  */
void symbol_user_token_number_set (symbol *sym, int user_number, location loc);


/** The error token. */
extern symbol *errtoken;
/** The token for unknown tokens.  */
extern symbol *undeftoken;
/** The end of input token.  */
extern symbol *endtoken;
/** The genuine start symbol.

   $accept: start-symbol $end */
extern symbol *accept;

/** The user start symbol. */
extern symbol *startsymbol;
/** The location of the \c \%start declaration.  */
extern location startsymbol_location;


/*-----------------.
| Semantic types.  |
`-----------------*/

/** A semantic type and its associated \c \%destructor and \c \%printer.

   Access the fields of this struct only through the interface functions in
   this file.  \sa symbol::destructor  */
typedef struct {
  /** The key, name of the semantic type.  */
  uniqstr tag;

  /** Any \c %destructor declared for this semantic type.  */
  code_props destructor;
  /** Any \c %printer declared for this semantic type.  */
  code_props printer;
} semantic_type;

/** Fetch (or create) the semantic type associated to KEY.  */
semantic_type *semantic_type_from_uniqstr (const uniqstr key);

/** Fetch (or create) the semantic type associated to KEY.  */
semantic_type *semantic_type_get (const char *key);

/** Set the \c destructor associated with \c type.  */
void semantic_type_destructor_set (semantic_type *type,
                                   code_props const *destructor);

/** Set the \c printer associated with \c type.  */
void semantic_type_printer_set (semantic_type *type,
                                code_props const *printer);

/*----------------------------------.
| Symbol and semantic type tables.  |
`----------------------------------*/

/** Create the symbol and semantic type tables.  */
void symbols_new (void);

/** Free all the memory allocated for symbols and semantic types.  */
void symbols_free (void);

/** Check that all the symbols are defined.

    Report any undefined symbols and consider them nonterminals.  */
void symbols_check_defined (void);

/** Sanity checks and #token_translations construction.

   Perform various sanity checks, assign symbol numbers, and set up
   #token_translations.  */
void symbols_pack (void);


/*---------------------------------------.
| Default %destructor's and %printer's.  |
`---------------------------------------*/

/** Set the default \c \%destructor for tagged values.  */
void default_tagged_destructor_set (code_props const *destructor);
/** Set the default \c \%destructor for tagless values.  */
void default_tagless_destructor_set (code_props const *destructor);

/** Set the default \c \%printer for tagged values.  */
void default_tagged_printer_set (code_props const *printer);
/** Set the default \c \%printer for tagless values.  */
void default_tagless_printer_set (code_props const *printer);

#endif /* !SYMTAB_H_ */
