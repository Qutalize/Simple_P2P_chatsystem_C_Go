CC      = gcc
CFLAGS  = -g -Wall
LDFLAGS = -lm -lrt
TARGETS = raw_chat udp_chat

all: $(TARGETS)

raw_chat: raw_chat.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

udp_chat: udp_chat.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)