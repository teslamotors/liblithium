CC=gcc -O1 -fomit-frame-pointer

.PHONY: all
all: test.txt test_hash.tex

test.txt: test
	./test > test.txt
	cmp test.txt test.exp

test_hash.tex: test_hash
	./test_hash 2> test_hash.tex
	cmp test_hash.tex test_hash.exp

test: gimli.o

test_hash: gimli.o gimli_hash.o

clean:
	@echo "cleaning..."
	@$(RM) test test.txt test_hash test_hash.tex *.o
