/* Tero Keski-Valkama, 176842 */

#include "essi.h"

char* get_module_name(void)
{
  return "shift";
}


bool is_known_word(word w)
{
  return (!strcmp("<<",w)) || (!strcmp(">>",w) || (!strcmp(">>a",w)) || (!strcmp("<n", w)) || (!strcmp(">n", w))
    || (!strcmp(">na",w)));
}


bool interpret(word w, FILE* f)
{
  char temp[256];
  print_asm_newline(f);
  print_asm0(f, "", "Shift-function");
  print_asm2(f, "cmp", "%g3", "0", "");
  sprintf(temp,"local_function_over_%d",stack_counter);  
  print_asm1(f, "bl", temp, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "cmp", "%g3", "0", "");
  sprintf(temp,"local_stack_underflow_%d",stack_counter);  
  print_asm1(f, "be", temp, "Error-handler");
  print_asm0(f, "nop", "delay");
  
  print_asm2(f, "ld","[%g1]","%l0","shift operation");
  if (! strcmp("<<",w)) {
    print_asm3(f, "sll","%l0","1","%l0","shift left");
  } else if (! strcmp(">>",w)) {
    print_asm3(f, "srl","%l0","1","%l0","shift right");    
  } else if (!strcmp(">>a",w)) {
    print_asm3(f, "sra","%l0","1","%l0","shift right, keep sign");
  } else if (!strcmp("<n",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    sprintf(temp,"local_stack_underflow_%d",stack_counter);  
    print_asm1(f, "be", temp, "Error-handler");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","shift operation");
    print_asm3(f, "sll","%l0","%l1","%l0","shift left");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
  } else if (!strcmp(">n",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    sprintf(temp,"local_stack_underflow_%d",stack_counter);  
    print_asm1(f, "be", temp, "Error-handler");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","shift operation");
    print_asm3(f, "srl","%l0","%l1","%l0","shift right");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
  } else if (!strcmp(">na",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    sprintf(temp,"local_stack_underflow_%d",stack_counter);  
    print_asm1(f, "be", temp, "Error-handler");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","shift operation");
    print_asm3(f, "sra","%l0","%l1","%l0","shift right, keep sign");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
  }
  print_asm2(f, "st","%l0","[%g1]","end shift");
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
