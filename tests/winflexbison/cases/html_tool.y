/* CVE-2026-56389: a grammar must not be able to choose the program run by
   --html. If this %define still steered the tool, SENTINEL_TOOL would appear
   in win_bison's output; the fix always runs "xsltproc" from PATH instead. */
%define tool.xsltproc "SENTINEL_TOOL"
%token A
%%
start : A ;
