/* Tero Keski-Valkama, 176842 */

#include "essi.h"

static int loop_nesting = 0;
static int loop_counter = 0;
static int loop_id_stack[100];
static int while_seen_stack[100];


char* get_module_name(void)
{
  return "loop";
}


bool is_known_word(word w)
{
  return ((!strcmp("begin",w))||(!strcmp("while",w))||(!strcmp("repeat",w)));
}


bool interpret(word w, FILE* f)
{
  char temp[256];
  char temp_underflow[256];
  char temp_overflow[256];
  char temp_over[256];
  sprintf(temp_underflow,"local_stack_underflow_%d",stack_counter);
  sprintf(temp_overflow,"local_stack_overflow_%d",stack_counter);
  sprintf(temp_over,"local_function_over_%d",stack_counter);

  print_asm_newline(f);
  print_asm0(f, "", "Loop-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  if (!strcmp("begin",w)) {
    loop_counter++;
    loop_id_stack[loop_nesting]=loop_counter;
    while_seen_stack[loop_nesting]=FALSE;
    loop_nesting++;
    sprintf(temp,"loop_%d:",loop_counter);
    print_asm0(f, temp, "loop start");
  } else if(!strcmp("while",w)) {
    if (loop_nesting<1) {
      printf("!While without begin!\n");
      return FALSE;
    }
    while_seen_stack[loop_nesting-1]=TRUE;
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","loop test");
    print_asm2(f, "cmp","%l0","0","compare to zero");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    sprintf(temp,"be loop_over_%d",loop_id_stack[loop_nesting-1]);
    print_asm0(f, temp, "branch on false");
    print_asm3(f, "add","%g1","4","%g1","adjust stack (delay slot)");
  } else if(!strcmp("repeat",w)) {
   if (loop_nesting<1) {
      printf("!Repeat without begin!\n");
      return FALSE;
    }
    if (while_seen_stack[loop_nesting-1]==FALSE) {
      printf("!Repeat without while!\n");
      return FALSE;
    }

    sprintf(temp,"b loop_%d",loop_id_stack[loop_nesting-1]);
    print_asm0(f, temp, "branch on false");
    print_asm0(f, "nop","(delay)");
    sprintf(temp,"loop_over_%d:",loop_id_stack[loop_nesting-1]);
    print_asm0(f, temp, "loop end");
    loop_nesting--;
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
