CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
TARGETS := test_list udp_server udp_client
all: $(TARGETS)

test_list: test_list.c ../server_logic.c
	gcc $^ -o $@ $(CFLAGS)
udp_server: udp_server.c
	gcc $^ -o $@ $(CFLAGS)
udp_client: udp_client.c
	gcc $^ -o $@ $(CFLAGS) -lpthread
clean:
	rm -rf $(TARGETS)
.PHONY: clean
