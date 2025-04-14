CFLAGS=-O2 -Wall -Wextra -pedantic

rule: clean first

first: httpd.c helpers/safe_string.c
	$(CC) -o httpd httpd.c helpers/safe_string.c $(CFLAGS)

clean:
	rm -f app

