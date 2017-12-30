CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
TARGETS := server client
all: $(TARGETS)

client : client.c endpoint.c message.c logging.c endpoint_list.c
	gcc $^ -o $@ $(CFLAGS) -pthread
server : server.c endpoint.c message.c logging.c endpoint_list.c
	gcc $^ -o $@ $(CFLAGS)

test:
	$(MAKE) -C tests

clean: 
	rm -f $(TARGETS) *.o
.PHONY: clean
