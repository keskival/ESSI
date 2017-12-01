/* Tero Keski-Valkama, 176842 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef char* word;
typedef int bool;

extern int stack_counter;

/* define TRUE and FALSE in case they are not defined */
#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

struct essi_module {
  char* (*get_mod_name)(void);   /* gives a name for module */
  bool (*known_word)(word);  /* returns true if module can interpret arg */
  bool (*interpret_word)(word, FILE*); /* interprets this word to stream */
  void* module_handle;
};

/* helper functions to print assembly statements */

extern int print_asm_newline(FILE* fp);

extern int print_asm0(FILE* fp, const char * opcode, const char* comment);

extern int print_asm1(FILE* fp, const char * opcode,const char* arg1, 
		      const char* comment);

extern int print_asm2(FILE* fp, const char * opcode,const char* arg1, 
		      const char* arg2, const char* comment);

extern int print_asm3(FILE* fp, const char * opcode, const char* arg1, 
		      const char* arg2 , const char* arg3, 
		      const char* comment);

extern int print_asm(FILE* fp, const char*);

extern void stack_overflow();
extern void stack_underflow();
