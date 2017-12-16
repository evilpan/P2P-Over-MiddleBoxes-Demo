CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
TARGETS := udp_server udp_client
all: $(TARGETS)

udp_server: udp_server.c
	gcc $^ -o $@ $(CFLAGS)
udp_client: udp_client.c
	gcc $^ -o $@ $(CFLAGS) -lpthread
clean:
	rm -rf $(TARGETS)
.PHONY: clean

