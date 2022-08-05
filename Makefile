CC=gcc
CFLAGS = -Wall -Wextra

client: client.c requests.c helpers.c buffer.c
	$(CC) -o client -g client.c requests.c helpers.c buffer.c $(CFLAGS)

run: client
	./client

clean:
	rm -f *.o client

pack:
	zip -FSr Blotiu_Mihnea-Andrei_323CA_Tema3PC.zip README Makefile *.c *.h

.PHONY: clean
