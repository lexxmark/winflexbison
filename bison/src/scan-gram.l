/* Bison Grammar Scanner                             -*- C -*-

   Copyright (C) 2002-2015, 2018-2019 Free Software Foundation, Inc.

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

%option debug nodefault noinput noyywrap never-interactive
%option prefix="gram_" outfile="lex.yy.c"

%{
#include <c-ctype.h>
#include <mbswidth.h>
#include <quote.h>
#include <quotearg.h>

#include <src/complain.h>
#include <src/files.h>
#include <src/getargs.h>
#include <src/gram.h>
#include <src/reader.h>
#include <src/scan-gram.h>
#include <src/uniqstr.h>

#define FLEX_PREFIX(Id) gram_ ## Id
#include <src/flex-scanner.h>

/* Work around a bug in flex 2.5.31.  See Debian bug 333231
   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef gram_wrap
#define gram_wrap() 1

#define YY_DECL GRAM_LEX_DECL

/* Location of scanner cursor.  */
static boundary scanner_cursor;

#define YY_USER_ACTION  location_compute (loc, &scanner_cursor, yytext, yyleng);

static size_t no_cr_read (FILE *, char *, size_t);
#define YY_INPUT(buf, result, size) ((result) = no_cr_read (yyin, buf, size))

/* Report that yytext is an extension, and evaluate to its token type.  */
#define BISON_DIRECTIVE(Directive)                              \
  (bison_directive (loc, yytext), PERCENT_ ## Directive)

#define RETURN_PERCENT_PARAM(Value)             \
  RETURN_VALUE(PERCENT_PARAM, param_ ## Value)

#define RETURN_PERCENT_FLAG(Value)                      \
  RETURN_VALUE(PERCENT_FLAG, uniqstr_new (Value))

#define RETURN_VALUE(Token, Value)              \
  do {                                          \
    val->Token = Value;                         \
    return Token;                               \
  } while (0)

#define ROLLBACK_CURRENT_TOKEN                                  \
  do {                                                          \
    scanner_cursor.column -= mbsnwidth (yytext, yyleng, 0);     \
    yyless (0);                                                 \
  } while (0)

#define DEPRECATED(Msg)                                         \
  do {                                                          \
    deprecated_directive (loc, yytext, Msg);                    \
    scanner_cursor.column -= mbsnwidth (Msg, strlen (Msg), 0);  \
    for (size_t i = strlen (Msg); i != 0; --i)                  \
      unput (Msg[i - 1]);                                       \
  } while (0)

/* A string representing the most recently saved token.  */
static char *last_string = NULL;

/* Bracketed identifier. */
static uniqstr bracketed_id_str = NULL;
static location bracketed_id_loc;
static boundary bracketed_id_start;
static int bracketed_id_context_state = 0;

void
gram_scanner_last_string_free (void)
{
  STRING_FREE;
}

static void handle_syncline (char *, location);
static unsigned long scan_integer (char const *p, int base, location loc);
static int convert_ucn_to_byte (char const *hex_text);
static void unexpected_eof (boundary, char const *);
static void unexpected_newline (boundary, char const *);

%}
 /* A C-like comment in directives/rules. */
%x SC_YACC_COMMENT
 /* Strings and characters in directives/rules. */
%x SC_ESCAPED_STRING SC_ESCAPED_CHARACTER
 /* A identifier was just read in directives/rules.  Special state
    to capture the sequence 'identifier :'. */
%x SC_AFTER_IDENTIFIER

 /* POSIX says that a tag must be both an id and a C union member, but
    historically almost any character is allowed in a tag.  We
    disallow NUL, as this simplifies our implementation.  We match
    angle brackets in nested pairs: several languages use them for
    generics/template types.  */
%x SC_TAG

 /* Four types of user code:
    - prologue (code between '%{' '%}' in the first section, before %%);
    - actions, printers, union, etc, (between braced in the middle section);
    - epilogue (everything after the second %%).
    - predicate (code between '%?{' and '{' in middle section); */
%x SC_PROLOGUE SC_BRACED_CODE SC_EPILOGUE SC_PREDICATE
 /* C and C++ comments in code. */
%x SC_COMMENT SC_LINE_COMMENT
 /* Strings and characters in code. */
%x SC_STRING SC_CHARACTER
 /* Bracketed identifiers support. */
%x SC_BRACKETED_ID SC_RETURN_BRACKETED_ID

letter    [.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_]
notletter [^.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_]{-}[%\{]
id        {letter}({letter}|[-0-9])*
int       [0-9]+
xint      0[xX][0-9abcdefABCDEF]+

/* Zero or more instances of backslash-newline.  Following GCC, allow
   white space between the backslash and the newline.  */
splice   (\\[ \f\t\v]*\n)*

/* An equal sign, with optional leading whitespaces. This is used in some
   deprecated constructs. */
sp       [[:space:]]*
eqopt    ({sp}=)?

%%
%{
  /* Nesting level.  Either for nested braces, or nested angle brackets
     (but not mixed).  */
  int nesting PACIFY_CC (= 0);

  /* Parent context state, when applicable.  */
  int context_state PACIFY_CC (= 0);

  /* Location of most recent identifier, when applicable.  */
  location id_loc PACIFY_CC (= empty_location);

  /* Where containing code started, when applicable.  Its initial
     value is relevant only when yylex is invoked in the SC_EPILOGUE
     start condition.  */
  boundary code_start = scanner_cursor;

  /* Where containing comment or string or character literal started,
     when applicable.  */
  boundary token_start PACIFY_CC (= scanner_cursor);

  /* We cannot trust YY_USER_INIT, whose semantics changes over time
     (it moved in Flex 2.5.38).  */
  static bool first = true;
  if (first)
    {
      scanner_cursor = loc->start;
      first = false;
    }
%}


  /*-----------------------.
  | Scanning white space.  |
  `-----------------------*/

<INITIAL,SC_AFTER_IDENTIFIER,SC_BRACKETED_ID,SC_RETURN_BRACKETED_ID>
{
  /* Comments and white space.  */
  "," {
     complain (loc, Wother, _("stray ',' treated as white space"));
  }
  [ \f\n\t\v]  |
  "//".*       continue;
  "/*" {
    token_start = loc->start;
    context_state = YY_START;
    BEGIN SC_YACC_COMMENT;
  }

  /* #line directives are not documented, and may be withdrawn or
     modified in future versions of Bison.  */
  ^"#line "{int}(" \"".*"\"")?"\n" {
    handle_syncline (yytext + sizeof "#line " - 1, *loc);
  }
}


  /*----------------------------.
  | Scanning Bison directives.  |
  `----------------------------*/

  /* For directives that are also command line options, the regex must be
        "%..."
     after "[-_]"s are removed, and the directive must match the --long
     option name, with a single string argument.  Otherwise, add exceptions
     to ../build-aux/cross-options.pl.  */

<INITIAL>
{
  "%binary"                         return BISON_DIRECTIVE (NONASSOC);
  "%code"                           return BISON_DIRECTIVE (CODE);
  "%debug"                          RETURN_PERCENT_FLAG ("parse.trace");
  "%default-prec"                   return BISON_DIRECTIVE (DEFAULT_PREC);
  "%define"                         return BISON_DIRECTIVE (DEFINE);
  "%defines"                        return BISON_DIRECTIVE (DEFINES);
  "%destructor"                     return BISON_DIRECTIVE (DESTRUCTOR);
  "%dprec"                          return BISON_DIRECTIVE (DPREC);
  "%empty"                          return BISON_DIRECTIVE (EMPTY);
  "%expect"                         return BISON_DIRECTIVE (EXPECT);
  "%expect-rr"                      return BISON_DIRECTIVE (EXPECT_RR);
  "%file-prefix"                    RETURN_VALUE (PERCENT_FILE_PREFIX, uniqstr_new (yytext));
  "%fixed-output-files"             RETURN_VALUE (PERCENT_YACC, uniqstr_new (yytext));
  "%initial-action"                 return BISON_DIRECTIVE (INITIAL_ACTION);
  "%glr-parser"                     return BISON_DIRECTIVE (GLR_PARSER);
  "%language"                       return BISON_DIRECTIVE (LANGUAGE);
  "%left"                           return PERCENT_LEFT;
  "%lex-param"                      RETURN_PERCENT_PARAM (lex);
  "%locations"                      RETURN_PERCENT_FLAG ("locations");
  "%merge"                          return BISON_DIRECTIVE (MERGE);
  "%no-default-prec"                return BISON_DIRECTIVE (NO_DEFAULT_PREC);
  "%no-lines"                       return BISON_DIRECTIVE (NO_LINES);
  "%nonassoc"                       return PERCENT_NONASSOC;
  "%nondeterministic-parser"        return BISON_DIRECTIVE (NONDETERMINISTIC_PARSER);
  "%nterm"                          return BISON_DIRECTIVE (NTERM);
  "%output"                         return BISON_DIRECTIVE (OUTPUT);
  "%param"                          RETURN_PERCENT_PARAM (both);
  "%parse-param"                    RETURN_PERCENT_PARAM (parse);
  "%prec"                           return PERCENT_PREC;
  "%precedence"                     return BISON_DIRECTIVE (PRECEDENCE);
  "%printer"                        return BISON_DIRECTIVE (PRINTER);
  "%pure-parser"                    RETURN_PERCENT_FLAG ("api.pure");
  "%require"                        return BISON_DIRECTIVE (REQUIRE);
  "%right"                          return PERCENT_RIGHT;
  "%skeleton"                       return BISON_DIRECTIVE (SKELETON);
  "%start"                          return PERCENT_START;
  "%term"                           return BISON_DIRECTIVE (TOKEN);
  "%token"                          return PERCENT_TOKEN;
  "%token-table"                    return BISON_DIRECTIVE (TOKEN_TABLE);
  "%type"                           return PERCENT_TYPE;
  "%union"                          return PERCENT_UNION;
  "%verbose"                        return BISON_DIRECTIVE (VERBOSE);
  "%yacc"                           RETURN_VALUE (PERCENT_YACC, uniqstr_new (yytext));

  /* Deprecated since Bison 3.0 (2013-07-25), but the warning is
     issued only since Bison 3.3. */
  "%error-verbose"                  RETURN_VALUE (PERCENT_ERROR_VERBOSE, uniqstr_new (yytext));

  /* Deprecated since Bison 2.6 (2012-07-19), but the warning is
     issued only since Bison 3.3. */
  "%name"[-_]"prefix"{eqopt}{sp}    RETURN_VALUE (PERCENT_NAME_PREFIX, uniqstr_new (yytext));

  /* Deprecated since Bison 2.7.90, 2012. */
  "%default"[-_]"prec"              DEPRECATED ("%default-prec");
  "%error"[-_]"verbose"             RETURN_VALUE (PERCENT_ERROR_VERBOSE, uniqstr_new (yytext));
  "%expect"[-_]"rr"                 DEPRECATED ("%expect-rr");
  "%file-prefix"{eqopt}             RETURN_VALUE (PERCENT_FILE_PREFIX, uniqstr_new (yytext));
  "%fixed"[-_]"output"[-_]"files"   RETURN_VALUE (PERCENT_YACC, uniqstr_new (yytext));
  "%no"[-_]"default"[-_]"prec"      DEPRECATED ("%no-default-prec");
  "%no"[-_]"lines"                  DEPRECATED ("%no-lines");
  "%output"{eqopt}                  DEPRECATED ("%output");
  "%pure"[-_]"parser"               DEPRECATED ("%pure-parser");
  "%token"[-_]"table"               DEPRECATED ("%token-table");

  "%"{id} {
    complain (loc, complaint, _("invalid directive: %s"), quote (yytext));
  }

  "="                     return EQUAL;
  "|"                     return PIPE;
  ";"                     return SEMICOLON;

  {id} {
    val->ID = uniqstr_new (yytext);
    id_loc = *loc;
    bracketed_id_str = NULL;
    BEGIN SC_AFTER_IDENTIFIER;
  }

  {int}      RETURN_VALUE (INT, scan_integer (yytext, 10, *loc));
  {xint}     RETURN_VALUE (INT, scan_integer (yytext, 16, *loc));

  /* Identifiers may not start with a digit.  Yet, don't silently
     accept "1FOO" as "1 FOO".  */
  {int}{id} {
    complain (loc, complaint, _("invalid identifier: %s"), quote (yytext));
  }

  /* Characters.  */
  "'"         token_start = loc->start; BEGIN SC_ESCAPED_CHARACTER;

  /* Strings. */
  "\""        token_start = loc->start; BEGIN SC_ESCAPED_STRING;

  /* Prologue. */
  "%{"        code_start = loc->start; BEGIN SC_PROLOGUE;

  /* Code in between braces.  */
  "{" {
    STRING_GROW;
    nesting = 0;
    code_start = loc->start;
    BEGIN SC_BRACED_CODE;
  }

  /* Semantic predicate. */
  "%?"[ \f\n\t\v]*"{" {
    nesting = 0;
    code_start = loc->start;
    BEGIN SC_PREDICATE;
  }

  /* A type. */
  "<*>"       return TAG_ANY;
  "<>"        return TAG_NONE;
  "<"         {
    nesting = 0;
    token_start = loc->start;
    BEGIN SC_TAG;
  }

  "%%" {
    static int percent_percent_count;
    if (++percent_percent_count == 2)
      BEGIN SC_EPILOGUE;
    return PERCENT_PERCENT;
  }

  "[" {
    bracketed_id_str = NULL;
    bracketed_id_start = loc->start;
    bracketed_id_context_state = YY_START;
    BEGIN SC_BRACKETED_ID;
  }

  [^\[%A-Za-z0-9_<>{}\"\'*;|=/, \f\n\t\v]+|. {
    complain (loc, complaint, "%s: %s",
              ngettext ("invalid character", "invalid characters", yyleng),
              quote_mem (yytext, yyleng));
  }

  <<EOF>> {
    loc->start = loc->end = scanner_cursor;
    yyterminate ();
  }
}


  /*--------------------------------------------------------------.
  | Supporting \0 complexifies our implementation for no expected |
  | added value.                                                  |
  `--------------------------------------------------------------*/

<SC_ESCAPED_CHARACTER,SC_ESCAPED_STRING,SC_TAG>
{
  \0        complain (loc, complaint, _("invalid null character"));
}


  /*-----------------------------------------------------------------.
  | Scanning after an identifier, checking whether a colon is next.  |
  `-----------------------------------------------------------------*/

<SC_AFTER_IDENTIFIER>
{
  "[" {
    if (bracketed_id_str)
      {
        ROLLBACK_CURRENT_TOKEN;
        BEGIN SC_RETURN_BRACKETED_ID;
        *loc = id_loc;
        return ID;
      }
    else
      {
        bracketed_id_start = loc->start;
        bracketed_id_context_state = YY_START;
        BEGIN SC_BRACKETED_ID;
      }
  }
  ":" {
    BEGIN (bracketed_id_str ? SC_RETURN_BRACKETED_ID : INITIAL);
    *loc = id_loc;
    return ID_COLON;
  }
  . {
    ROLLBACK_CURRENT_TOKEN;
    BEGIN (bracketed_id_str ? SC_RETURN_BRACKETED_ID : INITIAL);
    *loc = id_loc;
    return ID;
  }
  <<EOF>> {
    BEGIN (bracketed_id_str ? SC_RETURN_BRACKETED_ID : INITIAL);
    *loc = id_loc;
    return ID;
  }
}

  /*--------------------------------.
  | Scanning bracketed identifiers. |
  `--------------------------------*/

<SC_BRACKETED_ID>
{
  {id} {
    if (bracketed_id_str)
      {
        complain (loc, complaint,
                  _("unexpected identifier in bracketed name: %s"),
                  quote (yytext));
      }
    else
      {
        bracketed_id_str = uniqstr_new (yytext);
        bracketed_id_loc = *loc;
      }
  }
  "]" {
    BEGIN bracketed_id_context_state;
    if (bracketed_id_str)
      {
        if (INITIAL == bracketed_id_context_state)
          {
            val->BRACKETED_ID = bracketed_id_str;
            bracketed_id_str = 0;
            *loc = bracketed_id_loc;
            return BRACKETED_ID;
          }
      }
    else
      complain (loc, complaint, _("an identifier expected"));
  }

  [^\].A-Za-z0-9_/ \f\n\t\v]+|. {
    complain (loc, complaint, "%s: %s",
              ngettext ("invalid character in bracketed name",
                        "invalid characters in bracketed name", yyleng),
              quote_mem (yytext, yyleng));
  }

  <<EOF>> {
    BEGIN bracketed_id_context_state;
    unexpected_eof (bracketed_id_start, "]");
  }
}

<SC_RETURN_BRACKETED_ID>
{
  . {
    ROLLBACK_CURRENT_TOKEN;
    val->BRACKETED_ID = bracketed_id_str;
    bracketed_id_str = 0;
    *loc = bracketed_id_loc;
    BEGIN INITIAL;
    return BRACKETED_ID;
  }
}


  /*---------------------------------------------------------------.
  | Scanning a Yacc comment.  The initial '/ *' is already eaten.  |
  `---------------------------------------------------------------*/

<SC_YACC_COMMENT>
{
  "*/"     BEGIN context_state;
  .|\n     continue;
  <<EOF>>  unexpected_eof (token_start, "*/"); BEGIN context_state;
}


  /*------------------------------------------------------------.
  | Scanning a C comment.  The initial '/ *' is already eaten.  |
  `------------------------------------------------------------*/

<SC_COMMENT>
{
  "*"{splice}"/"  STRING_GROW; BEGIN context_state;
  <<EOF>>         unexpected_eof (token_start, "*/"); BEGIN context_state;
}


  /*--------------------------------------------------------------.
  | Scanning a line comment.  The initial '//' is already eaten.  |
  `--------------------------------------------------------------*/

<SC_LINE_COMMENT>
{
  "\n"           STRING_GROW; BEGIN context_state;
  {splice}       STRING_GROW;
  <<EOF>>        BEGIN context_state;
}


  /*------------------------------------------------.
  | Scanning a Bison string, including its escapes. |
  | The initial quote is already eaten.             |
  `------------------------------------------------*/

<SC_ESCAPED_STRING>
{
  "\"" {
    STRING_FINISH;
    BEGIN INITIAL;
    loc->start = token_start;
    complain (loc, Wyacc,
              _("POSIX Yacc does not support string literals"));
    RETURN_VALUE (STRING, last_string);
  }
  <<EOF>>   unexpected_eof (token_start, "\"");
  "\n"      unexpected_newline (token_start, "\"");
}

  /*----------------------------------------------------------.
  | Scanning a Bison character literal, decoding its escapes. |
  | The initial quote is already eaten.                       |
  `----------------------------------------------------------*/

<SC_ESCAPED_CHARACTER>
{
  "'" {
    STRING_FINISH;
    loc->start = token_start;
    val->CHAR = last_string[0];

    /* FIXME: Eventually, make these errors.  */
    if (last_string[0] == '\0')
    {
      complain (loc, Wother, _("empty character literal"));
      /* '\0' seems dangerous even if we are about to complain.  */
      val->CHAR = '\'';
    }
    else if (last_string[1] != '\0')
      complain (loc, Wother,
                _("extra characters in character literal"));
    STRING_FREE;
    BEGIN INITIAL;
    return CHAR;
  }
  "\n"      unexpected_newline (token_start, "'");
  <<EOF>>   unexpected_eof (token_start, "'");
}



  /*--------------------------------------------------------------.
  | Scanning a tag.  The initial angle bracket is already eaten.  |
  `--------------------------------------------------------------*/

<SC_TAG>
{
  ">" {
    --nesting;
    if (nesting < 0)
      {
        STRING_FINISH;
        loc->start = token_start;
        val->TAG = uniqstr_new (last_string);
        STRING_FREE;
        BEGIN INITIAL;
        return TAG;
      }
    STRING_GROW;
  }

  ([^<>]|->)+ STRING_GROW;
  "<"+   STRING_GROW; nesting += yyleng;

  <<EOF>>   unexpected_eof (token_start, ">");
}

  /*----------------------------.
  | Decode escaped characters.  |
  `----------------------------*/

<SC_ESCAPED_STRING,SC_ESCAPED_CHARACTER>
{
  \\[0-7]{1,3} {
    verify (UCHAR_MAX < ULONG_MAX);
    unsigned long c = strtoul (yytext + 1, NULL, 8);
    if (!c || UCHAR_MAX < c)
      complain (loc, complaint, _("invalid number after \\-escape: %s"),
                yytext+1);
    else
      obstack_1grow (&obstack_for_string, c);
  }

  \\x[0-9abcdefABCDEF]+ {
    verify (UCHAR_MAX < ULONG_MAX);
    unsigned long c = strtoul (yytext + 2, NULL, 16);
    if (!c || UCHAR_MAX < c)
      complain (loc, complaint, _("invalid number after \\-escape: %s"),
                yytext+1);
    else
      obstack_1grow (&obstack_for_string, c);
  }

  \\a   obstack_1grow (&obstack_for_string, '\a');
  \\b   obstack_1grow (&obstack_for_string, '\b');
  \\f   obstack_1grow (&obstack_for_string, '\f');
  \\n   obstack_1grow (&obstack_for_string, '\n');
  \\r   obstack_1grow (&obstack_for_string, '\r');
  \\t   obstack_1grow (&obstack_for_string, '\t');
  \\v   obstack_1grow (&obstack_for_string, '\v');

  /* \\[\"\'?\\] would be shorter, but it confuses xgettext.  */
  \\("\""|"'"|"?"|"\\")  obstack_1grow (&obstack_for_string, yytext[1]);

  \\(u|U[0-9abcdefABCDEF]{4})[0-9abcdefABCDEF]{4} {
    int c = convert_ucn_to_byte (yytext);
    if (c <= 0)
      complain (loc, complaint, _("invalid number after \\-escape: %s"),
                yytext+1);
    else
      obstack_1grow (&obstack_for_string, c);
  }
  \\(.|\n)      {
    char const *p = yytext + 1;
    /* Quote only if escaping won't make the character visible.  */
    if (c_isspace ((unsigned char) *p) && c_isprint ((unsigned char) *p))
      p = quote (p);
    else
      p = quotearg_style_mem (escape_quoting_style, p, 1);
    complain (loc, complaint, _("invalid character after \\-escape: %s"),
              p);
  }
}

  /*--------------------------------------------.
  | Scanning user-code characters and strings.  |
  `--------------------------------------------*/

<SC_CHARACTER,SC_STRING>
{
  {splice}|\\{splice}[^\n\[\]]  STRING_GROW;
}

<SC_CHARACTER>
{
  "'"           STRING_GROW; BEGIN context_state;
  \n            unexpected_newline (token_start, "'");
  <<EOF>>       unexpected_eof (token_start, "'");
}

<SC_STRING>
{
  "\""          STRING_GROW; BEGIN context_state;
  \n            unexpected_newline (token_start, "\"");
  <<EOF>>       unexpected_eof (token_start, "\"");
}


  /*---------------------------------------------------.
  | Strings, comments etc. can be found in user code.  |
  `---------------------------------------------------*/

<SC_BRACED_CODE,SC_PROLOGUE,SC_EPILOGUE,SC_PREDICATE>
{
  "'" {
    STRING_GROW;
    context_state = YY_START;
    token_start = loc->start;
    BEGIN SC_CHARACTER;
  }
  "\"" {
    STRING_GROW;
    context_state = YY_START;
    token_start = loc->start;
    BEGIN SC_STRING;
  }
  "/"{splice}"*" {
    STRING_GROW;
    context_state = YY_START;
    token_start = loc->start;
    BEGIN SC_COMMENT;
  }
  "/"{splice}"/" {
    STRING_GROW;
    context_state = YY_START;
    BEGIN SC_LINE_COMMENT;
  }
}



  /*-----------------------------------------------------------.
  | Scanning some code in braces (actions, predicates). The    |
  | initial "{" is already eaten.                              |
  `-----------------------------------------------------------*/

<SC_BRACED_CODE,SC_PREDICATE>
{
  "{"|"<"{splice}"%"  STRING_GROW; nesting++;
  "%"{splice}">"      STRING_GROW; nesting--;

  /* Tokenize '<<%' correctly (as '<<' '%') rather than incorrectly
     (as '<' '<%').  */
  "<"{splice}"<"  STRING_GROW;

  <<EOF>>   unexpected_eof (code_start, "}");
}

<SC_BRACED_CODE>
{
  "}" {
    obstack_1grow (&obstack_for_string, '}');

    --nesting;
    if (nesting < 0)
      {
        STRING_FINISH;
        loc->start = code_start;
        BEGIN INITIAL;
        RETURN_VALUE (BRACED_CODE, last_string);
      }
  }
}

<SC_PREDICATE>
{
  "}" {
    --nesting;
    if (nesting < 0)
      {
        STRING_FINISH;
        loc->start = code_start;
        BEGIN INITIAL;
        RETURN_VALUE (BRACED_PREDICATE, last_string);
      }
    else
      obstack_1grow (&obstack_for_string, '}');
  }
}

  /*--------------------------------------------------------------.
  | Scanning some prologue: from "%{" (already scanned) to "%}".  |
  `--------------------------------------------------------------*/

<SC_PROLOGUE>
{
  "%}" {
    STRING_FINISH;
    loc->start = code_start;
    BEGIN INITIAL;
    RETURN_VALUE (PROLOGUE, last_string);
  }

  <<EOF>>   unexpected_eof (code_start, "%}");
}


  /*---------------------------------------------------------------.
  | Scanning the epilogue (everything after the second "%%", which |
  | has already been eaten).                                       |
  `---------------------------------------------------------------*/

<SC_EPILOGUE>
{
  <<EOF>> {
    STRING_FINISH;
    loc->start = code_start;
    BEGIN INITIAL;
    RETURN_VALUE (EPILOGUE, last_string);
  }
}


  /*-----------------------------------------------------.
  | By default, grow the string obstack with the input.  |
  `-----------------------------------------------------*/

<SC_COMMENT,SC_LINE_COMMENT,SC_BRACED_CODE,SC_PREDICATE,SC_PROLOGUE,SC_EPILOGUE,SC_STRING,SC_CHARACTER,SC_ESCAPED_STRING,SC_ESCAPED_CHARACTER>. |
  <SC_COMMENT,SC_LINE_COMMENT,SC_BRACED_CODE,SC_PREDICATE,SC_PROLOGUE,SC_EPILOGUE>\n    STRING_GROW;

%%

/* Read bytes from FP into buffer BUF of size SIZE.  Return the
   number of bytes read.  Remove '\r' from input, treating \r\n
   and isolated \r as \n.  */

static size_t
no_cr_read (FILE *fp, char *buf, size_t size)
{
  size_t bytes_read = fread (buf, 1, size, fp);
  if (bytes_read)
    {
      char *w = memchr (buf, '\r', bytes_read);
      if (w)
        {
          char const *r = ++w;
          char const *lim = buf + bytes_read;

          for (;;)
            {
              /* Found an '\r'.  Treat it like '\n', but ignore any
                 '\n' that immediately follows.  */
              w[-1] = '\n';
              if (r == lim)
                {
                  int ch = getc (fp);
                  if (ch != '\n' && ungetc (ch, fp) != ch)
                    break;
                }
              else if (*r == '\n')
                r++;

              /* Copy until the next '\r'.  */
              do
                {
                  if (r == lim)
                    return w - buf;
                }
              while ((*w++ = *r++) != '\r');
            }

          return w - buf;
        }
    }

  return bytes_read;
}



/*------------------------------------------------------.
| Scan NUMBER for a base-BASE integer at location LOC.  |
`------------------------------------------------------*/

static unsigned long
scan_integer (char const *number, int base, location loc)
{
  verify (INT_MAX < ULONG_MAX);
  if (base == 16)
    complain (&loc, Wyacc,
              _("POSIX Yacc does not support hexadecimal literals"));

  unsigned long num = strtoul (number, NULL, base);

  if (INT_MAX < num)
    {
      complain (&loc, complaint, _("integer out of range: %s"),
                quote (number));
      num = INT_MAX;
    }

  return num;
}


/*------------------------------------------------------------------.
| Convert universal character name UCN to a single-byte character,  |
| and return that character.  Return -1 if UCN does not correspond  |
| to a single-byte character.                                       |
`------------------------------------------------------------------*/

static int
convert_ucn_to_byte (char const *ucn)
{
  verify (UCHAR_MAX <= INT_MAX);
  unsigned long code = strtoul (ucn + 2, NULL, 16);

  /* FIXME: Currently we assume Unicode-compatible unibyte characters
     on ASCII hosts (i.e., Latin-1 on hosts with 8-bit bytes).  On
     non-ASCII hosts we support only the portable C character set.
     These limitations should be removed once we add support for
     multibyte characters.  */

  if (UCHAR_MAX < code)
    return -1;

#if ! ('$' == 0x24 && '@' == 0x40 && '`' == 0x60 && '~' == 0x7e)
  {
    /* A non-ASCII host.  Use CODE to index into a table of the C
       basic execution character set, which is guaranteed to exist on
       all Standard C platforms.  This table also includes '$', '@',
       and '`', which are not in the basic execution character set but
       which are unibyte characters on all the platforms that we know
       about.  */
    static signed char const table[] =
      {
        '\0',   -1,   -1,   -1,   -1,   -1,   -1, '\a',
        '\b', '\t', '\n', '\v', '\f', '\r',   -1,   -1,
          -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
          -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
         ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
         '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
         '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
         '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
         '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',
         'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
         'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
         'X',  'Y',  'Z',  '[', '\\',  ']',  '^',  '_',
         '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
         'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
         'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
         'x',  'y',  'z',  '{',  '|',  '}',  '~'
      };

    code = code < sizeof table ? table[code] : -1;
  }
#endif

  return code;
}


/*---------------------------------------------------------------------.
| Handle '#line INT( "FILE")?\n'.  ARGS has already skipped '#line '.  |
`---------------------------------------------------------------------*/

static void
handle_syncline (char *args, location loc)
{
  char *file;
  unsigned long lineno = strtoul (args, &file, 10);
  if (INT_MAX <= lineno)
    {
      complain (&loc, Wother, _("line number overflow"));
      lineno = INT_MAX;
    }

  file = strchr (file, '"');
  if (file)
    {
      *strchr (file + 1, '"') = '\0';
      current_file = uniqstr_new (file + 1);
    }
  boundary_set (&scanner_cursor, current_file, lineno, 1);
}


/*----------------------------------------------------------------.
| For a token or comment starting at START, report message MSGID, |
| which should say that an end marker was found before the        |
| expected TOKEN_END. Then, pretend that TOKEN_END was found.     |
`----------------------------------------------------------------*/

static void
unexpected_end (boundary start, char const *msgid, char const *token_end)
{
  location loc;
  loc.start = start;
  loc.end = scanner_cursor;
  size_t i = strlen (token_end);

  /* Adjust scanner cursor so that any later message does not count
     the characters about to be inserted.  */
  scanner_cursor.column -= i;

  while (i != 0)
    unput (token_end[--i]);

  token_end = quote (token_end);
  /* Instead of '\'', display "'".  */
  if (STREQ (token_end, "'\\''"))
    token_end = "\"'\"";
  complain (&loc, complaint, msgid, token_end);
}


/*------------------------------------------------------------------------.
| Report an unexpected EOF in a token or comment starting at START.       |
| An end of file was encountered and the expected TOKEN_END was missing.  |
| After reporting the problem, pretend that TOKEN_END was found.          |
`------------------------------------------------------------------------*/

static void
unexpected_eof (boundary start, char const *token_end)
{
  unexpected_end (start, _("missing %s at end of file"), token_end);
}


/*----------------------------------------.
| Likewise, but for unexpected newlines.  |
`----------------------------------------*/

static void
unexpected_newline (boundary start, char const *token_end)
{
  unexpected_end (start, _("missing %s at end of line"), token_end);
}


/*-------------------------.
| Initialize the scanner.  |
`-------------------------*/

void
gram_scanner_initialize (void)
{
  obstack_init (&obstack_for_string);
}


/*-----------------------------------------------.
| Free all the memory allocated to the scanner.  |
`-----------------------------------------------*/

void
gram_scanner_free (void)
{
  obstack_free (&obstack_for_string, 0);
  /* Reclaim Flex's buffers.  */
  yylex_destroy ();
}
