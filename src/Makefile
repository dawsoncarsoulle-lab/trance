EXEC_PATH = ../build/facile

.PHONY: main

main:
	@mkdir -p ../build
	flex facile.lex
	bison -d facile.y
	@rm -f $(EXEC_PATH)
	gcc facile.tab.c lex.yy.c -o $(EXEC_PATH)
	@$(MAKE) -s clean

clean:
	@rm -f *.tab.*
	@rm -f *.output
	@rm -f lex.yy.c
