OBJECTS= symtab.o print_pcode.o matrix.o my_main.o display.o draw.o gmath.o stack.o thpool.o
CFLAGS= -g -Wall
LDFLAGS= -lm -pthread
CC= gcc
EXECUTABLE= mdl
SCRIPT= simple_anim.mdl

run: $(EXECUTABLE) $(SCRIPT)
	./$(EXECUTABLE) $(SCRIPT)

debug: $(EXECUTABLE) $(SCRIPT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXECUTABLE) $(SCRIPT)

$(EXECUTABLE): lex.yy.c y.tab.c y.tab.h $(OBJECTS)
	$(CC) $(CFLAGS) -o mdl lex.yy.c y.tab.c $(OBJECTS) $(LDFLAGS)

lex.yy.c: mdl.l y.tab.h
	flex -I mdl.l

y.tab.c y.tab.h: mdl.y symtab.h parser.h
	bison -d -y mdl.y

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *~ y.tab.c y.tab.h lex.yy.c mdl anim/* *.gif *.png
	rm -rf mdl.dSYM
