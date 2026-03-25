EXEC_NAME=main
CODE_FILE=code.fac
OUTPUT_FILE=check

.PHONY: main

main:
	flex facile.lex
	gcc lex.yy.c -o main

test:
	./$(EXEC_NAME) $(CODE_FILE) > $(OUTPUT_FILE)
	python3 not_token.py
