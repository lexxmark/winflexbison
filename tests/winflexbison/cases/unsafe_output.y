/* CVE-2026-56390: a grammar must not be able to write outside the output
   directory. Both directives below name a path one level up; win_bison must
   reject them, not create the files. */
%output "../pwned_out.c"
%header "../pwned_hdr.h"
%token A
%%
start : A ;
