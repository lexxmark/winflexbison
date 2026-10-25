/* win_bison --update must keep the line endings of the grammar it rewrites.
   This file is stored with CRLF on purpose (see ../.gitattributes). Each
   deprecated directive below gets a fixit, so several lines change. */
%no_lines
%name-prefix "calc_"
%token A
%%
start : A ;
