/* Issue #62: win_bison --update must rewrite a grammar using deprecated
   syntax in place and leave a backup, without failing with "cannot backup:
   Permission denied" (Windows rename() semantics differ from POSIX). The
   underscore form %no_lines is deprecated in favor of %no-lines and is
   rewritten via bison's fixit mechanism (Wdeprecated is on by default). */
%no_lines
%token A
%%
start : A ;
