#------ coustomer definitions

OBJS = mv rm cat cp head tail
SUBOBJS = err.c pa0_2016313621.c

#------ constant definitions

ALL_OBJ = $(OBJS:%=%.o)

all: $(OBJS)

clean:
	$(RM) $(OBJS) $(ALL_OBJ)

CC = gcc # normally still defined /usr/share/lib/make/make.rules

DO_OBJS = $(CC) -w -o $@ $@.c $(SUBOBJS)

#------ now compile

$(OBJS):	$(@:%=%.c)
		$(DO_OBJS)
