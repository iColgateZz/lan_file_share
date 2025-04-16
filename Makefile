CFLAGS=-O2 -Wall -Wextra -pedantic -lm

rule: clean first

first: share.c helpers/safe_string.c
	$(CC) -o share share.c helpers/safe_string.c $(CFLAGS)

clean:
	rm -f app

