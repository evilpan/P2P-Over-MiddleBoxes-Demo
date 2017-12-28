CFLAGS := -g  -Wall -Wno-implicit-function-declaration
TARGETS := server
all: $(TARGETS)

client : client.c common.c
	gcc $^ -o $@ $(CFLAGS) -pthread
server : server.c endpoint.c message.c logging.c endpoint_list.c
	gcc $^ -o $@ $(CFLAGS)

tests:
	$(MAKE) -C tests

clean: 
	rm -f $(TARGETS) *.o
.PHONY: clean
