/* Ambiguous expression grammar (no precedence): shift/reduce conflicts.
   Exercises win_bison's conflict reporting. */
%token NUM
%%
expr : NUM
     | expr '+' expr
     | expr '*' expr
     ;
