CFLAGS := -g -Wall -Wno-implicit-function-declaration
TARGETS := test_logging test_endpoint test_message test_eplist
all: $(TARGETS)

test_logging: test_logging.c ../logging.c
	gcc $^ -o $@ $(CFLAGS)
test_endpoint: test_endpoint.c ../endpoint.c
	gcc $^ -o $@ $(CFLAGS)
test_message: test_message.c ../message.c
	gcc $^ -o $@ $(CFLAGS)
test_eplist: test_eplist.c ../endpoint.c ../endpoint_list.c
	gcc $^ -o $@ $(CFLAGS)
clean:
	rm -rf $(TARGETS)
.PHONY: clean
