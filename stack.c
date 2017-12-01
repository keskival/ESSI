/* Tero Keski-Valkama, 176842 */

#include "essi.h"

char* get_module_name(void)
{
  return "stack";
}


bool is_known_word(word w)
{
  return ((!strcmp("dup",w))||(!strcmp("drop",w))||(!strcmp("over",w))||(!strcmp("swap",w))||(!strcmp("rot",w)));
}


bool interpret(word w, FILE* f)
{
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Stack-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  if (!strcmp("dup",w)) {
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm2(f, "cmp", "%g3", "1999", "(delay, np)");
    print_asm1(f, "be", temp_overflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","dup operation");
    print_asm3(f, "add","%g1","-4","%g1","adjust stack");
    print_asm3(f, "add","%g3","1","%g3","adjust stack size");
    print_asm2(f, "st","%l0","[%g1]","end dup");
  } else if (!strcmp("drop",w)) {
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
  } else if (!strcmp("over",w)) {
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm2(f, "cmp", "%g3", "1999", "(delay, np)");
    print_asm1(f, "be", temp_overflow, "");
    print_asm0(f, "nop", "delay");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "ld","[%g1]","%l0","dup operation");
    print_asm3(f, "add","%g1","-8","%g1","adjust stack");
    print_asm3(f, "add","%g3","1","%g3","adjust stack size");
    print_asm2(f, "st","%l0","[%g1]","end dup");
  } else if (!strcmp("swap",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","dup operation");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "ld","[%g1]","%l1","dup operation");
    print_asm2(f, "st","%l0","[%g1]","end dup");
    print_asm3(f, "add","%g1","-4","%g1","adjust stack");
    print_asm2(f, "st","%l1","[%g1]","end dup");
  } else if (!strcmp("rot",w)) {
    print_asm2(f, "cmp", "%g3", "2", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","dup operation");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "ld","[%g1]","%l1","dup operation");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "ld","[%g1]","%l2","dup operation");
    print_asm2(f, "st","%l1","[%g1]","end dup");
    print_asm3(f, "add","%g1","-4","%g1","adjust stack");
    print_asm2(f, "st","%l0","[%g1]","end dup");
    print_asm3(f, "add","%g1","-4","%g1","adjust stack");
    print_asm2(f, "st","%l2","[%g1]","end dup");
  } else {
    return FALSE;
  }
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
