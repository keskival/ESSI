CC= gcc
CCOPTS= -ggdb  -W -Wall
#LDOPTS= -L/opt/local/lib -L/opt/local/gnu/lib -R/opt/local/lib  -lfl
LDOPTS=  -L/opt/local/gnu/lib -lfl

OBJS= essi.o essi_lex.o 
SRCS= Makefile essi.c essi_lex.lex commands.h essi.h $(MODSRCS) essimod.c module.c

MODOBJS= int.o add.o if.o io.o stack.o abs.o comp.o mem.o hex.o loop.o mul.o shift.o
MODSRCS=$(patsubst %.o,%.c,$(MODOBJS)) 
MODULES=$(patsubst %.o,mod_%.o,$(MODOBJS))

.SECONDARY: $(MODOBJS)

all: essi modules

essi : $(OBJS) libessimod.so
	$(CC) -o essi $(OBJS) $(LDOPTS) -R. -L. -lessimod

modules : $(MODULES) $(MODOBJS) libessimod.so

%.o : %.c
	$(CC) -fPIC -c $(CCOPTS) $<

mod_%.o : %.o libessimod.so
	$(CC)  -shared -o $@ -R. -L. -lessimod $<

dist : $(SRCS)
	gtar cvzf essi.tar.gz $(SRCS)

libessimod.so : essimod.o
	gcc -shared -o libessimod.so essimod.o 

essi_lex.c : essi_lex.lex commands.h
	flex -oessi_lex.c essi_lex.lex

essi.o : essi.c essi.h commands.h

essimod.o : essimod.c essi.h
