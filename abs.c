/* Tero Keski-Valkama, 176842 */

#include "essi.h"

static int abs_counter=0;

char* get_module_name(void)
{
  return "abs";
}


bool is_known_word(word w)
{
  return (!strcmp("abs",w));
}


bool interpret(word w, FILE* f)
{
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  char temp_abs[255];

  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);
  sprintf(temp_abs,"abs_%d",abs_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Abs-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "ble", temp_underflow, "");
  print_asm0(f, "nop", "delay");
  if (! strcmp("abs",w)) {
    print_asm2(f, "ld","[%g1]","%l0","abs operation");
    print_asm2(f, "cmp", "%l0", "0", "");
    print_asm1(f, "bge", temp_abs, "Jump over negation");
    print_asm0(f, "nop", "delay");
    print_asm1(f, "neg","%l0","Negate");

    sprintf(temp_abs,"abs_%d:",abs_counter);
    print_asm0(f, temp_abs, "");
    print_asm2(f, "st","%l0","[%g1]","end abs");
    abs_counter++;
  } else {
    return FALSE;
  }

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
