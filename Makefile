CFLAGS=-O3 -Wall -Wextra -pedantic

rule: clean first

first: httpd.c 
	$(CC) -o httpd httpd.c $(CFLAGS)

clean:
	rm -f app

