TEST_SRCS = $(wildcard *.facile)
EXES = $(patsubst %.facile,build/%.exe,$(TEST_SRCS))

ASSEMBLER_EXEC = ilasm
FACILE_COMPILER_PATH = ../build/facile

.PHONY: test clean

test: build/tester $(EXES)
	@./build/tester || true

build/tester: tester.c ../include/error.h
	@mkdir -p build
	@gcc tester.c -o build/tester

build/%.exe: %.facile ../build/facile
	@mkdir -p build
	@if $(FACILE_COMPILER_PATH) $< 2>/dev/null; then    \
		$(ASSEMBLER_EXEC) $*.il /output:$@ >/dev/null;      \
		rm -f $*.il;                            \
	else                                        \
		touch $@;                               \
	fi

clean:
	@rm -rf build
