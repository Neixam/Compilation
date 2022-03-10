#  Auteur       : Bourennane Amine
#  Creation     : 17-10-2020
#  Modification : 17-10-2020

#	Variable

CC      =	gcc
CFLAGS  =	-Wall
LDFLAGS =   -Wall -lfl -ly
EXEC    = 	bin/tpcc
EXE_SCRI=	src/exec_test.sh
SCRIPT	=	src/test_bash.sh
RESULT	=	output/resultat.txt
RES_EXE	=	output_exe/resultat.txt
INC_PATH=	include/
SRC		=	abstract-tree.c \
			symbol-table.c \
			traduction.c 
SRC_PATH=	src/
OBJ_PATH=	obj/
OBJ		=	$(addprefix $(OBJ_PATH), $(SRC:.c=.o))
SRC_PATH=	src/
OBJ_PATH=	obj/
OBJ		=	$(addprefix $(OBJ_PATH), $(SRC:.c=.o))

# Bison file
YAC		:=	as.y
YSRC	:=	src/yac/$(YAC:.y=).tab.c
YOBJ	:=	$(patsubst src%, obj%, $(YSRC:.c=.o))
YINC	:=	$(patsubst src/yac%, include%, $(YSRC:.c=.h))
YTAB    :=  $(YAC:.y=.output)
YGRA	:=	$(YAC:.y=.dot)
YPDF	:=	$(YAC:.y=.pdf)

# Flex file
LEX		:=	as.lex
LSRC	=	src/lex/lex.yy.c
LOBJ	=	$(patsubst src%, obj%, $(LSRC:.c=.o))

#	Compilation

$(EXEC)		:	$(YOBJ) $(LOBJ) $(OBJ)
	@mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

$(SCRIPT)	:   $(EXEC)
	@./$@
	@cat $(RESULT)
	@./$(EXE_SCRI)
	@cat $(RES_EXE)

$(YTAB)     :	src/$(YAC)
	bison -v -o $@ $< -I $(INC_PATH)

$(YPDF)		:	$(YGRA)
	dot -Tpdf -o $@ $<

$(YGRA)		:	src/$(YAC)
	bison -g -o $@ $<

obj/abstract-tree.o	:	src/abstract-tree.c include/abstract-tree.h
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

obj/symbol-table.o	:	src/symbol-table.c include/symbol-table.h include/abstract-tree.h
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

obj/traduction.o	:	src/traduction.c include/abstract-tree.h include/traduction.h
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

obj/powl.o			:	src/powl.c include/powl.h
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

$(YOBJ)		:	$(YSRC)
	@mkdir -p obj obj/yac
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

$(LOBJ)		:	$(LSRC) 
	@mkdir -p obj obj/lex
	$(CC) -o $@ -c $< $(CFLAGS) -I $(INC_PATH)

$(LSRC)		:	src/$(LEX)
	flex -o $@ $<

$(YSRC)		:	src/$(YAC)
	@mkdir -p $(INC_PATH)
	bison -d -o $@ $<
	@mv $(YSRC:.c=.h) $(INC_PATH)

#	Rules

all		:	$(EXEC)

test	:	$(SCRIPT)

table	:   $(YTAB)

graph	:	$(YPDF)
	@evince $< &

clean	:
	rm -f $(YSRC) $(YOBJ) $(LSRC) $(LOBJ) $(YINC) $(RESULT) $(YTAB) $(YPDF) $(YGRA) $(OBJ)

mrproper:	clean
	rm -rf output/ output_exe/ obj/ $(EXEC)

re		:	mrproper all
