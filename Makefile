CC := gcc
CFLAGS := -std=c99

SRCS := cc.c
OBJS := $(SRCS:.c=.o)

cc.exe: $(OBJS) .depend
	$(CC) $(CFLAGS) $(OBJS) -o $@
	
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $^ > .depend
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

include .depend