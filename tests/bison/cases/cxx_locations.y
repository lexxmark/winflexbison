/* Issue #10: %language "c++" + %locations previously failed to compile under
   MSVC (location.cc-equivalent arithmetic did `-static_cast<unsigned>(rhs)`,
   negating after the cast, which MSVC 2017 rejected; fixed by negating
   first, then casting). This exercises real location arithmetic (@1) to
   guard the fix, not just the presence of the generated location code. */
%language "c++"
%defines
%locations

%code {
#include <iostream>
int yylex(yy::parser::semantic_type* lvalp, yy::parser::location_type* lloc);
}

%define api.value.type {int}
%token NUM

%%
input:
    %empty
  | input line
  ;

line:
    NUM { std::cout << @1 << ": " << $1 << "\n"; }
  ;
%%

int yylex(yy::parser::semantic_type* lvalp, yy::parser::location_type* lloc)
{
    static int state = 0;
    lloc->begin.line = 1;
    lloc->begin.column = 1;
    lloc->end.line = 1;
    lloc->end.column = 2;
    if (state == 0)
    {
        *lvalp = 42;
        state = 1;
        return yy::parser::token::NUM;
    }
    return 0;
}

void yy::parser::error(const location_type& loc, const std::string& msg)
{
    std::cerr << loc << ": " << msg << "\n";
}
