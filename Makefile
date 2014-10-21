.PHONY: all
all: ascii-plot

ascii-plot: ascii-plot.c
	gcc -ansi -Wall -pedantic -O3 -o$@ $^

.PHONY: clean
clean:
	-rm ascii-plot

.PHONY: install
install: ascii-plot
	install $< /usr/local/bin/ascii-plot
