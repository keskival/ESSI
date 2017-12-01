/* Tero Keski-Valkama, 176842 */

#include "essi.h"

static int comp_counter = 0;

char* get_module_name(void)
{
  return "comp";
}


bool is_known_word(word w)
{
  return ((!strcmp("<",w)) || (!strcmp(">",w)) || (!strcmp(">=",w)) || (!strcmp("<=",w)));
}


bool interpret(word w, FILE* f)
{
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  char temp[256];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Comp-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  sprintf(temp,"comp_%d", comp_counter);
  print_asm2(f, "cmp", "%g3", "1", "");
  print_asm1(f, "ble", temp_underflow, "");
  print_asm0(f, "nop", "delay");
  print_asm2(f, "ld","[%g1]","%l0","comp operation");
  print_asm2(f, "ld","[%g1+4]","%l1","");
  print_asm3(f, "add","%g1","4","%g1","adjust stack");
  print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
  print_asm2(f, "cmp","%l0","%l1","Comparison");
  
  if (!strcmp("<",w)) print_asm1(f, "ble",temp,"jump over if comp false <");
  else if (!strcmp(">",w)) print_asm1(f, "bge",temp,"jump over if comp false >");
  else if (!strcmp("<=",w)) print_asm1(f, "bl",temp,"jump over if comp false <=");
  else if (!strcmp(">=",w)) print_asm1(f, "bg",temp,"jump over if comp false >=");
  
  print_asm1(f, "clr","[%g1]","Zero(delay)");
  print_asm2(f, "set","1","%l1","One");
  print_asm2(f, "st","%l1","[%g1]","");
  sprintf(temp,"comp_%d:", comp_counter++);
  print_asm0(f, temp,"end of comparison");

  print_asm1(f, "b", temp_over, "Jump over errorhandler");
  print_asm0(f, "nop", "delay");
  sprintf(temp_underflow,"local_stack_underflow_%d:",stack_counter);
  print_asm0(f, temp_underflow, "Error-handler");
  print_asm2(f, "mov", "-1", "%g3", "errorcode");
  sprintf(temp_over,"local_function_over_%d:",stack_counter);
  print_asm0(f, temp_over, "Function over");
  stack_counter++;


  return TRUE;  
}
