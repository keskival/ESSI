/* Tero Keski-Valkama, 176842 */

#include "essi.h"

char* get_module_name(void)
{
  return "mem";
}


bool is_known_word(word w)
{
  return (!strcmp("@",w)) || (!strcmp("@b",w) || (!strcmp("!",w)) || (!strcmp("!b",w)));
}


bool interpret(word w, FILE* f)
{
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_mem_ref_failed[256];
  char temp_over[256];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_mem_ref_failed,"local_function_mem_ref_fail_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);
  
  print_asm_newline(f);
  print_asm0(f, "", "Mem-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "be", temp_underflow, "");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "cmp", "%l0", "0", "");
  print_asm1(f, "bl", temp_mem_ref_failed, "");
  print_asm0(f, "nop", "delay");


  print_asm2(f, "set","262144","%l1","Upper-limit of memory adress-space");
  print_asm2(f, "cmp", "%l0", "%l1", "");
  print_asm1(f, "bge", temp_mem_ref_failed, "");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "ld","[%g1]","%l0","memory operation");
  print_asm3(f, "add","%l0","%g2","%l0","Get mem pointer");
  if (! strcmp("@",w)) {
    print_asm2(f, "ld","[%l0]","%l1","load word from memory");
    print_asm2(f, "st","%l1","[%g1]","end memory");
  } else if (! strcmp("@b",w)) {
    print_asm2(f, "ldsb","[%l0]","%l1","load signed byte from memory");
    print_asm2(f, "st","%l1","[%g1]","end memory");
  } else if (! strcmp("!",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","get value");
    print_asm2(f, "st","%l1","[%l0]","store into memory");
    print_asm3(f, "add","%g3","-2","%g3","adjust stack-size");
    print_asm3(f, "add","%g1","8","%g1","adjust stack");
  } else if (! strcmp("!b",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1+4]","%l1","get value");
    print_asm2(f, "stb","%l1","[%l0]","store byte into memory");
    print_asm3(f, "add","%g3","-2","%g3","adjust stack-size");
    print_asm3(f, "add","%g1","8","%g1","adjust stack");
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

  sprintf(temp_mem_ref_failed,"local_function_mem_ref_fail_%d: ",stack_counter);
  print_asm0(f, temp_mem_ref_failed, "Error-handler");
  print_asm2(f, "mov", "-3", "%g3", "errorcode");
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
