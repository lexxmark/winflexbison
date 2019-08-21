/* A Bison parser, made by GNU Bison 3.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */

  /* On column 0 to please syntax-check.  */
#include <config.h>

/* Substitute the type names.  */
#define YYSTYPE         GRAM_STYPE
#define YYLTYPE         GRAM_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         gram_parse
#define yylex           gram_lex
#define yyerror         gram_error
#define yydebug         gram_debug
#define yynerrs         gram_nerrs



# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#include "src/parse-gram.h"


/* Unqualified %code blocks.  */

  #include "system.h"
  #include <errno.h>

  #include "c-ctype.h"
  #include "complain.h"
  #include "conflicts.h"
  #include "files.h"
  #include "getargs.h"
  #include "gram.h"
  #include "named-ref.h"
  #include "quotearg.h"
  #include "reader.h"
  #include "scan-code.h"
  #include "scan-gram.h"
  #include "vasnprintf.h"
  #include "xmemdup0.h"

  static int current_prec = 0;
  static location current_lhs_loc;
  static named_ref *current_lhs_named_ref;
  static symbol *current_lhs_symbol;
  static symbol_class current_class = unknown_sym;

  /** Set the new current left-hand side symbol, possibly common
   * to several right-hand side parts of rule.
   */
  static void current_lhs (symbol *sym, location loc, named_ref *ref);

  #define YYLLOC_DEFAULT(Current, Rhs, N)         \
    (Current) = lloc_default (Rhs, N)
  static YYLTYPE lloc_default (YYLTYPE const *, int);

  #define YY_LOCATION_PRINT(File, Loc)            \
    location_print (Loc, File)

  /* Strip initial '{' and final '}' (must be first and last characters).
     Return the result.  */
  static char *strip_braces (char *code);

  /* Convert CODE by calling code_props_plain_init if PLAIN, otherwise
     code_props_symbol_action_init.  Call
     gram_scanner_last_string_free to release the latest string from
     the scanner (should be CODE). */
  static char const *translate_code (char *code, location loc, bool plain);

  /* Convert CODE by calling code_props_plain_init after having
     stripped the first and last characters (expected to be '{', and
     '}').  Call gram_scanner_last_string_free to release the latest
     string from the scanner (should be CODE). */
  static char const *translate_code_braceless (char *code, location loc);

  /* Handle a %error-verbose directive.  */
  static void handle_error_verbose (location const *loc, char const *directive);

  /* Handle a %file-prefix directive.  */
  static void handle_file_prefix (location const *loc,
                                  location const *dir_loc,
                                  char const *directive, char const *value);

  /* Handle a %name-prefix directive.  */
  static void handle_name_prefix (location const *loc,
                                  char const *directive, char const *value);

  /* Handle a %pure-parser directive.  */
  static void handle_pure_parser (location const *loc, char const *directive);

  /* Handle a %require directive.  */
  static void handle_require (location const *loc, char const *version);

  /* Handle a %skeleton directive.  */
  static void handle_skeleton (location const *loc, char const *skel);

  /* Handle a %yacc directive.  */
  static void handle_yacc (location const *loc, char const *directive);

  static void gram_error (location const *, char const *);

  /* A string that describes a char (e.g., 'a' -> "'a'").  */
  static char const *char_name (char);

  #define YYTYPE_INT16 int_fast16_t
  #define YYTYPE_INT8 int_fast8_t
  #define YYTYPE_UINT16 uint_fast16_t
  #define YYTYPE_UINT8 uint_fast8_t

  /* Add style to semantic values in traces.  */
  static void tron (FILE *yyo);
  static void troff (FILE *yyo);

  /** Add a lex-param and/or a parse-param.
   *
   * \param type  where to push this formal argument.
   * \param decl  the formal argument.  Destroyed.
   * \param loc   the location in the source.
   */
  static void add_param (param_type type, char *decl, location loc);
  static param_type current_param = param_none;


#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
# define YYCOPY_NEEDED 1
#endif


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined GRAM_LTYPE_IS_TRIVIAL && GRAM_LTYPE_IS_TRIVIAL \
             && defined GRAM_STYPE_IS_TRIVIAL && GRAM_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   234

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  60
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  122
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  166

#define YYUNDEFTOK  2
#define YYMAXUTOK   314

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59
};

#if GRAM_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   294,   294,   303,   304,   308,   309,   315,   319,   324,
     325,   330,   331,   332,   333,   334,   339,   344,   345,   346,
     347,   348,   349,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   361,   362,   371,   372,   376,   387,   391,   395,
     403,   413,   414,   424,   425,   431,   444,   444,   449,   449,
     454,   458,   468,   469,   470,   471,   475,   476,   481,   482,
     486,   487,   491,   492,   493,   506,   515,   519,   523,   531,
     532,   536,   549,   550,   562,   566,   570,   578,   580,   585,
     592,   602,   606,   610,   618,   619,   627,   628,   634,   635,
     636,   643,   643,   651,   652,   653,   658,   661,   663,   665,
     667,   669,   671,   673,   675,   677,   682,   683,   692,   716,
     717,   718,   719,   731,   733,   748,   753,   754,   759,   768,
     769,   773,   774
};
#endif

#if GRAM_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"string\"", "\"%token\"",
  "\"%nterm\"", "\"%type\"", "\"%destructor\"", "\"%printer\"",
  "\"%left\"", "\"%right\"", "\"%nonassoc\"", "\"%precedence\"",
  "\"%prec\"", "\"%dprec\"", "\"%merge\"", "\"%code\"",
  "\"%default-prec\"", "\"%define\"", "\"%defines\"", "\"%error-verbose\"",
  "\"%expect\"", "\"%expect-rr\"", "\"%<flag>\"", "\"%file-prefix\"",
  "\"%glr-parser\"", "\"%initial-action\"", "\"%language\"",
  "\"%name-prefix\"", "\"%no-default-prec\"", "\"%no-lines\"",
  "\"%nondeterministic-parser\"", "\"%output\"", "\"%pure-parser\"",
  "\"%require\"", "\"%skeleton\"", "\"%start\"", "\"%token-table\"",
  "\"%verbose\"", "\"%yacc\"", "\"{...}\"", "\"%?{...}\"",
  "\"[identifier]\"", "\"char\"", "\":\"", "\"epilogue\"", "\"=\"",
  "\"identifier\"", "\"identifier:\"", "\"%%\"", "\"|\"", "\"%{...%}\"",
  "\";\"", "\"<tag>\"", "\"<*>\"", "\"<>\"", "\"integer\"", "\"%param\"",
  "\"%union\"", "\"%empty\"", "$accept", "input", "prologue_declarations",
  "prologue_declaration", "$@1", "params", "grammar_declaration",
  "code_props_type", "union_name", "symbol_declaration", "$@2", "$@3",
  "precedence_declarator", "tag.opt", "generic_symlist",
  "generic_symlist_item", "tag", "nterm_decls", "token_decls",
  "token_decl.1", "token_decl", "int.opt", "token_decls_for_prec",
  "token_decl_for_prec.1", "token_decl_for_prec", "symbol_decls",
  "symbol_decl.1", "grammar", "rules_or_grammar_declaration", "rules",
  "$@4", "rhses.1", "rhs", "named_ref.opt", "variable", "value", "id",
  "id_colon", "symbol", "string_as_id", "string_as_id.opt", "epilogue.opt", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314
};
# endif

#define YYPACT_NINF -130

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-130)))

#define YYTABLE_NINF -122

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -130,    36,   110,  -130,   -22,  -130,  -130,     2,  -130,  -130,
    -130,  -130,  -130,  -130,   -19,  -130,    -9,    40,  -130,   -17,
      -2,  -130,    57,  -130,    21,    66,    77,  -130,  -130,  -130,
      78,  -130,    87,    92,    44,  -130,  -130,  -130,   165,  -130,
    -130,  -130,    50,  -130,  -130,    58,  -130,    29,  -130,    15,
      15,  -130,  -130,  -130,    44,    47,    44,  -130,  -130,  -130,
    -130,    61,  -130,    30,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,    51,  -130,    53,     8,  -130,
    -130,    64,    67,  -130,    68,    20,    44,    56,    44,  -130,
      69,  -130,   -37,    59,   -37,  -130,    69,  -130,    59,    44,
      44,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,    79,
    -130,  -130,  -130,  -130,  -130,   111,  -130,  -130,  -130,  -130,
      20,  -130,  -130,  -130,    44,    44,  -130,  -130,  -130,   -37,
     -37,  -130,   147,    44,  -130,   108,  -130,  -130,    44,   -37,
    -130,  -130,  -130,   -21,   175,  -130,  -130,    44,    97,   101,
      99,   100,  -130,  -130,  -130,   117,    64,   175,  -130,  -130,
    -130,  -130,  -130,    64,  -130,  -130
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,    48,    46,     0,    41,    42,
      52,    53,    54,    55,     0,    37,     0,     9,    11,     0,
       0,     7,     0,    15,     0,     0,     0,    38,    19,    20,
       0,    24,     0,     0,     0,    27,    28,    29,     0,     6,
      31,    22,    43,     4,     5,     0,    34,     0,    30,     0,
       0,   118,   114,   113,     0,    50,    81,   116,    84,   117,
      39,     0,   108,   109,    10,    12,    13,    14,    16,    17,
      18,    21,    25,    26,    35,     0,   115,     0,     0,    86,
      88,   106,     0,    44,     0,     0,     0,    51,    74,    77,
      72,    80,     0,    49,    66,    69,    72,    47,    65,    82,
       0,    85,    40,   111,   112,   110,     8,    90,    89,     0,
      87,     2,   107,    91,    33,    23,    45,    62,    63,    64,
      36,    58,    61,    60,    75,     0,    78,    73,    79,    67,
       0,    70,   119,    83,   122,     0,    32,    59,    76,    68,
     120,    71,    96,    92,    93,    96,    95,     0,     0,     0,
       0,     0,    99,    57,   100,     0,   106,    94,   101,   102,
     103,   104,   105,   106,    97,    98
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -130,  -130,  -130,  -130,  -130,  -130,   156,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,    43,  -130,  -130,   114,   -66,
     -35,    83,  -130,   -84,   -53,  -130,   -47,  -130,    82,  -130,
    -130,  -130,    35,  -129,  -130,  -130,   -46,  -130,   -34,   -36,
    -130,  -130
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    43,    82,   115,    77,    45,    84,    46,
      50,    49,    47,   155,   120,   121,   122,    97,    93,    94,
      95,   128,    87,    88,    89,    55,    56,    78,    79,    80,
     135,   143,   144,   113,    63,   106,    57,    81,    58,    59,
     141,   111
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      74,    90,   124,    96,    96,    51,    52,    99,  -121,    75,
      53,    91,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    60,   101,    51,    14,    15,   129,   164,    61,   145,
      48,   146,    51,   103,   165,   126,     3,    27,    62,    65,
      90,   138,    90,    64,    34,    52,    96,    51,    96,    53,
      91,   123,    91,   133,    66,    54,    76,   109,    52,   131,
      67,    68,    53,    52,   139,   101,    42,    53,    92,    69,
     104,   126,    52,   117,   118,   119,    53,   105,    90,    90,
      70,    71,    86,    96,    96,   126,   123,    52,    91,    91,
      72,    53,    90,    96,   131,    73,   140,    83,    85,   101,
     100,   102,    91,   107,   131,   108,   112,   114,   116,   125,
     156,     4,   130,   158,     5,     6,     7,     8,     9,    10,
      11,    12,    13,   156,   134,   127,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      51,   136,   142,   159,   160,   161,   162,   163,    44,    38,
     110,    39,    40,   137,    98,     0,    75,    41,    42,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    51,   132,
     157,    14,    15,     0,     0,     0,     0,     0,   147,   148,
     149,     0,     0,     0,    27,     0,   150,   151,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    76,     0,   -56,   152,     0,    52,     0,
       0,     0,    53,    42,     0,     0,     0,     0,   153,     0,
       0,     0,     0,     0,   154
};

static const yytype_int16 yycheck[] =
{
      34,    47,    86,    49,    50,     3,    43,    54,     0,     1,
      47,    47,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    40,    56,     3,    16,    17,    92,   156,    47,    50,
      52,    52,     3,     3,   163,    88,     0,    29,    47,    56,
      86,   125,    88,     3,    36,    43,    92,     3,    94,    47,
      86,    85,    88,   100,    56,    53,    48,    49,    43,    94,
       3,    40,    47,    43,   130,    99,    58,    47,    53,     3,
      40,   124,    43,    53,    54,    55,    47,    47,   124,   125,
       3,     3,    53,   129,   130,   138,   120,    43,   124,   125,
       3,    47,   138,   139,   129,     3,   132,    47,    40,   133,
      53,    40,   138,    52,   139,    52,    42,    40,    40,    53,
     144,     1,    53,   147,     4,     5,     6,     7,     8,     9,
      10,    11,    12,   157,    45,    56,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
       3,    40,    44,    56,    53,    56,    56,    40,     2,    49,
      78,    51,    52,   120,    50,    -1,     1,    57,    58,     4,
       5,     6,     7,     8,     9,    10,    11,    12,     3,    96,
     145,    16,    17,    -1,    -1,    -1,    -1,    -1,    13,    14,
      15,    -1,    -1,    -1,    29,    -1,    21,    22,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    40,    41,    -1,    43,    -1,
      -1,    -1,    47,    58,    -1,    -1,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    59
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    61,    62,     0,     1,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    49,    51,
      52,    57,    58,    63,    66,    67,    69,    72,    52,    71,
      70,     3,    43,    47,    53,    85,    86,    96,    98,    99,
      40,    47,    47,    94,     3,    56,    56,     3,    40,     3,
       3,     3,     3,     3,    98,     1,    48,    66,    87,    88,
      89,    97,    64,    47,    68,    40,    53,    82,    83,    84,
      96,    99,    53,    78,    79,    80,    96,    77,    78,    86,
      53,    98,    40,     3,    40,    47,    95,    52,    52,    49,
      88,   101,    42,    93,    40,    65,    40,    53,    54,    55,
      74,    75,    76,    98,    83,    53,    84,    56,    81,    79,
      53,    80,    81,    86,    45,    90,    40,    75,    83,    79,
      99,   100,    44,    91,    92,    50,    52,    13,    14,    15,
      21,    22,    41,    53,    59,    73,    98,    92,    98,    56,
      53,    56,    56,    40,    93,    93
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    60,    61,    62,    62,    63,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    64,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    67,    67,    68,    68,    66,    70,    69,    71,    69,
      69,    69,    72,    72,    72,    72,    73,    73,    74,    74,
      75,    75,    76,    76,    76,    77,    78,    78,    78,    79,
      79,    80,    81,    81,    82,    82,    82,    83,    83,    84,
      84,    85,    85,    85,    86,    86,    87,    87,    88,    88,
      88,    90,    89,    91,    91,    91,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    93,    93,    94,    95,
      95,    95,    95,    96,    96,    97,    98,    98,    99,   100,
     100,   101,   101
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     0,     2,     1,     1,     1,     3,     1,
       2,     1,     2,     2,     2,     1,     2,     2,     2,     1,
       1,     2,     0,     3,     1,     2,     2,     1,     1,     1,
       2,     1,     2,     1,     1,     2,     3,     1,     1,     2,
       3,     1,     1,     0,     1,     3,     0,     3,     0,     3,
       2,     2,     1,     1,     1,     1,     0,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     1,
       2,     3,     0,     1,     1,     2,     3,     1,     2,     2,
       1,     1,     2,     3,     1,     2,     1,     2,     1,     2,
       2,     0,     5,     1,     3,     2,     0,     3,     4,     2,
       2,     3,     3,     3,     3,     3,     0,     1,     1,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     0,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        YY_LAC_DISCARD ("YYBACKUP");                              \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if GRAM_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined GRAM_LTYPE_IS_TRIVIAL && GRAM_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
/* "%code pre-printer" blocks.  */
tron (yyo);

  switch (yytype)
    {
    case 3: /* "string"  */
      { fputs (((*yyvaluep).STRING), yyo); }
        break;

    case 20: /* "%error-verbose"  */
      { fputs (((*yyvaluep).PERCENT_ERROR_VERBOSE), yyo); }
        break;

    case 23: /* "%<flag>"  */
      { fprintf (yyo, "%%%s", ((*yyvaluep).PERCENT_FLAG)); }
        break;

    case 24: /* "%file-prefix"  */
      { fputs (((*yyvaluep).PERCENT_FILE_PREFIX), yyo); }
        break;

    case 28: /* "%name-prefix"  */
      { fputs (((*yyvaluep).PERCENT_NAME_PREFIX), yyo); }
        break;

    case 33: /* "%pure-parser"  */
      { fputs (((*yyvaluep).PERCENT_PURE_PARSER), yyo); }
        break;

    case 39: /* "%yacc"  */
      { fputs (((*yyvaluep).PERCENT_YACC), yyo); }
        break;

    case 40: /* "{...}"  */
      { fputs (((*yyvaluep).BRACED_CODE), yyo); }
        break;

    case 41: /* "%?{...}"  */
      { fputs (((*yyvaluep).BRACED_PREDICATE), yyo); }
        break;

    case 42: /* "[identifier]"  */
      { fprintf (yyo, "[%s]", ((*yyvaluep).BRACKETED_ID)); }
        break;

    case 43: /* "char"  */
      { fputs (char_name (((*yyvaluep).CHAR)), yyo); }
        break;

    case 45: /* "epilogue"  */
      { fputs (((*yyvaluep).EPILOGUE), yyo); }
        break;

    case 47: /* "identifier"  */
      { fputs (((*yyvaluep).ID), yyo); }
        break;

    case 48: /* "identifier:"  */
      { fprintf (yyo, "%s:", ((*yyvaluep).ID_COLON)); }
        break;

    case 51: /* "%{...%}"  */
      { fputs (((*yyvaluep).PROLOGUE), yyo); }
        break;

    case 53: /* "<tag>"  */
      { fprintf (yyo, "<%s>", ((*yyvaluep).TAG)); }
        break;

    case 56: /* "integer"  */
      { fprintf (yyo, "%d", ((*yyvaluep).INT)); }
        break;

    case 57: /* "%param"  */
      {
  switch (((*yyvaluep).PERCENT_PARAM))
    {
#define CASE(In, Out)                                           \
      case param_ ## In: fputs ("%" #Out, yyo); break
      CASE (lex,   lex-param);
      CASE (parse, parse-param);
      CASE (both,  param);
#undef CASE
      case param_none: aver (false); break;
    }
}
        break;

    case 67: /* code_props_type  */
      { fprintf (yyo, "%s", code_props_type_string (((*yyvaluep).code_props_type))); }
        break;

    case 73: /* tag.opt  */
      { fputs (((*yyvaluep).yytype_73), yyo); }
        break;

    case 74: /* generic_symlist  */
      { symbol_list_syms_print (((*yyvaluep).generic_symlist), yyo); }
        break;

    case 75: /* generic_symlist_item  */
      { symbol_list_syms_print (((*yyvaluep).generic_symlist_item), yyo); }
        break;

    case 76: /* tag  */
      { fprintf (yyo, "<%s>", ((*yyvaluep).tag)); }
        break;

    case 77: /* nterm_decls  */
      { symbol_list_syms_print (((*yyvaluep).nterm_decls), yyo); }
        break;

    case 78: /* token_decls  */
      { symbol_list_syms_print (((*yyvaluep).token_decls), yyo); }
        break;

    case 79: /* token_decl.1  */
      { symbol_list_syms_print (((*yyvaluep).yytype_79), yyo); }
        break;

    case 80: /* token_decl  */
      { fprintf (yyo, "%s", ((*yyvaluep).token_decl) ? ((*yyvaluep).token_decl)->tag : "<NULL>"); }
        break;

    case 81: /* int.opt  */
      { fprintf (yyo, "%d", ((*yyvaluep).yytype_81)); }
        break;

    case 82: /* token_decls_for_prec  */
      { symbol_list_syms_print (((*yyvaluep).token_decls_for_prec), yyo); }
        break;

    case 83: /* token_decl_for_prec.1  */
      { symbol_list_syms_print (((*yyvaluep).yytype_83), yyo); }
        break;

    case 84: /* token_decl_for_prec  */
      { fprintf (yyo, "%s", ((*yyvaluep).token_decl_for_prec) ? ((*yyvaluep).token_decl_for_prec)->tag : "<NULL>"); }
        break;

    case 85: /* symbol_decls  */
      { symbol_list_syms_print (((*yyvaluep).symbol_decls), yyo); }
        break;

    case 86: /* symbol_decl.1  */
      { symbol_list_syms_print (((*yyvaluep).yytype_86), yyo); }
        break;

    case 94: /* variable  */
      { fputs (((*yyvaluep).variable), yyo); }
        break;

    case 95: /* value  */
      {
  switch (((*yyvaluep).value).kind)
    {
    case muscle_code:    fprintf (yyo,  "{%s}",  ((*yyvaluep).value).chars); break;
    case muscle_keyword: fprintf (yyo,   "%s",   ((*yyvaluep).value).chars); break;
    case muscle_string:  fprintf (yyo, "\"%s\"", ((*yyvaluep).value).chars); break;
    }
}
        break;

    case 96: /* id  */
      { fprintf (yyo, "%s", ((*yyvaluep).id) ? ((*yyvaluep).id)->tag : "<NULL>"); }
        break;

    case 97: /* id_colon  */
      { fprintf (yyo, "%s:", ((*yyvaluep).id_colon)->tag); }
        break;

    case 98: /* symbol  */
      { fprintf (yyo, "%s", ((*yyvaluep).symbol) ? ((*yyvaluep).symbol)->tag : "<NULL>"); }
        break;

    case 99: /* string_as_id  */
      { fprintf (yyo, "%s", ((*yyvaluep).string_as_id) ? ((*yyvaluep).string_as_id)->tag : "<NULL>"); }
        break;

    case 100: /* string_as_id.opt  */
      { fprintf (yyo, "%s", ((*yyvaluep).yytype_100) ? ((*yyvaluep).yytype_100)->tag : "<NULL>"); }
        break;

      default:
        break;
    }
/* "%code post-printer" blocks.  */
troff (yyo);

}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !GRAM_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !GRAM_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return 1 if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYSIZE_T *yycapacity, YYSIZE_T yyadd,
#if GRAM_DEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yytype_int16 **yybottom,
                      yytype_int16 *yybottom_no_free,
                      yytype_int16 **yytop, yytype_int16 *yytop_empty)
{
  YYSIZE_T yysize_old =
    (YYSIZE_T) (*yytop == yytop_empty ? 0 : *yytop - *yybottom + 1);
  YYSIZE_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYSIZE_T yyalloc = 2 * yysize_new;
      yytype_int16 *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return 1;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        (yytype_int16*) YYSTACK_ALLOC (yyalloc * sizeof *yybottom_new);
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return 1;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                         \
do {                                                             \
  if (!yy_lac_established)                                       \
    {                                                            \
      YYDPRINTF ((stderr,                                        \
                  "LAC: initial context established for %s\n",   \
                  yytname[yytoken]));                            \
      yy_lac_established = 1;                                    \
      {                                                          \
        int yy_lac_status =                                      \
          yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken); \
        if (yy_lac_status == 2)                                  \
          goto yyexhaustedlab;                                   \
        if (yy_lac_status == 1)                                  \
          goto yyerrlab;                                         \
      }                                                          \
    }                                                            \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if GRAM_DEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      if (yydebug)                                                       \
        YYFPRINTF (stderr, "LAC: initial context discarded due to "      \
                   Event "\n");                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return 2 if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yytype_int16 *yyesa, yytype_int16 **yyes,
        YYSIZE_T *yyes_capacity, yytype_int16 *yyssp, int yytoken)
{
  yytype_int16 *yyes_prev = yyssp;
  yytype_int16 *yyesp = yyes_prev;
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yytname[yytoken]));
  if (yytoken == YYUNDEFTOK)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          yyrule = yydefact[*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      {
        YYSIZE_T yylen = yyr2[yyrule];
        YYDPRINTF ((stderr, " R%d", yyrule - 1));
        if (yyesp != yyes_prev)
          {
            YYSIZE_T yysize = (YYSIZE_T) (yyesp - *yyes + 1);
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yylen -= yysize;
                yyesp = yyes_prev;
              }
          }
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      {
        yytype_int16 yystate;
        {
          const int yylhs = yyr1[yyrule] - YYNTOKENS;
          const int yyi = yypgoto[yylhs] + *yyesp;
          yystate = ((yytype_int16)
                     (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyesp
                      ? yytable[yyi]
                      : yydefgoto[yylhs]));
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            *yyesp = yystate;
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if GRAM_DEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return 2;
              }
            *++yyesp = yystate;
          }
        YYDPRINTF ((stderr, " G%d", (int) yystate));
      }
    }
}


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.  In order to see if a particular token T is a
   valid looakhead, invoke yy_lac (YYESA, YYES, YYES_CAPACITY, YYSSP, T).

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store or if
   yy_lac returned 2.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyesa, yytype_int16 **yyes,
                YYSIZE_T *yyes_capacity, yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      YYDPRINTF ((stderr, "Constructing syntax error message\n"));
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          int yyx;

          for (yyx = 0; yyx < YYNTOKENS; ++yyx)
            if (yyx != YYTERROR && yyx != YYUNDEFTOK)
              {
                {
                  int yy_lac_status = yy_lac (yyesa, yyes, yyes_capacity,
                                              yyssp, yyx);
                  if (yy_lac_status == 2)
                    return 2;
                  if (yy_lac_status == 1)
                    continue;
                }
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
# if GRAM_DEBUG
      else if (yydebug)
        YYFPRINTF (stderr, "No expected tokens.\n");
# endif
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
    case 74: /* generic_symlist  */
      { symbol_list_free (((*yyvaluep).generic_symlist)); }
        break;

    case 75: /* generic_symlist_item  */
      { symbol_list_free (((*yyvaluep).generic_symlist_item)); }
        break;

    case 77: /* nterm_decls  */
      { symbol_list_free (((*yyvaluep).nterm_decls)); }
        break;

    case 78: /* token_decls  */
      { symbol_list_free (((*yyvaluep).token_decls)); }
        break;

    case 79: /* token_decl.1  */
      { symbol_list_free (((*yyvaluep).yytype_79)); }
        break;

    case 82: /* token_decls_for_prec  */
      { symbol_list_free (((*yyvaluep).token_decls_for_prec)); }
        break;

    case 83: /* token_decl_for_prec.1  */
      { symbol_list_free (((*yyvaluep).yytype_83)); }
        break;

    case 85: /* symbol_decls  */
      { symbol_list_free (((*yyvaluep).symbol_decls)); }
        break;

    case 86: /* symbol_decl.1  */
      { symbol_list_free (((*yyvaluep).yytype_86)); }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined GRAM_LTYPE_IS_TRIVIAL && GRAM_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

    yytype_int16 yyesa[20];
    yytype_int16 *yyes;
    YYSIZE_T yyes_capacity;

  int yy_lac_established = 0;
  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  yyes = yyesa;
  yyes_capacity = sizeof yyesa / sizeof *yyes;
  if (YYMAXDEPTH < yyes_capacity)
    yyes_capacity = YYMAXDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
{
  /* Bison's grammar can initial empty locations, hence a default
     location is needed. */
  boundary_set (&yylloc.start, current_file, 1, 1, 1);
  boundary_set (&yylloc.end, current_file, 1, 1, 1);
}


  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      YY_LAC_ESTABLISH;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  YY_LAC_DISCARD ("shift");

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
  case 6:
    {
      muscle_code_grow (union_seen ? "post_prologue" : "pre_prologue",
                        translate_code ((yyvsp[0].PROLOGUE), (yylsp[0]), true), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;

  case 7:
    {
      muscle_percent_define_ensure ((yyvsp[0].PERCENT_FLAG), (yylsp[0]), true);
    }
    break;

  case 8:
    {
      muscle_percent_define_insert ((yyvsp[-1].variable), (yyloc), (yyvsp[0].value).kind, (yyvsp[0].value).chars,
                                    MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
    }
    break;

  case 9:
    { defines_flag = true; }
    break;

  case 10:
    {
      defines_flag = true;
      spec_header_file = xstrdup ((yyvsp[0].STRING));
    }
    break;

  case 11:
    { handle_error_verbose (&(yyloc), (yyvsp[0].PERCENT_ERROR_VERBOSE)); }
    break;

  case 12:
    { expected_sr_conflicts = (yyvsp[0].INT); }
    break;

  case 13:
    { expected_rr_conflicts = (yyvsp[0].INT); }
    break;

  case 14:
    { handle_file_prefix (&(yyloc), &(yylsp[-1]), (yyvsp[-1].PERCENT_FILE_PREFIX), (yyvsp[0].STRING)); }
    break;

  case 15:
    {
      nondeterministic_parser = true;
      glr_parser = true;
    }
    break;

  case 16:
    {
      muscle_code_grow ("initial_action", translate_code ((yyvsp[0].BRACED_CODE), (yylsp[0]), false), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;

  case 17:
    { language_argmatch ((yyvsp[0].STRING), grammar_prio, (yylsp[-1])); }
    break;

  case 18:
    { handle_name_prefix (&(yyloc), (yyvsp[-1].PERCENT_NAME_PREFIX), (yyvsp[0].STRING)); }
    break;

  case 19:
    { no_lines_flag = true; }
    break;

  case 20:
    { nondeterministic_parser = true; }
    break;

  case 21:
    { spec_outfile = (yyvsp[0].STRING); }
    break;

  case 22:
    { current_param = (yyvsp[0].PERCENT_PARAM); }
    break;

  case 23:
    { current_param = param_none; }
    break;

  case 24:
    { handle_pure_parser (&(yyloc), (yyvsp[0].PERCENT_PURE_PARSER)); }
    break;

  case 25:
    { handle_require (&(yylsp[0]), (yyvsp[0].STRING)); }
    break;

  case 26:
    { handle_skeleton (&(yylsp[0]), (yyvsp[0].STRING)); }
    break;

  case 27:
    { token_table_flag = true; }
    break;

  case 28:
    { report_flag |= report_states; }
    break;

  case 29:
    { handle_yacc (&(yyloc), (yyvsp[0].PERCENT_YACC)); }
    break;

  case 30:
    { current_class = unknown_sym; yyerrok; }
    break;

  case 32:
    { add_param (current_param, (yyvsp[0].BRACED_CODE), (yylsp[0])); }
    break;

  case 33:
    { add_param (current_param, (yyvsp[0].BRACED_CODE), (yylsp[0])); }
    break;

  case 35:
    {
      grammar_start_symbol_set ((yyvsp[0].symbol), (yylsp[0]));
    }
    break;

  case 36:
    {
      code_props code;
      code_props_symbol_action_init (&code, (yyvsp[-1].BRACED_CODE), (yylsp[-1]));
      code_props_translate_code (&code);
      {
        for (symbol_list *list = (yyvsp[0].generic_symlist); list; list = list->next)
          symbol_list_code_props_set (list, (yyvsp[-2].code_props_type), &code);
        symbol_list_free ((yyvsp[0].generic_symlist));
      }
    }
    break;

  case 37:
    {
      default_prec = true;
    }
    break;

  case 38:
    {
      default_prec = false;
    }
    break;

  case 39:
    {
      /* Do not invoke muscle_percent_code_grow here since it invokes
         muscle_user_name_list_grow.  */
      muscle_code_grow ("percent_code()",
                        translate_code_braceless ((yyvsp[0].BRACED_CODE), (yylsp[0])), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;

  case 40:
    {
      muscle_percent_code_grow ((yyvsp[-1].ID), (yylsp[-1]), translate_code_braceless ((yyvsp[0].BRACED_CODE), (yylsp[0])), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;

  case 41:
    { (yyval.code_props_type) = destructor; }
    break;

  case 42:
    { (yyval.code_props_type) = printer; }
    break;

  case 43:
    {}
    break;

  case 44:
    { muscle_percent_define_insert ("api.value.union.name",
                                         (yylsp[0]), muscle_keyword, (yyvsp[0].ID),
                                         MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE); }
    break;

  case 45:
    {
      union_seen = true;
      muscle_code_grow ("union_members", translate_code_braceless ((yyvsp[0].BRACED_CODE), (yylsp[0])), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;

  case 46:
    { current_class = nterm_sym; }
    break;

  case 47:
    {
      current_class = unknown_sym;
      symbol_list_free ((yyvsp[0].nterm_decls));
    }
    break;

  case 48:
    { current_class = token_sym; }
    break;

  case 49:
    {
      current_class = unknown_sym;
      symbol_list_free ((yyvsp[0].token_decls));
    }
    break;

  case 50:
    {
      symbol_list_free ((yyvsp[0].symbol_decls));
    }
    break;

  case 51:
    {
      ++current_prec;
      for (symbol_list *list = (yyvsp[0].token_decls_for_prec); list; list = list->next)
        symbol_precedence_set (list->content.sym, current_prec, (yyvsp[-1].precedence_declarator), (yylsp[-1]));
      symbol_list_free ((yyvsp[0].token_decls_for_prec));
    }
    break;

  case 52:
    { (yyval.precedence_declarator) = left_assoc; }
    break;

  case 53:
    { (yyval.precedence_declarator) = right_assoc; }
    break;

  case 54:
    { (yyval.precedence_declarator) = non_assoc; }
    break;

  case 55:
    { (yyval.precedence_declarator) = precedence_assoc; }
    break;

  case 56:
    { (yyval.yytype_73) = NULL; }
    break;

  case 57:
    { (yyval.yytype_73) = (yyvsp[0].TAG); }
    break;

  case 59:
    { (yyval.generic_symlist) = symbol_list_append ((yyvsp[-1].generic_symlist), (yyvsp[0].generic_symlist_item)); }
    break;

  case 60:
    { (yyval.generic_symlist_item) = symbol_list_sym_new ((yyvsp[0].symbol), (yylsp[0])); }
    break;

  case 61:
    { (yyval.generic_symlist_item) = symbol_list_type_new ((yyvsp[0].tag), (yylsp[0])); }
    break;

  case 63:
    { (yyval.tag) = uniqstr_new ("*"); }
    break;

  case 64:
    { (yyval.tag) = uniqstr_new (""); }
    break;

  case 66:
    {
      (yyval.token_decls) = (yyvsp[0].yytype_79);
    }
    break;

  case 67:
    {
      (yyval.token_decls) = symbol_list_type_set ((yyvsp[0].yytype_79), (yyvsp[-1].TAG), (yylsp[-1]));
    }
    break;

  case 68:
    {
      (yyval.token_decls) = symbol_list_append ((yyvsp[-2].token_decls), symbol_list_type_set ((yyvsp[0].yytype_79), (yyvsp[-1].TAG), (yylsp[-1])));
    }
    break;

  case 69:
    { (yyval.yytype_79) = symbol_list_sym_new ((yyvsp[0].token_decl), (yylsp[0])); }
    break;

  case 70:
    { (yyval.yytype_79) = symbol_list_append ((yyvsp[-1].yytype_79), symbol_list_sym_new ((yyvsp[0].token_decl), (yylsp[0]))); }
    break;

  case 71:
    {
      (yyval.token_decl) = (yyvsp[-2].id);
      symbol_class_set ((yyvsp[-2].id), current_class, (yylsp[-2]), true);
      if (0 <= (yyvsp[-1].yytype_81))
        symbol_user_token_number_set ((yyvsp[-2].id), (yyvsp[-1].yytype_81), (yylsp[-1]));
      if ((yyvsp[0].yytype_100))
        symbol_make_alias ((yyvsp[-2].id), (yyvsp[0].yytype_100), (yylsp[0]));
    }
    break;

  case 72:
    { (yyval.yytype_81) = -1; }
    break;

  case 74:
    {
      (yyval.token_decls_for_prec) = (yyvsp[0].yytype_83);
    }
    break;

  case 75:
    {
      (yyval.token_decls_for_prec) = symbol_list_type_set ((yyvsp[0].yytype_83), (yyvsp[-1].TAG), (yylsp[-1]));
    }
    break;

  case 76:
    {
      (yyval.token_decls_for_prec) = symbol_list_append ((yyvsp[-2].token_decls_for_prec), symbol_list_type_set ((yyvsp[0].yytype_83), (yyvsp[-1].TAG), (yylsp[-1])));
    }
    break;

  case 77:
    { (yyval.yytype_83) = symbol_list_sym_new ((yyvsp[0].token_decl_for_prec), (yylsp[0])); }
    break;

  case 78:
    { (yyval.yytype_83) = symbol_list_append ((yyvsp[-1].yytype_83), symbol_list_sym_new ((yyvsp[0].token_decl_for_prec), (yylsp[0]))); }
    break;

  case 79:
    {
      (yyval.token_decl_for_prec) = (yyvsp[-1].id);
      symbol_class_set ((yyvsp[-1].id), token_sym, (yylsp[-1]), false);
      if (0 <= (yyvsp[0].yytype_81))
        symbol_user_token_number_set ((yyvsp[-1].id), (yyvsp[0].yytype_81), (yylsp[0]));
    }
    break;

  case 81:
    {
      (yyval.symbol_decls) = (yyvsp[0].yytype_86);
    }
    break;

  case 82:
    {
      (yyval.symbol_decls) = symbol_list_type_set ((yyvsp[0].yytype_86), (yyvsp[-1].TAG), (yylsp[-1]));
    }
    break;

  case 83:
    {
      (yyval.symbol_decls) = symbol_list_append ((yyvsp[-2].symbol_decls), symbol_list_type_set ((yyvsp[0].yytype_86), (yyvsp[-1].TAG), (yylsp[-1])));
    }
    break;

  case 84:
    { (yyval.yytype_86) = symbol_list_sym_new ((yyvsp[0].symbol), (yylsp[0])); }
    break;

  case 85:
    { (yyval.yytype_86) = symbol_list_append ((yyvsp[-1].yytype_86), symbol_list_sym_new ((yyvsp[0].symbol), (yylsp[0]))); }
    break;

  case 90:
    {
      yyerrok;
    }
    break;

  case 91:
    { current_lhs ((yyvsp[-1].id_colon), (yylsp[-1]), (yyvsp[0].yytype_93)); }
    break;

  case 92:
    {
      /* Free the current lhs. */
      current_lhs (0, (yylsp[-4]), 0);
    }
    break;

  case 93:
    { grammar_current_rule_end ((yylsp[0])); }
    break;

  case 94:
    { grammar_current_rule_end ((yylsp[0])); }
    break;

  case 96:
    { grammar_current_rule_begin (current_lhs_symbol, current_lhs_loc,
                                  current_lhs_named_ref); }
    break;

  case 97:
    { grammar_current_rule_symbol_append ((yyvsp[-1].symbol), (yylsp[-1]), (yyvsp[0].yytype_93)); }
    break;

  case 98:
    { grammar_current_rule_action_append ((yyvsp[-1].BRACED_CODE), (yylsp[-1]), (yyvsp[0].yytype_93), (yyvsp[-2].yytype_73)); }
    break;

  case 99:
    { grammar_current_rule_predicate_append ((yyvsp[0].BRACED_PREDICATE), (yylsp[0])); }
    break;

  case 100:
    { grammar_current_rule_empty_set ((yylsp[0])); }
    break;

  case 101:
    { grammar_current_rule_prec_set ((yyvsp[0].symbol), (yylsp[0])); }
    break;

  case 102:
    { grammar_current_rule_dprec_set ((yyvsp[0].INT), (yylsp[0])); }
    break;

  case 103:
    { grammar_current_rule_merge_set ((yyvsp[0].TAG), (yylsp[0])); }
    break;

  case 104:
    { grammar_current_rule_expect_sr ((yyvsp[0].INT), (yylsp[0])); }
    break;

  case 105:
    { grammar_current_rule_expect_rr ((yyvsp[0].INT), (yylsp[0])); }
    break;

  case 106:
    { (yyval.yytype_93) = NULL; }
    break;

  case 107:
    { (yyval.yytype_93) = named_ref_new ((yyvsp[0].BRACKETED_ID), (yylsp[0])); }
    break;

  case 109:
    { (yyval.value).kind = muscle_keyword; (yyval.value).chars = ""; }
    break;

  case 110:
    { (yyval.value).kind = muscle_keyword; (yyval.value).chars = (yyvsp[0].ID); }
    break;

  case 111:
    { (yyval.value).kind = muscle_string;  (yyval.value).chars = (yyvsp[0].STRING); }
    break;

  case 112:
    { (yyval.value).kind = muscle_code;    (yyval.value).chars = strip_braces ((yyvsp[0].BRACED_CODE)); }
    break;

  case 113:
    { (yyval.id) = symbol_from_uniqstr ((yyvsp[0].ID), (yylsp[0])); }
    break;

  case 114:
    {
      if (current_class == nterm_sym)
        {
          gram_error (&(yylsp[0]),
                      _("character literals cannot be nonterminals"));
          YYERROR;
        }
      (yyval.id) = symbol_get (char_name ((yyvsp[0].CHAR)), (yylsp[0]));
      symbol_class_set ((yyval.id), token_sym, (yylsp[0]), false);
      symbol_user_token_number_set ((yyval.id), (yyvsp[0].CHAR), (yylsp[0]));
    }
    break;

  case 115:
    { (yyval.id_colon) = symbol_from_uniqstr ((yyvsp[0].ID_COLON), (yylsp[0])); }
    break;

  case 118:
    {
      (yyval.string_as_id) = symbol_get (quotearg_style (c_quoting_style, (yyvsp[0].STRING)), (yylsp[0]));
      symbol_class_set ((yyval.string_as_id), token_sym, (yylsp[0]), false);
    }
    break;

  case 119:
    { (yyval.yytype_100) = NULL; }
    break;

  case 122:
    {
      muscle_code_grow ("epilogue", translate_code ((yyvsp[0].EPILOGUE), (yylsp[0]), true), (yylsp[0]));
      code_scanner_last_string_free ();
    }
    break;



        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
  }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyesa, &yyes, &yyes_capacity, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        if (yychar != YYEMPTY)
          YY_LAC_ESTABLISH;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}


/* Return the location of the left-hand side of a rule whose
   right-hand side is RHS[1] ... RHS[N].  Ignore empty nonterminals in
   the right-hand side, and return an empty location equal to the end
   boundary of RHS[0] if the right-hand side is empty.  */

static YYLTYPE
lloc_default (YYLTYPE const *rhs, int n)
{
  YYLTYPE loc;

  /* SGI MIPSpro 7.4.1m miscompiles "loc.start = loc.end = rhs[n].end;".
     The bug is fixed in 7.4.2m, but play it safe for now.  */
  loc.start = rhs[n].end;
  loc.end = rhs[n].end;

  /* Ignore empty nonterminals the start of the right-hand side.
     Do not bother to ignore them at the end of the right-hand side,
     since empty nonterminals have the same end as their predecessors.  */
  for (int i = 1; i <= n; i++)
    if (! equal_boundaries (rhs[i].start, rhs[i].end))
      {
        loc.start = rhs[i].start;
        break;
      }

  return loc;
}

static
char *strip_braces (char *code)
{
  code[strlen (code) - 1] = 0;
  return code + 1;
}

static
char const *
translate_code (char *code, location loc, bool plain)
{
  code_props plain_code;
  if (plain)
    code_props_plain_init (&plain_code, code, loc);
  else
    code_props_symbol_action_init (&plain_code, code, loc);
  code_props_translate_code (&plain_code);
  gram_scanner_last_string_free ();
  return plain_code.code;
}

static
char const *
translate_code_braceless (char *code, location loc)
{
  return translate_code (strip_braces (code), loc, true);
}

static void
add_param (param_type type, char *decl, location loc)
{
  static char const alphanum[26 + 26 + 1 + 10 + 1] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "_"
    "0123456789";

  char const *name_start = NULL;
  {
    char *p;
    /* Stop on last actual character.  */
    for (p = decl; p[1]; p++)
      if ((p == decl
           || ! memchr (alphanum, p[-1], sizeof alphanum - 1))
          && memchr (alphanum, p[0], sizeof alphanum - 10 - 1))
        name_start = p;

    /* Strip the surrounding '{' and '}', and any blanks just inside
       the braces.  */
    --p;
    while (c_isspace ((unsigned char) *p))
      --p;
    p[1] = '\0';
    ++decl;
    while (c_isspace ((unsigned char) *decl))
      ++decl;
  }

  if (! name_start)
    complain (&loc, complaint, _("missing identifier in parameter declaration"));
  else
    {
      char *name = xmemdup0 (name_start, strspn (name_start, alphanum));
      if (type & param_lex)
        muscle_pair_list_grow ("lex_param", decl, name);
      if (type & param_parse)
        muscle_pair_list_grow ("parse_param", decl, name);
      free (name);
    }

  gram_scanner_last_string_free ();
}


static void
handle_error_verbose (location const *loc, char const *directive)
{
  bison_directive (loc, directive);
  muscle_percent_define_insert (directive, *loc, muscle_keyword, "",
                                MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
}


static void
handle_file_prefix (location const *loc,
                    location const *dir_loc,
                    char const *directive, char const *value)
{
  bison_directive (loc, directive);
  bool warned = false;

  if (location_empty (spec_file_prefix_loc))
    {
      spec_file_prefix_loc = *loc;
      spec_file_prefix = value;
    }
  else
    {
      duplicate_directive (directive, spec_file_prefix_loc, *loc);
      warned = true;
    }

  if (!warned
      && STRNEQ (directive, "%file-prefix"))
    deprecated_directive (dir_loc, directive, "%file-prefix");
}


static void
handle_name_prefix (location const *loc,
                    char const *directive, char const *value)
{
  bison_directive (loc, directive);

  char buf1[1024];
  size_t len1 = sizeof (buf1);
  char *old = asnprintf (buf1, &len1, "%s\"%s\"", directive, value);
  if (!old)
    xalloc_die ();

  if (location_empty (spec_name_prefix_loc))
    {
      spec_name_prefix = value;
      spec_name_prefix_loc = *loc;

      char buf2[1024];
      size_t len2 = sizeof (buf2);
      char *new = asnprintf (buf2, &len2, "%%define api.prefix {%s}", value);
      if (!new)
        xalloc_die ();
      deprecated_directive (loc, old, new);
      if (new != buf2)
        free (new);
    }
  else
    duplicate_directive (old, spec_file_prefix_loc, *loc);

  if (old != buf1)
    free (old);
}


static void
handle_pure_parser (location const *loc, char const *directive)
{
  bison_directive (loc, directive);
  deprecated_directive (loc, directive, "%define api.pure");
  muscle_percent_define_insert ("api.pure", *loc, muscle_keyword, "",
                                MUSCLE_PERCENT_DEFINE_GRAMMAR_FILE);
}


static void
handle_require (location const *loc, char const *version)
{
  /* Changes of behavior are only on minor version changes, so "3.0.5"
     is the same as "3.0". */
  errno = 0;
  char* cp = NULL;
  unsigned long major = strtoul (version, &cp, 10);
  if (errno || *cp != '.')
    {
      complain (loc, complaint, _("invalid version requirement: %s"),
                version);
      return;
    }
  ++cp;
  unsigned long minor = strtoul (cp, NULL, 10);
  if (errno)
    {
      complain (loc, complaint, _("invalid version requirement: %s"),
                version);
      return;
    }
  required_version = major * 100 + minor;
  /* Pretend to be at least 3.4, to check features published in 3.4
     while developping it.  */
  const char* api_version = "3.4";
  const char* package_version =
    strverscmp (api_version, PACKAGE_VERSION) > 0
    ? api_version : PACKAGE_VERSION;
  if (strverscmp (version, package_version) > 0)
    {
      complain (loc, complaint, _("require bison %s, but have %s"),
                version, package_version);
      exit (EX_MISMATCH);
    }
}

static void
handle_skeleton (location const *loc, char const *skel)
{
  char const *skeleton_user = skel;
  if (strchr (skeleton_user, '/'))
    {
      size_t dir_length = strlen (current_file);
      while (dir_length && current_file[dir_length - 1] != '/')
        --dir_length;
      while (dir_length && current_file[dir_length - 1] == '/')
        --dir_length;
      char *skeleton_build =
        xmalloc (dir_length + 1 + strlen (skeleton_user) + 1);
      if (dir_length > 0)
        {
          memcpy (skeleton_build, current_file, dir_length);
          skeleton_build[dir_length++] = '/';
        }
      strcpy (skeleton_build + dir_length, skeleton_user);
      skeleton_user = uniqstr_new (skeleton_build);
      free (skeleton_build);
    }
  skeleton_arg (skeleton_user, grammar_prio, *loc);
}


static void
handle_yacc (location const *loc, char const *directive)
{
  bison_directive (loc, directive);
  bool warned = false;

  if (location_empty (yacc_loc))
    yacc_loc = *loc;
  else
    {
      duplicate_directive (directive, yacc_loc, *loc);
      warned = true;
    }

  if (!warned
      && STRNEQ (directive, "%fixed-output-files")
      && STRNEQ (directive, "%yacc"))
    deprecated_directive (loc, directive, "%fixed-output-files");
}


static void
gram_error (location const *loc, char const *msg)
{
  complain (loc, complaint, "%s", msg);
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

static
void
current_lhs (symbol *sym, location loc, named_ref *ref)
{
  current_lhs_symbol = sym;
  current_lhs_loc = loc;
  if (sym)
    symbol_location_as_lhs_set (sym, loc);
  /* In order to simplify memory management, named references for lhs
     are always assigned by deep copy into the current symbol_list
     node.  This is because a single named-ref in the grammar may
     result in several uses when the user factors lhs between several
     rules using "|".  Therefore free the parser's original copy.  */
  free (current_lhs_named_ref);
  current_lhs_named_ref = ref;
}

static void tron (FILE *yyo)
{
  begin_use_class ("value", yyo);
}

static void troff (FILE *yyo)
{
  end_use_class ("value", yyo);
}
