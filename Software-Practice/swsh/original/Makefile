CC		= gcc
CFLAGS		= -g -Wall

RM		= rm
TAR		= tar

###### fill here if you add other code/header files #####
# add .c files here
CSRCS		= swsh.c parse_input.c pa0_2016313621.c exec_func.c cmds.c bin/err.c

# add other files here
OTHERS	= Makefile pa0_2016313621.h swsh_header.h parse_input.h
#########################################################

TARGET		= swsh
OBJECTS		= $(CSRCS:.c=.o)

ifeq (tar,$(firstword $(MAKECMDGOALS)))
  TAR_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(TAR_ARGS):;@:)
endif

all: sub $(TARGET)

sub:
	cd bin; $(MAKE)

.c.o:
	$(CC) $(CFLAGS) -w -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) -w -o $@ $^


tar:
	$(TAR) zcvf $(TAR_ARGS).tar.gz $(CSRCS) $(OTHERS)

.PHONY:	clean

clean:
	$(RM) -f $(OBJECTS) $(TARGET) *~ ; cd bin; $(MAKE) clean
