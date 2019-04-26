CC=gcc -O1 -fomit-frame-pointer

test.out: test test_hash
	./test > test.out
	cmp test.out test.exp
	./test_hash 2> testVectors.tex

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
	@rm -r test 2> /dev/null || true
