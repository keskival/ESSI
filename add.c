/* Tero Keski-Valkama, 176842 */

#include "essi.h"

char* get_module_name(void)
{
  return "add";
}


bool is_known_word(word w)
{
  return (!strcmp("+",w)) || (!strcmp("-",w) || (!strcmp("neg",w)));
}


bool interpret(word w, FILE* f)
{
  int sub=0;
  int neg=0;
  char temp[256];  

  print_asm_newline(f);
  print_asm0(f, "", "Add-function");
  print_asm2(f, "cmp", "%g3", "0", "");
  sprintf(temp,"local_function_over_%d",stack_counter);  
  print_asm1(f, "bl", temp, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "cmp", "%g3", "0", "");
  sprintf(temp,"local_stack_underflow_%d",stack_counter);  
  print_asm1(f, "be", temp, "Error-handler");
  print_asm0(f, "nop", "delay");
  
  if (! strcmp("-",w)) { sub=1; }
  if (! strcmp("neg",w)) { neg=1; }
  print_asm2(f, "ld","[%g1]","%l0","add/sub/neg operation");
  if (!neg) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp, "Error-handler");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","");
  }
  if (sub) {
    print_asm3(f, "sub","%l1","%l0","%l1","actual subtract");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
  } else if (neg) {
    print_asm2(f, "neg","%l0","%l1","actual neg");
  } else {
    print_asm3(f, "add","%l1","%l0","%l1","actual add");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
  }
  print_asm2(f, "st","%l1","[%g1]","end add/sub/neg");

  sprintf(temp,"local_function_over_%d",stack_counter);  
  print_asm1(f, "b", temp, "Jump over errorhandler");
  print_asm0(f, "nop", "delay");
  sprintf(temp,"local_stack_underflow_%d:",stack_counter);  
  print_asm0(f, temp, "Error-handler");
  print_asm2(f, "mov", "-1", "%g3", "errorcode");
  sprintf(temp,"local_function_over_%d:",stack_counter);  
  print_asm0(f, temp, "Function over");
  stack_counter++;

  return TRUE;
}
