/* Tero Keski-Valkama, 176842 */

#include "essi.h"

char* get_module_name(void)
{
  return "io";
}


bool is_known_word(word w)
{
  return ((!strcmp(".",w))||(!strcmp("putc",w))||(!strcmp("getc",w)));
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
  print_asm0(f, "", "IO-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  if (!strcmp(".",w)) {
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%o0","print int (pop)");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "mov","%g1","%l7","save global g1");
    print_asm2(f, "mov","%g2","%l6","save global g2");
    print_asm0(f, "call print_int","printing routine");
    print_asm2(f, "mov","%g3","%l5","save global g3 (delay)");
    print_asm2(f, "mov","%l7","%g1","restore global values");
    print_asm2(f, "mov","%l6","%g2","");
    print_asm2(f, "mov","%l5","%g3","");
  } else if (!strcmp("putc",w)) {
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%o0","put char (pop)");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack-size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "mov","%g1","%l7","save global g1");
    print_asm2(f, "mov","%g2","%l6","save global g2");
    print_asm0(f, "call put_char","printing routine");
    print_asm2(f, "mov","%g3","%l5","save global g3 (delay)");
    print_asm2(f, "mov","%l7","%g1","restore global values");
    print_asm2(f, "mov","%l6","%g2","");
    print_asm2(f, "mov","%l5","%g3","");
  } else if (!strcmp("getc",w)) {
    print_asm2(f, "cmp", "%g3", "1999", "");
    print_asm1(f, "be", temp_overflow, "");
    print_asm2(f, "mov","%g1","%l7","save global g1");
    print_asm2(f, "mov","%g2","%l6","save global g2");
    print_asm0(f, "call get_char","printing routine");
    print_asm2(f, "mov","%g3","%l5","save global g3 (delay)");
    print_asm2(f, "mov","%l7","%g1","restore global values");
    print_asm2(f, "mov","%l6","%g2","");
    print_asm2(f, "mov","%l5","%g3","");
    print_asm3(f, "add","%g1","-4","%g1","adjust stack");
    print_asm3(f, "add","%g3","1","%g3","adjust stack-size");
    print_asm2(f, "st","%o0","[%g1]","get char (push)");
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
