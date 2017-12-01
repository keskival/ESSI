/* Tero Keski-Valkama, 176842 */

#include "essi.h"

static int modulo_counter=0;
static int division_counter=0;

char* get_module_name(void)
{
  return "mul";
}


bool is_known_word(word w)
{
  return (!strcmp("*",w)) || (!strcmp("/",w) || (!strcmp("mod",w)));
}


bool interpret(word w, FILE* f)
{
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  char temp_modulo[255];
  char temp_division[255];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);
  sprintf(temp_modulo,"modulo_%d",modulo_counter);
  sprintf(temp_division,"division_%d",division_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Mul-function");


  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  if (! strcmp("*",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","mul operation");
    print_asm2(f, "ld","[%g1+4]","%l1","");
    print_asm3(f, "smul","%l1","%l0","%l1","actual multiplication");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "st","%l1","[%g1]","end mul");



  } else if (! strcmp("/",w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");
    print_asm2(f, "ld","[%g1]","%l0","div operation");
    print_asm2(f, "ld","[%g1+4]","%l1","");

    print_asm3(f, "smul", "%l1", "1", "%l1", "Extend sign to Y");
    print_asm3(f, "sdivcc","%l1","%l0","%l1","actual division");

#ifdef DIVISION_NEG_ROUND_DOWN
    /* Doesn't work correctly if division is negative and goes even. */
    print_asm3(f, "xorcc", "%l0", "%l1", "%l3", "Xorring sign");
    print_asm1(f, "bpos", temp_division, "Jump over");
    print_asm0(f, "nop", "Delay");
    print_asm1(f, "dec", "%l1, "Subtract one to round down");
    sprintf(temp_division,"division_%d:",division_counter);
    print_asm0(f, temp_division, "Jump target");
    division_counter++;
#endif
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "st","%l1","[%g1]","end div");
  } else if (! strcmp("mod", w)) {
    print_asm2(f, "cmp", "%g3", "1", "");
    print_asm1(f, "ble", temp_underflow, "");

    print_asm2(f, "ld","[%g1]","%l0","mod operation");
    print_asm2(f, "ld","[%g1+4]","%l1","");

    print_asm3(f, "smul", "%l1", "1", "%l1", "Extend sign to Y");
    print_asm0(f, "nop", "Delay");
    print_asm0(f, "nop", "Delay");
    print_asm0(f, "nop", "Delay");
    print_asm3(f, "sdiv", "%l1","%l0","%l2","division");
    print_asm2(f, "ld","[%g1]","%l0","mod operation");
    print_asm3(f, "smul","%l2","%l0","%l2","multiplication");
    print_asm3(f, "sub","%l1","%l2","%l0","actual modulo");

    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    print_asm3(f, "add","%g1","4","%g1","adjust stack");
    print_asm2(f, "st","%l0","[%g1]","end mod");
    modulo_counter++;
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
