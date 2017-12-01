%{

#include "commands.h"

%}

%option noyywrap
%option nomain

%%
\%?   { return TOK_CMD_COMMENT; }
:l(oad)?         { return TOK_CMD_LOAD; }
:m(odules)?      { return TOK_CMD_LIST; }
:e(xit)?   { return TOK_CMD_EXIT; }
:q(uit)?   { return TOK_CMD_EXIT; }
:s(tack)?   { return TOK_CMD_STACK; }
\n         { return TOK_NEWLINE; }
:[ \t\n]   { return TOK_COLON; }
;[ \t\n]   { return TOK_SEMI; }
[ \t]+     { /* hum-de-dum */ }
[^:\" \t\n]+ { return TOK_WORD; }
<<EOF>>      { return TOK_EOF; }
%%
