GLIB_FLAGS = $(shell pkg-config glib-2.0 --cflags)
GLIB_LIBS  = $(shell pkg-config glib-2.0 --libs)

BUILD_DIR = "build"
EXEC_NAME="facile"

.PHONY: main

main:
	flex facile.lex
	bison -d facile.y
	@rm -f ../$(BUILD_DIR)/$(EXEC_NAME)
	gcc facile.tab.c lex.yy.c $(GLIB_FLAGS) -o ../$(BUILD_DIR)/$(EXEC_NAME) $(GLIB_LIBS)
	@$(MAKE) -s clean

clean:
	@rm -f *.tab.*
	@rm -f *.output
	@rm -f lex.yy.c
