TEST_SRCS = $(wildcard *.facile)
EXES = $(patsubst %.facile,build/%.exe,$(TEST_SRCS))

.PHONY: test clean

test: build/tester $(EXES)
	@./build/tester || true

build/tester: tester.c ../include/error.h
	@mkdir -p build
	@cc tester.c -o build/tester

build/%.exe: %.facile ../build/facile
	@mkdir -p build
	@if ../build/facile $< 2>/dev/null; then    \
		ilasm $*.il /output:$@ >/dev/null;      \
		rm -f $*.il;                            \
	else                                        \
		touch $@;                               \
	fi

clean:
	@rm -rf build
