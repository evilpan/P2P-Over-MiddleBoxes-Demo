CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
TARGETS := client server tests tools
all: $(TARGETS)

client : client.c common.c
	gcc $^ -o $@ $(CFLAGS) -pthread
server : server.c server_logic.c common.c
	gcc $^ -o $@ $(CFLAGS)
tests:
	$(MAKE) -C tests
tools:
	$(MAKE) -C tools

clean: 
	rm -f client server
	make -C tests clean
	make -C tools clean
.PHONY: clean tests tools
