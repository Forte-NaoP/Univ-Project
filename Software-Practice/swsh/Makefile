CC = gcc
CFLAGS = -I. -Icommands
SRCS = mystring.c argparse.c commands/inline.c commands/error_handler.c swsh.c
OBJS = $(SRCS:.c=.o)
TARGET = swsh

all: commands_build $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

commands_build:
	$(MAKE) -C commands all

clean:
	$(MAKE) -C commands clean
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean commands_build
