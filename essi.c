/*
 * Extensible Stack System Interpreter
 *
 *
 * Tero Keski-Valkama, 176842 */

#include "essi.h"  /* stdio.h , string.h , ctype.h */
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include "commands.h"

#define MAX_MODULES 100    /* loadable modules for primitives */
#define MAX_DICT 1000      /* dicionary of defined subroutines */
#define RUNTIME_STACK_SIZE 2000 /* max stack for essi-runtime */

struct essi_module module[MAX_MODULES];
int mod_count=0;
int err_index=0;

/* data structure for a single word */
struct worddef {
  char* name; /* word name */
  void* handle; /* object handle */
};
typedef struct worddef worddef_t;

worddef_t words[MAX_DICT];
int dict_size=0;

/* run-time data structures */
unsigned int interp_stack[RUNTIME_STACK_SIZE];
unsigned int* stack_ptr = (RUNTIME_STACK_SIZE-1) + interp_stack;
unsigned int interpreter_storage[262144]; /* 262144 words = 2^20 bytes */

int debug=FALSE;  /* generate lots of debug output? */

void exit_program(void);
void interpret_word(int word_token);
void translate_word(int word_token, FILE*);
void open_and_init_asm(void);
void finish_and_exec_asm(void);

/*static int first_global_translated = FALSE; */
static int inside_subroutine = FALSE;
static int stack_size = 0;

static FILE* global_asm_out;  
char global_asm_fn_stem[80];

int main(void)
{
  open_and_init_asm();
  while ( TRUE ) {
    interpret_word( yylex() );
  }
  return EXIT_SUCCESS;
}

void open_and_init_asm(void)
{
  char global_asm_fn[80];

  /* open a temporary file */
  tmpnam(global_asm_fn_stem);
  strcpy(global_asm_fn, global_asm_fn_stem);
  strcat(global_asm_fn,".s");
  global_asm_out = fopen(global_asm_fn, "w");
  if ( ! global_asm_out ) {
    fprintf(stderr,"Could not open temporary file %s: %s\n",
	    global_asm_fn , strerror(errno));
    exit(1);
  }
  
  /* ...and print program start */

  print_asm(global_asm_out,
	    "\t .section \".text\"\n"
	    "\t .align 4\n"
	    "\t .global essi_program_start\n"
	    "essi_program_start:  ! start program\n"
	    "\t save %sp,-96,%sp\n"
	    "\t mov %g1,%l7 ! save global values\n"
	    "\t mov %g2,%l6\n"
	    "\t mov %g3,%l5\n"
	    "\t mov %i0,%g1  ! set up sp\n"
	    "\t mov %i1,%g2  ! set up storage\n"
	    "\t mov %i2,%g3  ! set up stack-size\n"
	    );



}

void finish_and_exec_asm(void)
{
  char syscmd[256];
  char global_asm_fn[80];
  char global_o_fn[80];
  int sys_status;
  void* dl_handle;
  int (*dl_func)(unsigned int*, unsigned int*, unsigned int);

  strcpy(global_asm_fn, global_asm_fn_stem);
  strcat(global_asm_fn,".s");
  strcpy(global_o_fn, global_asm_fn_stem);
  strcat(global_o_fn,".o");

  print_asm2(global_asm_out,"mov","%g3","%i0","return stack size"); /* Changed from stack pointer, because of error coding */
  print_asm2(global_asm_out,"mov","%l7","%g1","restore global values");
  print_asm2(global_asm_out,"mov","%l6","%g2","");
  print_asm2(global_asm_out,"mov","%l5","%g3","");
  print_asm0(global_asm_out,"ret","finish definition");
  print_asm0(global_asm_out,"restore","remove stack frame");

  fclose(global_asm_out);

  sprintf(syscmd,"/usr/ccs/bin/as -K PIC -o %s %s",
	  global_o_fn,global_asm_fn);
  if(debug){fprintf(stderr,"Execing %s\n",syscmd);}
  sys_status = system(syscmd);
  if ( WEXITSTATUS(sys_status) ) {
    fprintf(stderr,"Execution of command %s failed: %s\n",
	    syscmd,strerror(errno) );
    exit(1);
  }
  if (debug) { printf("Leaving temporary file %s intact.\n",
		      global_asm_fn);
  } else {
    if ( unlink(global_asm_fn) ) {
      fprintf(stderr,"Could not remove file %s : %s\n",
	      global_asm_fn, strerror(errno));
      exit(1);
    }
  }
  dl_handle = dlopen(global_o_fn, RTLD_NOW | RTLD_GLOBAL );
  if (! dl_handle ) {
    fprintf(stderr,"Dynamic object open failed.\n");
    exit(1);
  }
  dl_func = dlsym(dl_handle, "essi_program_start");
  if (! dl_func ) {
    fprintf(stderr,"dlsym failed: %s\n",dlerror() );
    exit(1);
  }

  /* exec the function */
  stack_size = (*dl_func)(stack_ptr, interpreter_storage, stack_size);
  stack_ptr = interp_stack+(RUNTIME_STACK_SIZE-1)-stack_size;
  if (stack_size==-1) {
    printf("!Stack Underflow (Stack reset)!\n");
    stack_ptr = interp_stack+(RUNTIME_STACK_SIZE-1);
    stack_size=0;
  }
  if (stack_size==-2) {
    printf("!Stack Overflow (Stack reset)!\n");
    stack_ptr = interp_stack+(RUNTIME_STACK_SIZE-1);
    stack_size=0;
  }
  if (stack_size==-3) {
    printf("!Memory reference over reserved space (Stack reset)!\n");
    stack_ptr = interp_stack+(RUNTIME_STACK_SIZE-1);
    stack_size=0;
  }

  if (debug) printf("\nDebug: Stack size: %d\n" , stack_size);  
  /* hope everything went fine, now just clean up */
  dlclose(dl_handle);
  if (debug) { printf("Leaving temporary file %s intact.\n", global_o_fn);
  } else {
    if ( unlink(global_o_fn) ) {
      fprintf(stderr,"Could not remove file %s : %s\n",
	      global_o_fn, strerror(errno));
      exit(1);
    }
  }
}

/* Interpret word from the interactive session */
void interpret_word(int word_token)
{
    switch( word_token ) {

    case TOK_CMD_STACK:
      {
        printf("Stack size: %d\n", stack_size);
      }
      break;

    case TOK_CMD_COMMENT:
      {
        int t=0;
        while ( t != TOK_NEWLINE) {
  	  t = yylex();
        }
      }
      break;
    
    case TOK_CMD_EXIT:
      exit_program();
      break;

    case TOK_EOF:
      finish_and_exec_asm();
      exit_program();
      break;
 
    case TOK_NEWLINE:
      finish_and_exec_asm();
      open_and_init_asm();
      break;

    case TOK_CMD_LIST:
      { 
	int i;
	printf("%d modules loaded\n",mod_count);
	for(i=0; i<mod_count; i++) {
	  printf("%s\n", (*(module[i].get_mod_name))() );
	}
      }
      break;

    case TOK_CMD_LOAD:
      { 
	int t;
	char* fn;
	void* mod_handle;
	struct essi_module new_mod;

	t = yylex();
	if (t == TOK_WORD) {
	  /* try to open file */
	  fn = yytext;
	  mod_handle = dlopen(fn,RTLD_NOW | RTLD_GLOBAL);
	  if (! mod_handle ) {
	    fprintf(stderr,"Dynamic library open failed: %s\n",dlerror());
	    break;
	  }
	  new_mod.get_mod_name = dlsym(mod_handle, "get_module_name");
	  if (! new_mod.get_mod_name ) {
	    fprintf(stderr,"dlsym failed for get_module_name: %s\n",dlerror());
	    break;
	  }
	  new_mod.known_word = dlsym(mod_handle, "is_known_word");
	  if (! new_mod.known_word ) {
	    fprintf(stderr,"dlsym failed for is_known_word: %s\n",dlerror());
	    break;
	  }
	  new_mod.interpret_word = dlsym(mod_handle, "interpret");
	  if (! new_mod.interpret_word ) {
	    fprintf(stderr,"dlsym failed for interpret: %s\n",dlerror());
	    break;
	  }
	  new_mod.module_handle = mod_handle;
	  module[mod_count] = new_mod;
	  mod_count++;
	  printf("Loaded %s\n", (*(new_mod.get_mod_name))() );
	} else {
	  fprintf(stderr,"File name expected after load command.\n");
	}
      }
      break;
      
    default:
      translate_word( word_token , global_asm_out );
      break;
    }
}


/* translate a word to machine language (and load) */
void translate_word(int word_token, FILE* outfile)
{

  switch ( word_token ) {
  case TOK_NEWLINE:
    break;
  case TOK_COLON:
    { /* start a definition */
      int defnametok, next_token, inside_sub, new_word_index;
      FILE* asmout;
      void* dl_handle;
      void (*dl_func)(void);
      int sys_status;
      char funcname[80];
      char temp[256]; /* 256 bytes ought to be enough for anybody */
      char temp_fn[256];
      char temp_o_fn[256];
      char temp_asm_fn[256];
      char syscmd[256];

      defnametok=yylex();
      if (defnametok != TOK_WORD) {
	fprintf(stderr, "Expected a word for definition name, got \"%s\".\n",
		yytext);
	return;
      }
      inside_sub = inside_subroutine;
      inside_subroutine = TRUE;
      /* store the word name */
      new_word_index = dict_size;
      dict_size++;
      words[new_word_index].name = (char*) malloc(strlen(yytext)+1);
      strcpy(words[new_word_index].name,yytext);
      /* open a temporary file */
      tmpnam(temp_fn);
      strcpy(temp_asm_fn, temp_fn);
      strcat(temp_asm_fn,".s");
      strcpy(temp_o_fn,temp_fn);
      strcat(temp_o_fn,".o");
      asmout = fopen(temp_asm_fn, "w");
      if ( ! asmout ) {
	fprintf(stderr,"Could not open temporary file %s: %s\n",
		temp_asm_fn , strerror(errno));
	exit(1);
      }
      /* now translate definition */
      sprintf(funcname,"essi_%s",yytext);
      sprintf(temp,
	      "\t .section \".text\"\n"
	      "\t .align 4\n"
	      "\t .global %s\n"
	      "%s:  ! start word definition\n",funcname,funcname);
      print_asm(asmout, temp);
      print_asm3(asmout, "save", "%sp","-96","%sp","stack frame");

      print_asm2(asmout,"mov","%i2","%g3","get stack size");
      print_asm2(asmout,"mov","%i1","%g2","get global values");
      print_asm2(asmout,"mov","%i0","%g1","");

      next_token = yylex();
      while ( next_token != TOK_SEMI ) {
	translate_word(next_token, asmout);
	next_token = yylex();
      }

      print_asm2(asmout,"mov","%g3","%i2","pass stack size");
      print_asm2(asmout,"mov","%g2","%i1","pass global values");
      print_asm2(asmout,"mov","%g1","%i0","");

      print_asm0(asmout,"ret","finish definition");
      print_asm0(asmout,"restore","remove stack frame");
      /* print_asm0(asmout,"nop","delay"); */

      fclose(asmout);
      sprintf(syscmd,"/usr/ccs/bin/as -K PIC -o %s %s",
          temp_o_fn,temp_asm_fn);
      if(debug){fprintf(stderr,"Execing %s\n",syscmd);}
      sys_status = system(syscmd);
      if ( WEXITSTATUS(sys_status) ) {
	fprintf(stderr,"Execution of command %s failed: %s\n",
		syscmd,strerror(errno) );
	exit(1);
      }
      if (debug) { printf("Leaving temporary file %s intact.\n",
			  temp_asm_fn);
      } else {
	if ( unlink(temp_asm_fn) ) {
	  fprintf(stderr,"Could not remove file %s : %s\n",
		  temp_asm_fn, strerror(errno));
	  exit(1);
	}
      }
      dl_handle = dlopen(temp_o_fn, RTLD_LAZY | RTLD_GLOBAL);
      if (! dl_handle ) {
	fprintf(stderr,"Dynamic object open failed.\n");
	exit(1);
      }
      words[new_word_index].handle = dl_handle;
      dl_func = dlsym(dl_handle, funcname);
      if (! dl_func ) {
	fprintf(stderr,"dlsym failed: %s\n",dlerror() );
	exit(1);
      }
      /* now the funcname is in the global symbol space for the linker */
      if (debug) { printf("Leaving temporary file %s intact.\n",
			  temp_o_fn);
      } else {
	if ( unlink(temp_o_fn) ) {
	  fprintf(stderr,"Could not remove file %s : %s\n",
		  temp_o_fn, strerror(errno));
	  exit(1);
	}
      }
      inside_subroutine = inside_sub;
      return;
    }
    
  case TOK_SEMI:
    fprintf(stderr,"Semicolon outside definition!\n");
    break;
    
  case TOK_WORD:
    {
      int i;
      char temp[256]; /* 256 bytes ought to be enough for anybody */
      
      /* first check for programmer-defined word */
      for(i=0; i<dict_size; i++) {
	if ( ! strcmp(words[i].name , yytext) ) {
	  /* match, generate call */
          print_asm_newline(outfile);
          print_asm0(outfile, "", "Call user-defined function");
	  print_asm2(outfile,"cmp","%g3", "0", "Test for error");
	  sprintf(temp,"essi_err_bfr_fn_%d",err_index);
	  print_asm1(outfile,"bl",temp, "Error, jump over call");
	  print_asm0(outfile,"nop","delay");


          print_asm2(outfile,"mov","%g3","%o2","pass stack size");
          print_asm2(outfile,"mov","%g2","%o1","pass global values");
          print_asm2(outfile,"mov","%g1","%o0","");

          sprintf(temp,"call essi_%s",yytext);
	  print_asm0(outfile,temp,"Call defined word");
	  print_asm0(outfile,"nop","Call delay");

          print_asm2(outfile,"mov","%o2","%g3","get stack size");
          print_asm2(outfile,"mov","%o1","%g2","get global values");
          print_asm2(outfile,"mov","%o0","%g1","");

	  sprintf(temp,"essi_err_bfr_fn_%d:",err_index);
	  print_asm0(outfile,temp,"Error, jump over call");
          err_index++;
	  return;
	}
      }
      /* then try built-in from modules */
      for(i=0; i<mod_count; i++) {
	if ( (*(module[i].known_word)) (yytext)) 
	  {
	    (*(module[i].interpret_word))(yytext, outfile);
	    return;
	  }
      }
      fprintf(stderr, "Unknown word \"%s\".\n",yytext);
    }
    break;
    
  default:
    fprintf(stderr,"Unknown token. Internal error.\n");
    exit_program();
    
  }
}
    
void exit_program(void)
{
  /* some cleanup is missing here ... */

  printf("Exiting. Bye.\n");  
  exit(EXIT_SUCCESS);
}
