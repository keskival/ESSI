#define TOK_CMD_STACK 256
#define TOK_CMD_COMMENT 257
#define TOK_WORD 258
#define TOK_CMD_LOAD 259
#define TOK_CMD_LIST 260
#define TOK_CMD_EXIT 261
#define TOK_EOF 262
#define TOK_COLON 263
#define TOK_SEMI 264
#define TOK_NEWLINE 265

extern char* yytext;
extern int yylex(void);
