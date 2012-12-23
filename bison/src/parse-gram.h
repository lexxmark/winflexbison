/* A Bison parser, made by GNU Bison 2.6.5.63-3ada.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_GRAM__________SRC_PARSE_GRAM_H_INCLUDED
# define YY_GRAM__________SRC_PARSE_GRAM_H_INCLUDED
/* Enabling traces.  */
#ifndef GRAM_DEBUG
# if defined YYDEBUG
#  if YYDEBUG
#   define GRAM_DEBUG 1
#  else
#   define GRAM_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define GRAM_DEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined GRAM_DEBUG */
#if GRAM_DEBUG
extern int gram_debug;
#endif

/* Tokens.  */
#ifndef GRAM_TOKENTYPE
# define GRAM_TOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum gram_tokentype {
     GRAM_EOF = 0,
     STRING = 258,
     INT = 259,
     PERCENT_TOKEN = 260,
     PERCENT_NTERM = 261,
     PERCENT_TYPE = 262,
     PERCENT_DESTRUCTOR = 263,
     PERCENT_PRINTER = 264,
     PERCENT_LEFT = 265,
     PERCENT_RIGHT = 266,
     PERCENT_NONASSOC = 267,
     PERCENT_PREC = 268,
     PERCENT_DPREC = 269,
     PERCENT_MERGE = 270,
     PERCENT_CODE = 271,
     PERCENT_DEBUG = 272,
     PERCENT_DEFAULT_PREC = 273,
     PERCENT_DEFINE = 274,
     PERCENT_DEFINES = 275,
     PERCENT_ERROR_VERBOSE = 276,
     PERCENT_EXPECT = 277,
     PERCENT_EXPECT_RR = 278,
     PERCENT_FILE_PREFIX = 279,
     PERCENT_GLR_PARSER = 280,
     PERCENT_INITIAL_ACTION = 281,
     PERCENT_LANGUAGE = 282,
     PERCENT_LEX_PARAM = 283,
     PERCENT_LOCATIONS = 284,
     PERCENT_NAME_PREFIX = 285,
     PERCENT_NO_DEFAULT_PREC = 286,
     PERCENT_NO_LINES = 287,
     PERCENT_NONDETERMINISTIC_PARSER = 288,
     PERCENT_OUTPUT = 289,
     PERCENT_PARSE_PARAM = 290,
     PERCENT_PURE_PARSER = 291,
     PERCENT_REQUIRE = 292,
     PERCENT_SKELETON = 293,
     PERCENT_START = 294,
     PERCENT_TOKEN_TABLE = 295,
     PERCENT_VERBOSE = 296,
     PERCENT_YACC = 297,
     BRACED_CODE = 298,
     BRACKETED_ID = 299,
     CHAR = 300,
     EPILOGUE = 301,
     EQUAL = 302,
     ID = 303,
     ID_COLON = 304,
     PERCENT_PERCENT = 305,
     PIPE = 306,
     PROLOGUE = 307,
     SEMICOLON = 308,
     TYPE = 309,
     TYPE_TAG_ANY = 310,
     TYPE_TAG_NONE = 311,
     PERCENT_UNION = 312
   };
#endif
/* Tokens.  */
#define GRAM_EOF 0
#define STRING 258
#define INT 259
#define PERCENT_TOKEN 260
#define PERCENT_NTERM 261
#define PERCENT_TYPE 262
#define PERCENT_DESTRUCTOR 263
#define PERCENT_PRINTER 264
#define PERCENT_LEFT 265
#define PERCENT_RIGHT 266
#define PERCENT_NONASSOC 267
#define PERCENT_PREC 268
#define PERCENT_DPREC 269
#define PERCENT_MERGE 270
#define PERCENT_CODE 271
#define PERCENT_DEBUG 272
#define PERCENT_DEFAULT_PREC 273
#define PERCENT_DEFINE 274
#define PERCENT_DEFINES 275
#define PERCENT_ERROR_VERBOSE 276
#define PERCENT_EXPECT 277
#define PERCENT_EXPECT_RR 278
#define PERCENT_FILE_PREFIX 279
#define PERCENT_GLR_PARSER 280
#define PERCENT_INITIAL_ACTION 281
#define PERCENT_LANGUAGE 282
#define PERCENT_LEX_PARAM 283
#define PERCENT_LOCATIONS 284
#define PERCENT_NAME_PREFIX 285
#define PERCENT_NO_DEFAULT_PREC 286
#define PERCENT_NO_LINES 287
#define PERCENT_NONDETERMINISTIC_PARSER 288
#define PERCENT_OUTPUT 289
#define PERCENT_PARSE_PARAM 290
#define PERCENT_PURE_PARSER 291
#define PERCENT_REQUIRE 292
#define PERCENT_SKELETON 293
#define PERCENT_START 294
#define PERCENT_TOKEN_TABLE 295
#define PERCENT_VERBOSE 296
#define PERCENT_YACC 297
#define BRACED_CODE 298
#define BRACKETED_ID 299
#define CHAR 300
#define EPILOGUE 301
#define EQUAL 302
#define ID 303
#define ID_COLON 304
#define PERCENT_PERCENT 305
#define PIPE 306
#define PROLOGUE 307
#define SEMICOLON 308
#define TYPE 309
#define TYPE_TAG_ANY 310
#define TYPE_TAG_NONE 311
#define PERCENT_UNION 312



#if ! defined GRAM_STYPE && ! defined GRAM_STYPE_IS_DECLARED
typedef union GRAM_STYPE
{
/* Line 2042 of yacc.c  */
#line 115 "parse-gram.y"

  symbol *symbol;
  symbol_list *list;
  int integer;
  char const *chars;
  char *code;
  assoc assoc;
  uniqstr uniqstr;
  unsigned char character;
  named_ref *named_ref;


/* Line 2042 of yacc.c  */
#line 194 "parse-gram.h"
} GRAM_STYPE;
# define GRAM_STYPE_IS_TRIVIAL 1
# define gram_stype GRAM_STYPE /* obsolescent; will be withdrawn */
# define GRAM_STYPE_IS_DECLARED 1
#endif

#if ! defined GRAM_LTYPE && ! defined GRAM_LTYPE_IS_DECLARED
typedef struct GRAM_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} GRAM_LTYPE;
# define gram_ltype GRAM_LTYPE /* obsolescent; will be withdrawn */
# define GRAM_LTYPE_IS_DECLARED 1
# define GRAM_LTYPE_IS_TRIVIAL 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int gram_parse (void *YYPARSE_PARAM);
#else
int gram_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int gram_parse (void);
#else
int gram_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_GRAM__________SRC_PARSE_GRAM_H_INCLUDED  */
