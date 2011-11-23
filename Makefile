
.PHONY: clean

example: huffman.c huffman.h
	gcc -o example -ggdb huffman.c

clean:
	rm -rf *.o example *.wlog
