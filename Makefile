CC := gcc
CFLAGS := -std=c99 -g -Wall -pedantic -fopenmp

SRCS := cc.c lexer.c parser.c
OBJS := $(SRCS:.c=.o)

.PHONY: clean

cc.exe: $(OBJS) .depend
	$(CC) $(CFLAGS) $(OBJS) -o $@
	
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $^ > .depend
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

include .depend

clean:
	rm -fv $(OBJS) cc.exe .depend