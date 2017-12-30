CFLAGS := -g -std=c99 -Wall -Wno-implicit-function-declaration
TARGETS := p2pchat tools
all: $(TARGETS)

p2pchat:
	$(MAKE) -C p2pchat
test:
	$(MAKE) -C p2pchat/tests
tool:
	$(MAKE) -C tools

clean: 
	make -C p2pchat clean
	make -C p2pchat/tests clean
	make -C tools clean
.PHONY: clean p2pchat test tools
