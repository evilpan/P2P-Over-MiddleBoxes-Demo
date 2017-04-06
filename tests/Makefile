CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
all: test_list

test_list: test_list.c ../server_logic.c
	gcc $^ -o $@ $(CFLAGS)
clean:
	rm -rf test_list
.PHONY: clean
