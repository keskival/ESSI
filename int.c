/* Tero Keski-Valkama, 176842 */

#include "essi.h"
#include <math.h>
#include <stdlib.h>

char* get_module_name(void)
{
  return "int";
}


bool is_known_word(word w)
{
  int i=0;
  if ((strlen(w)>1)&&(w[0]=='-')) i=1;
  while( w[i] != 0 ) {
    if ( !isdigit( (int)w[i] ) ) { return FALSE; }
    i++;
  }
  return TRUE;
}


bool interpret(word w, FILE* f)
{
  long long int num=atoll(w);
  long long int num2=pow(2,31)-1;
  long long int num3=pow(2,31);
  if ((strlen(w)>11)||(num>=num2)||(num<-num3)) {
    printf("Numeric constant doesn't fit in 32 bits. Set to largest possible.(2147483647)\n");
    w="2147483647";
  }
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Int-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "set",w,"%l0","push constant");
  print_asm2(f, "cmp", "%g3", "1999", "");
  print_asm1(f, "be", temp_overflow, "");
  print_asm0(f, "nop", "delay");
  print_asm3(f, "add","%g3","1","%g3","adjust stack-size");
  print_asm3(f, "add","%g1","-4","%g1","adjust stack");
  print_asm2(f, "st","%l0","[%g1]","end push");

  print_asm1(f, "b", temp_over, "Jump over errorhandler");
  print_asm0(f, "nop", "delay");
  sprintf(temp_underflow,"local_stack_underflow_%d:",stack_counter);
  print_asm0(f, temp_underflow, "Error-handler");
  print_asm2(f, "mov", "-1", "%g3", "errorcode");
  print_asm1(f, "b", temp_over, "Jump over errorhandler");
  print_asm0(f, "nop", "delay");


  sprintf(temp_overflow,"local_stack_overflow_%d:",stack_counter);
  print_asm0(f, temp_overflow, "Error-handler");
  print_asm2(f, "mov", "-2", "%g3", "errorcode");
  sprintf(temp_over,"local_function_over_%d:",stack_counter);
  print_asm0(f, temp_over, "Function over");
  stack_counter++;


  return TRUE;
}
