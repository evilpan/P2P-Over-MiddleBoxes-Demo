CFLAGS := -g -ggdb -std=c99
all: client server

client : client.c
	gcc $^ -o $@ $(CFLAGS) -pthread
server : server.c
	gcc $^ -o $@ $(CFLAGS)

clean: 
	rm -f client server
.PHONY: clean
