/* 'unused' is not reachable from the start symbol: bison must warn about the
   useless nonterminal / rule. */
%token A B
%%
start  : A ;
unused : B ;
