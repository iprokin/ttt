GCC=$(shell which musl-gcc >/dev/null && echo "musl-gcc" || echo "gcc")

all:
	$(GCC) -Os -Wall -std=c99 -pedantic ttt.c -o ttt
