%{/* Bison Grammar Parser                             -*- C -*-

   Copyright (C) 2002-2011 Free Software Foundation, Inc.

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
#include "conflicts.h"
#include "files.h"
#include "getargs.h"
#include "gram.h"
#include "muscle-tab.h"
#include "named-ref.h"
#include "quotearg.h"
#include "reader.h"
#include "symlist.h"
#include "scan-gram.h"
#include "scan-code.h"

#define YYLLOC_DEFAULT(Current, Rhs, N)  (Current) = lloc_default (Rhs, N)
static YYLTYPE lloc_default (YYLTYPE const *, int);

#define YY_LOCATION_PRINT(File, Loc) \
	  location_print (File, Loc)

static void version_check (location const *loc, char const *version);

/* Request detailed syntax error messages, and pass them to GRAM_ERROR.
   FIXME: depends on the undocumented availability of YYLLOC.  */
#undef  yyerror
#define yyerror(Msg) \
	gram_error (&yylloc, Msg)
static void gram_error (location const *, char const *);

static char const *char_name (char);

/** Add a lex-param or a parse-param.
 *
 * \param type  \a lex_param or \a parse_param
 * \param decl  the formal argument
 * \param loc   the location in the source.
 */
static void add_param (char const *type, char *decl, location loc);


static symbol_class current_class = unknown_sym;
static uniqstr current_type = NULL;
static symbol *current_lhs_symbol;
static location current_lhs_location;
static named_ref *current_lhs_named_ref;
static int current_prec = 0;

/** Set the new current left-hand side symbol, possibly common
 * to several right-hand side parts of rule.
 */
static
void
current_lhs(symbol *sym, location loc, named_ref *ref)
{
  current_lhs_symbol = sym;
  current_lhs_location = loc;
  /* In order to simplify memory management, named references for lhs
     are always assigned by deep copy into the current symbol_list
     node.  This is because a single named-ref in the grammar may
     result in several uses when the user factors lhs between several
     rules using "|".  Therefore free the parser's original copy.  */
  free (current_lhs_named_ref);
  current_lhs_named_ref = ref;
}


#define YYTYPE_INT16 int_fast16_t
#define YYTYPE_INT8 int_fast8_t
#define YYTYPE_UINT16 uint_fast16_t
#define YYTYPE_UINT8 uint_fast8_t
%}

%debug
%verbose
%defines
%locations
%pure-parser
%error-verbose
%define parse.lac full
%name-prefix="gram_"
%expect 0

%initial-action
{
  /* Bison's grammar can initial empty locations, hence a default
     location is needed. */
  boundary_set (&@$.start, current_file, 1, 1);
  boundary_set (&@$.end, current_file, 1, 1);
}

%union
{
  symbol *symbol;
  symbol_list *list;
  int integer;
  char const *chars;
  char *code;
  assoc assoc;
  uniqstr uniqstr;
  unsigned char character;
  named_ref *named_ref;
};

/* Define the tokens together with their human representation.  */
%token GRAM_EOF 0 "end of file"
%token STRING     "string"
%token INT        "integer"

%token PERCENT_TOKEN       "%token"
%token PERCENT_NTERM       "%nterm"

%token PERCENT_TYPE        "%type"
%token PERCENT_DESTRUCTOR  "%destructor"
%token PERCENT_PRINTER     "%printer"

%token PERCENT_LEFT        "%left"
%token PERCENT_RIGHT       "%right"
%token PERCENT_NONASSOC    "%nonassoc"

%token PERCENT_PREC          "%prec"
%token PERCENT_DPREC         "%dprec"
%token PERCENT_MERGE         "%merge"


/*----------------------.
| Global Declarations.  |
`----------------------*/

%token
  PERCENT_CODE            "%code"
  PERCENT_DEBUG           "%debug"
  PERCENT_DEFAULT_PREC    "%default-prec"
  PERCENT_DEFINE          "%define"
  PERCENT_DEFINES         "%defines"
  PERCENT_ERROR_VERBOSE   "%error-verbose"
  PERCENT_EXPECT          "%expect"
  PERCENT_EXPECT_RR	  "%expect-rr"
  PERCENT_FILE_PREFIX     "%file-prefix"
  PERCENT_GLR_PARSER      "%glr-parser"
  PERCENT_INITIAL_ACTION  "%initial-action"
  PERCENT_LANGUAGE        "%language"
  PERCENT_LEX_PARAM       "%lex-param"
  PERCENT_LOCATIONS       "%locations"
  PERCENT_NAME_PREFIX     "%name-prefix"
  PERCENT_NO_DEFAULT_PREC "%no-default-prec"
  PERCENT_NO_LINES        "%no-lines"
  PERCENT_NONDETERMINISTIC_PARSER
			  "%nondeterministic-parser"
  PERCENT_OUTPUT          "%output"
  PERCENT_PARSE_PARAM     "%parse-param"
  PERCENT_PURE_PARSER     "%pure-parser"
  PERCENT_REQUIRE	  "%require"
  PERCENT_SKELETON        "%skeleton"
  PERCENT_START           "%start"
  PERCENT_TOKEN_TABLE     "%token-table"
  PERCENT_VERBOSE         "%verbose"
  PERCENT_YACC            "%yacc"
;

%token BRACED_CODE     "{...}"
%token BRACKETED_ID    "[identifier]"
%token CHAR            "char"
%token EPILOGUE        "epilogue"
%token EQUAL           "="
%token ID              "identifier"
%token ID_COLON        "identifier:"
%token PERCENT_PERCENT "%%"
%token PIPE            "|"
%token PROLOGUE        "%{...%}"
%token SEMICOLON       ";"
%token TYPE            "type"
%token TYPE_TAG_ANY    "<*>"
%token TYPE_TAG_NONE   "<>"

%type <character> CHAR
%printer { fputs (char_name ($$), stderr); } CHAR

/* braceless is not to be used for rule or symbol actions, as it
   calls code_props_plain_init.  */
%type <chars> STRING "%{...%}" EPILOGUE braceless content.opt
%type <code> "{...}"
%printer { fputs (quotearg_style (c_quoting_style, $$), stderr); }
	 STRING
%printer { fprintf (stderr, "{\n%s\n}", $$); }
	 braceless content.opt "{...}" "%{...%}" EPILOGUE

%type <uniqstr> BRACKETED_ID ID ID_COLON TYPE variable
%printer { fputs ($$, stderr); } <uniqstr>
%printer { fprintf (stderr, "[%s]", $$); } BRACKETED_ID
%printer { fprintf (stderr, "%s:", $$); } ID_COLON
%printer { fprintf (stderr, "<%s>", $$); } TYPE

%type <integer> INT
%printer { fprintf (stderr, "%d", $$); } <integer>

%type <symbol> id id_colon string_as_id symbol symbol.prec
%printer { fprintf (stderr, "%s", $$->tag); } <symbol>
%printer { fprintf (stderr, "%s:", $$->tag); } id_colon

%type <assoc> precedence_declarator
%type <list>  symbols.1 symbols.prec generic_symlist generic_symlist_item
%type <named_ref> named_ref.opt

%%

input:
  prologue_declarations "%%" grammar epilogue.opt
;


	/*------------------------------------.
	| Declarations: before the first %%.  |
	`------------------------------------*/

prologue_declarations:
  /* Nothing */
| prologue_declarations prologue_declaration
;

prologue_declaration:
  grammar_declaration
| "%{...%}"
    {
      code_props plain_code;
      code_props_plain_init (&plain_code, $1, @1);
      code_props_translate_code (&plain_code);
      gram_scanner_last_string_free ();
      muscle_code_grow (union_seen ? "post_prologue" : "pre_prologue",
                        plain_code.code, @1);
      code_scanner_last_string_free ();
    }
| "%debug"                         { debug_flag = true; }
| "%define" variable content.opt
    {
      muscle_percent_define_insert ($2, @2, $3,
                                    MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
    }
| "%defines"                       { defines_flag = true; }
| "%defines" STRING
    {
      defines_flag = true;
      spec_defines_file = xstrdup ($2);
    }
| "%error-verbose"                 { error_verbose = true; }
| "%expect" INT                    { expected_sr_conflicts = $2; }
| "%expect-rr" INT		   { expected_rr_conflicts = $2; }
| "%file-prefix" STRING            { spec_file_prefix = $2; }
| "%file-prefix" "=" STRING        { spec_file_prefix = $3; } /* deprecated */
| "%glr-parser"
    {
      nondeterministic_parser = true;
      glr_parser = true;
    }
| "%initial-action" "{...}"
    {
      code_props action;
      code_props_symbol_action_init (&action, $2, @2);
      code_props_translate_code (&action);
      gram_scanner_last_string_free ();
      muscle_code_grow ("initial_action", action.code, @2);
      code_scanner_last_string_free ();
    }
| "%language" STRING		{ language_argmatch ($2, grammar_prio, @1); }
| "%lex-param" "{...}"		{ add_param ("lex_param", $2, @2); }
| "%locations"                  { locations_flag = true; }
| "%name-prefix" STRING         { spec_name_prefix = $2; }
| "%name-prefix" "=" STRING     { spec_name_prefix = $3; } /* deprecated */
| "%no-lines"                   { no_lines_flag = true; }
| "%nondeterministic-parser"	{ nondeterministic_parser = true; }
| "%output" STRING              { spec_outfile = $2; }
| "%output" "=" STRING          { spec_outfile = $3; }  /* deprecated */
| "%parse-param" "{...}"	{ add_param ("parse_param", $2, @2); }
| "%pure-parser"
    {
      /* %pure-parser is deprecated in favor of `%define api.pure', so use
         `%define api.pure' in a backward-compatible manner here.  First, don't
         complain if %pure-parser is specified multiple times.  */
      if (!muscle_find_const ("percent_define(api.pure)"))
        muscle_percent_define_insert ("api.pure", @1, "",
                                      MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
      /* In all cases, use api.pure now so that the backend doesn't complain if
         the skeleton ignores api.pure, but do warn now if there's a previous
         conflicting definition from an actual %define.  */
      if (!muscle_percent_define_flag_if ("api.pure"))
        muscle_percent_define_insert ("api.pure", @1, "",
                                      MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
    }
| "%require" STRING             { version_check (&@2, $2); }
| "%skeleton" STRING
    {
      char const *skeleton_user = $2;
      if (mbschr (skeleton_user, '/'))
        {
          size_t dir_length = strlen (current_file);
          char *skeleton_build;
          while (dir_length && current_file[dir_length - 1] != '/')
            --dir_length;
          while (dir_length && current_file[dir_length - 1] == '/')
            --dir_length;
          skeleton_build =
            xmalloc (dir_length + 1 + strlen (skeleton_user) + 1);
          if (dir_length > 0)
            {
              strncpy (skeleton_build, current_file, dir_length);
              skeleton_build[dir_length++] = '/';
            }
          strcpy (skeleton_build + dir_length, skeleton_user);
          skeleton_user = uniqstr_new (skeleton_build);
          free (skeleton_build);
        }
      skeleton_arg (skeleton_user, grammar_prio, @1);
    }
| "%token-table"                { token_table_flag = true; }
| "%verbose"                    { report_flag |= report_states; }
| "%yacc"                       { yacc_flag = true; }
| /*FIXME: Err?  What is this horror doing here? */ ";"
;

grammar_declaration:
  precedence_declaration
| symbol_declaration
| "%start" symbol
    {
      grammar_start_symbol_set ($2, @2);
    }
| "%destructor" "{...}" generic_symlist
    {
      symbol_list *list;
      for (list = $3; list; list = list->next)
	symbol_list_destructor_set (list, $2, @2);
      symbol_list_free ($3);
    }
| "%printer" "{...}" generic_symlist
    {
      symbol_list *list;
      for (list = $3; list; list = list->next)
	symbol_list_printer_set (list, $2, @2);
      symbol_list_free ($3);
    }
| "%default-prec"
    {
      default_prec = true;
    }
| "%no-default-prec"
    {
      default_prec = false;
    }
| "%code" braceless
    {
      /* Do not invoke muscle_percent_code_grow here since it invokes
         muscle_user_name_list_grow.  */
      muscle_code_grow ("percent_code()", $2, @2);
      code_scanner_last_string_free ();
    }
| "%code" ID braceless
    {
      muscle_percent_code_grow ($2, @2, $3, @3);
      code_scanner_last_string_free ();
    }
;


/*----------*
 | %union.  |
 *----------*/

%token PERCENT_UNION "%union";

union_name:
  /* Nothing. */ {}
| ID             { muscle_code_grow ("union_name", $1, @1); }
;

grammar_declaration:
  "%union" union_name braceless
    {
      union_seen = true;
      muscle_code_grow ("stype", $3, @3);
      code_scanner_last_string_free ();
    }
;




symbol_declaration:
  "%nterm" { current_class = nterm_sym; } symbol_defs.1
    {
      current_class = unknown_sym;
      current_type = NULL;
    }
| "%token" { current_class = token_sym; } symbol_defs.1
    {
      current_class = unknown_sym;
      current_type = NULL;
    }
| "%type" TYPE symbols.1
    {
      symbol_list *list;
      tag_seen = true;
      for (list = $3; list; list = list->next)
	symbol_type_set (list->content.sym, $2, @2);
      symbol_list_free ($3);
    }
;

precedence_declaration:
  precedence_declarator type.opt symbols.prec
    {
      symbol_list *list;
      ++current_prec;
      for (list = $3; list; list = list->next)
	{
	  symbol_type_set (list->content.sym, current_type, @2);
	  symbol_precedence_set (list->content.sym, current_prec, $1, @1);
	}
      symbol_list_free ($3);
      current_type = NULL;
    }
;

precedence_declarator:
  "%left"     { $$ = left_assoc; }
| "%right"    { $$ = right_assoc; }
| "%nonassoc" { $$ = non_assoc; }
;

type.opt:
  /* Nothing. */ { current_type = NULL; }
| TYPE           { current_type = $1; tag_seen = true; }
;

/* Just like symbols.1 but accept INT for the sake of POSIX.  */
symbols.prec:
  symbol.prec
    { $$ = symbol_list_sym_new ($1, @1); }
| symbols.prec symbol.prec
    { $$ = symbol_list_prepend ($1, symbol_list_sym_new ($2, @2)); }
;

symbol.prec:
    symbol { $$ = $1; }
  | symbol INT { $$ = $1; symbol_user_token_number_set ($1, $2, @2); }
  ;

/* One or more symbols to be %typed. */
symbols.1:
  symbol
    { $$ = symbol_list_sym_new ($1, @1); }
| symbols.1 symbol
    { $$ = symbol_list_prepend ($1, symbol_list_sym_new ($2, @2)); }
;

generic_symlist:
  generic_symlist_item { $$ = $1; }
| generic_symlist generic_symlist_item { $$ = symbol_list_prepend ($1, $2); }
;

generic_symlist_item:
  symbol            { $$ = symbol_list_sym_new ($1, @1); }
| TYPE              { $$ = symbol_list_type_new ($1, @1); }
| "<*>"             { $$ = symbol_list_default_tagged_new (@1); }
| "<>"             { $$ = symbol_list_default_tagless_new (@1); }
;

/* One token definition.  */
symbol_def:
  TYPE
     {
       current_type = $1;
       tag_seen = true;
     }
| id
     {
       symbol_class_set ($1, current_class, @1, true);
       symbol_type_set ($1, current_type, @1);
     }
| id INT
    {
      symbol_class_set ($1, current_class, @1, true);
      symbol_type_set ($1, current_type, @1);
      symbol_user_token_number_set ($1, $2, @2);
    }
| id string_as_id
    {
      symbol_class_set ($1, current_class, @1, true);
      symbol_type_set ($1, current_type, @1);
      symbol_make_alias ($1, $2, @$);
    }
| id INT string_as_id
    {
      symbol_class_set ($1, current_class, @1, true);
      symbol_type_set ($1, current_type, @1);
      symbol_user_token_number_set ($1, $2, @2);
      symbol_make_alias ($1, $3, @$);
    }
;

/* One or more symbol definitions. */
symbol_defs.1:
  symbol_def
| symbol_defs.1 symbol_def
;


	/*------------------------------------------.
	| The grammar section: between the two %%.  |
	`------------------------------------------*/

grammar:
  rules_or_grammar_declaration
| grammar rules_or_grammar_declaration
;

/* As a Bison extension, one can use the grammar declarations in the
   body of the grammar.  */
rules_or_grammar_declaration:
  rules
| grammar_declaration ";"
| error ";"
    {
      yyerrok;
    }
;

rules:
  id_colon named_ref.opt { current_lhs ($1, @1, $2); } rhses.1
  {
    /* Free the current lhs. */
    current_lhs (0, @1, 0);
  }
;

rhses.1:
  rhs                { grammar_current_rule_end (@1); }
| rhses.1 "|" rhs    { grammar_current_rule_end (@3); }
| rhses.1 ";"
;

rhs:
  /* Nothing.  */
    { grammar_current_rule_begin (current_lhs_symbol, current_lhs_location,
				  current_lhs_named_ref); }
| rhs symbol named_ref.opt
    { grammar_current_rule_symbol_append ($2, @2, $3); }
| rhs "{...}" named_ref.opt
    { grammar_current_rule_action_append ($2, @2, $3); }
| rhs "%prec" symbol
    { grammar_current_rule_prec_set ($3, @3); }
| rhs "%dprec" INT
    { grammar_current_rule_dprec_set ($3, @3); }
| rhs "%merge" TYPE
    { grammar_current_rule_merge_set ($3, @3); }
;

named_ref.opt:
  /* Nothing. */ { $$ = 0; }
|
  BRACKETED_ID   { $$ = named_ref_new($1, @1); }
;


/*----------------------------*
 | variable and content.opt.  |
 *---------------------------*/

/* The STRING form of variable is deprecated and is not M4-friendly.
   For example, M4 fails for `%define "[" "value"'.  */
variable:
  ID
| STRING { $$ = uniqstr_new ($1); }
;

/* Some content or empty by default. */
content.opt:
  /* Nothing. */   { $$ = ""; }
| ID { $$ = $1; }
| STRING
;


/*-------------*
 | braceless.  |
 *-------------*/

braceless:
  "{...}"
    {
      code_props plain_code;
      $1[strlen ($1) - 1] = '\n';
      code_props_plain_init (&plain_code, $1+1, @1);
      code_props_translate_code (&plain_code);
      gram_scanner_last_string_free ();
      $$ = plain_code.code;
    }
;


/*---------------*
 | Identifiers.  |
 *---------------*/

/* Identifiers are returned as uniqstr values by the scanner.
   Depending on their use, we may need to make them genuine symbols.  */

id:
  ID
    { $$ = symbol_from_uniqstr ($1, @1); }
| CHAR
    {
      $$ = symbol_get (char_name ($1), @1);
      symbol_class_set ($$, token_sym, @1, false);
      symbol_user_token_number_set ($$, $1, @1);
    }
;

id_colon:
  ID_COLON { $$ = symbol_from_uniqstr ($1, @1); }
;


symbol:
  id
| string_as_id
;

/* A string used as an ID: quote it.  */
string_as_id:
  STRING
    {
      $$ = symbol_get (quotearg_style (c_quoting_style, $1), @1);
      symbol_class_set ($$, token_sym, @1, false);
    }
;

epilogue.opt:
  /* Nothing.  */
| "%%" EPILOGUE
    {
      code_props plain_code;
      code_props_plain_init (&plain_code, $2, @2);
      code_props_translate_code (&plain_code);
      gram_scanner_last_string_free ();
      muscle_code_grow ("epilogue", plain_code.code, @2);
      code_scanner_last_string_free ();
    }
;

%%


/* Return the location of the left-hand side of a rule whose
   right-hand side is RHS[1] ... RHS[N].  Ignore empty nonterminals in
   the right-hand side, and return an empty location equal to the end
   boundary of RHS[0] if the right-hand side is empty.  */

static YYLTYPE
lloc_default (YYLTYPE const *rhs, int n)
{
  int i;
  YYLTYPE loc;

  /* SGI MIPSpro 7.4.1m miscompiles "loc.start = loc.end = rhs[n].end;".
     The bug is fixed in 7.4.2m, but play it safe for now.  */
  loc.start = rhs[n].end;
  loc.end = rhs[n].end;

  /* Ignore empty nonterminals the start of the the right-hand side.
     Do not bother to ignore them at the end of the right-hand side,
     since empty nonterminals have the same end as their predecessors.  */
  for (i = 1; i <= n; i++)
    if (! equal_boundaries (rhs[i].start, rhs[i].end))
      {
	loc.start = rhs[i].start;
	break;
      }

  return loc;
}


/* Add a lex-param or a parse-param (depending on TYPE) with
   declaration DECL and location LOC.  */

static void
add_param (char const *type, char *decl, location loc)
{
  static char const alphanum[26 + 26 + 1 + 10] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "_"
    "0123456789";
  char const *name_start = NULL;
  char *p;

  /* Stop on last actual character.  */
  for (p = decl; p[1]; p++)
    if ((p == decl
	 || ! memchr (alphanum, p[-1], sizeof alphanum))
	&& memchr (alphanum, p[0], sizeof alphanum - 10))
      name_start = p;

  /* Strip the surrounding '{' and '}', and any blanks just inside
     the braces.  */
  while (*--p == ' ' || *p == '\t')
    continue;
  p[1] = '\0';
  while (*++decl == ' ' || *decl == '\t')
    continue;

  if (! name_start)
    complain_at (loc, _("missing identifier in parameter declaration"));
  else
    {
      char *name;
      size_t name_len;

      for (name_len = 1;
	   memchr (alphanum, name_start[name_len], sizeof alphanum);
	   name_len++)
	continue;

      name = xmalloc (name_len + 1);
      memcpy (name, name_start, name_len);
      name[name_len] = '\0';
      muscle_pair_list_grow (type, decl, name);
      free (name);
    }

  gram_scanner_last_string_free ();
}


static void
version_check (location const *loc, char const *version)
{
  if (strverscmp (version, PACKAGE_VERSION) > 0)
    {
      complain_at (*loc, "require bison %s, but have %s",
		   version, PACKAGE_VERSION);
      exit (63);
    }
}

static void
gram_error (location const *loc, char const *msg)
{
  complain_at (*loc, "%s", msg);
}

char const *
token_name (int type)
{
  return yytname[YYTRANSLATE (type)];
}

static char const *
char_name (char c)
{
  if (c == '\'')
    return "'\\''";
  else
    {
      char buf[4];
      buf[0] = '\''; buf[1] = c; buf[2] = '\''; buf[3] = '\0';
      return quotearg_style (escape_quoting_style, buf);
    }
}
