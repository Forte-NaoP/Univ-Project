#------ custom definitions

OBJS = mv rm cat cp head tail
SUBOBJS = error_handler.c mystring.c
BIN = bin

#------ constant definitions

ALL_OBJ = $(OBJS:%=%.o)

all: $(OBJS)

clean:
	$(RM) $(BIN)/$(OBJS:%=$(BIN)/%) $(ALL_OBJ)

# normally still defined /usr/share/lib/make/make.rules
CC = gcc

DO_OBJS = $(CC) -w -o $(BIN)/$@ $@.c $(SUBOBJS)

#------ now compile

$(OBJS):$(@:%=%.c)
	$(DO_OBJS)
