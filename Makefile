CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
all: client server

client : client.c common.c
	gcc $^ -o $@ $(CFLAGS) -pthread
server : server.c server_logic.c common.c
	gcc $^ -o $@ $(CFLAGS)

test:
	$(MAKE) -C tests

clean: 
	rm -f client server
	cd tests && make clean
.PHONY: clean
