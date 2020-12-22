# Output binary name
bin=a2

# Set the following to '0' to disable log messages:
LOGGER ?= 1

# Compiler/linker flags
CFLAGS += -g -Wall -lm -fPIC -DLOGGER=$(LOGGER)
LDFLAGS +=

src=shell.c ui.c command.c
obj=$(src:.c=.o)

all: $(bin)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

shell.o: shell.c ui.h
ui.o: ui.h ui.c
command.o: command.c command.h

clean:
	rm -f $(bin) $(obj) libshell.so vgcore.*
