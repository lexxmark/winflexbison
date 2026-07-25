/* References a symbol that is neither a token nor has any rule: bison must
   emit an error and exit non-zero. */
%%
start : undefined_symbol ;
