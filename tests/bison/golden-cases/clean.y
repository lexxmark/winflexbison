/* A clean, unambiguous grammar: win_bison must be silent (empty stderr, exit 0).
   Guards against spurious warnings/output on the happy path. */
%token NUM
%%
expr : NUM
     | expr '+' NUM
     ;
