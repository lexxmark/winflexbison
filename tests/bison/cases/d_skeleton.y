/* Issue #89: the D skeletons in bison 3.8.2 generate D that does not compile.

   Two separate bugs, both fixed upstream on 2022-09-20 and both reachable
   from this one grammar:

     * %code lexer made lalr1.d write "private class YYLexer implements
       Lexer {" -- "implements" is Java, D spells it ":". A parse error on
       the class declaration, so nothing after it compiles either.
       Upstream be45280, akimd/bison#84.

     * The SymbolKind.toString d.m4 emits took a template sink constrained
       by isOutputRange and wrote through put(), neither of which the
       generated file imports (it imports std.format and std.conv only).
       The template is only instantiated when something formats a
       SymbolKind, which "%define parse.error detailed" arranges: the error
       message builder calls format() on one. Upstream 0faf371,
       akimd/bison#88.

   The tests over this fixture check the generated text, so they run
   everywhere; if a D compiler happens to be installed they also compile it.
   See tests/bison/CMakeLists.txt. Body kept as small as upstream's own
   d.at fixtures -- the skeleton is what is under test, not the grammar. */
%language "D"
%define parse.error detailed
%token END "end"
%code lexer
{
  Symbol yylex () { return Symbol(); }
  void yyerror (string s) { import std.stdio; writeln(s); }
}
%%
start: END {};
%%
void main() {}
