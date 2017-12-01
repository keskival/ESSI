/* Tero Keski-Valkama, 176842 */

#include "essi.h"

static int if_nesting = 0;
static int if_counter = 0;
static int if_id_stack[100];
static int else_seen_stack[100];

char* get_module_name(void)
{
  return "if";
}


bool is_known_word(word w)
{
  return ((!strcmp("if",w))||(!strcmp("else",w))||(!strcmp("endif",w)));
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
  print_asm0(f, "", "If-function");

  print_asm2(f, "cmp", "%g3", "0", "");
  print_asm1(f, "bl", temp_over, "Jump over function, previous generated error");
  print_asm0(f, "nop", "delay");

  if (!strcmp("if",w)) { 
    if_counter++;
    if_id_stack[if_nesting] = if_counter;
    else_seen_stack[if_nesting] = FALSE;
    print_asm2(f, "cmp", "%g3", "0", "");
    print_asm1(f, "be", temp_underflow, "");
    print_asm0(f, "nop", "delay");
    print_asm2(f, "ld","[%g1]","%l0","if operation:");
    print_asm2(f, "cmp","%l0","0","compare to zero");
    print_asm3(f, "add","%g3","-1","%g3","adjust stack size");
    sprintf(temp,"be falseif_%d",if_counter);
    print_asm0(f, temp, "branch on false");
    print_asm3(f, "add","%g1","4","%g1","adjust stack (delay)");
    if_nesting++;
  } else if (!strcmp("else",w)) { 
    sprintf(temp,"b endif_%d",if_id_stack[if_nesting-1]);
    print_asm0(f,temp,"end of then-branch");
    print_asm0(f,"nop","delay");
    sprintf(temp,"falseif_%d:",if_id_stack[if_nesting-1]);
    print_asm0(f,temp,"else-branch");
    else_seen_stack[if_nesting-1] = TRUE;
  } else if (!strcmp("endif",w)) { 
    if_nesting--;
    if ( ! else_seen_stack[if_nesting] ) {
      sprintf(temp,"falseif_%d:",if_id_stack[if_nesting]);
      print_asm(f,temp);
    }
    sprintf(temp,"endif_%d:",if_id_stack[if_nesting]);
    print_asm(f,temp);
    
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
