/* Tero Keski-Valkama, 176842 */

#include "essi.h"

int stack_counter=0;

int print_asm_newline(FILE* fp)
{
  return fprintf(fp, "\n");
}

int print_asm0(FILE* fp, const char * opcode, const char* comment)
{
  return fprintf(fp, "\t%s         ! %s\n",opcode,comment);
}

int print_asm1(FILE* fp, const char * opcode,const char* arg1, 
	       const char* comment)
{
  return fprintf(fp, "\t%s\t%s  ! %s\n",opcode,arg1,comment);
}

int print_asm2(FILE* fp, const char * opcode,const char* arg1, 
	       const char* arg2, const char* comment)
{
  return fprintf(fp, "\t%s\t%s, %s  ! %s\n",opcode,arg1,arg2,comment);
}
  
int print_asm3(FILE* fp, const char * opcode,const char* arg1, 
	       const char* arg2, const char* arg3, const char* comment)
{
  return fprintf(fp, "\t%s\t%s, %s, %s ! %s\n",
		 opcode,arg1,arg2,arg3,comment);
}

int print_asm(FILE* fp, const char* text)
{
  return fprintf(fp, "%s\n", text);

}

int print_int(int a)
{
  return printf("%d ",a);
}

int put_char(char a)
{
  return putc(a,stdout);
}

int get_char()
{
  return getc(stdin);
}
