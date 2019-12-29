CC=gcc
RM=rm -f

SRCS=v1541.c blocks.c
OBJS=$(subst .c,.o,$(SRCS))

all: v1541

v1541: $(OBJS)
	$(CC) -o v1541 $(OBJS)

v1541.o: v1541.c

clean:
	$(RM) $(OBJS)
