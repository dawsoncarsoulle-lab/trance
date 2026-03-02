GLIB_FLAGS = $(shell pkg-config glib-2.0 --cflags)
GLIB_LIBS  = $(shell pkg-config glib-2.0 --libs)

EXEC_PATH = ../build/facile

.PHONY: main

main:
	flex facile.lex
	bison -d facile.y
	@rm -f $(EXEC_PATH)
	gcc facile.tab.c lex.yy.c -g $(GLIB_FLAGS) -o $(EXEC_PATH) $(GLIB_LIBS)
	@$(MAKE) -s clean

clean:
	@rm -f *.tab.*
	@rm -f *.output
	@rm -f lex.yy.c
