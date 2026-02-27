EXEC_NAME=facile

.PHONY: main

main:
	flex facile.lex
	bison -d facile.y
	gcc facile.tab.c facile.tab.h lex.yy.c -o $(EXEC_NAME)

clean:
	@rm -f *.tab.*
	@rm -f lex.yy.c
	@rm -f $(EXEC_NAME)
