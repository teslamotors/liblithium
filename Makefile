CC=gcc -O1 -fomit-frame-pointer

.PHONY: all
all: test.txt test_hash.tex

test.txt: test
	./test > test.txt
	cmp test.txt test.exp

test_hash.tex: test_hash
	./test_hash 2> test_hash.tex
	cmp test_hash.tex test_hash.exp

obj:
	mkdir -p obj

test: test.c gimli.c
	$(CC) -o test test.c gimli.c

obj/gimli.o: obj
	@echo CC $@
	@gcc -c gimli.c -o obj/gimli.o

test_hash: obj/gimli.o test_hash.c gimli_hash.c
	@echo CC $@
	@$(CC) obj/gimli.o test_hash.c gimli_hash.c -o test_hash

clean:
	@echo "cleaning..."
	@$(RM) -r test test.txt test_hash test_hash.tex obj
