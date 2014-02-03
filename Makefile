
CFLAGS = -O0 -g -DLNET_DEBUG -std=gnu99 -Wall -Iinclude
LD = ld
CC = clang

OBJECTS = \
deps/list.o \
src/buffer.o \
src/message.o \
src/error.o \
src/client/client.o \
src/client/channel.o \
src/client/message.o \
src/client/peer.o \
src/client/request/channel_list.o \
src/client/request/connect.o \
src/client/request/join_channel.o \
src/client/request/leave_channel.o \
src/client/request/set_name.o \
src/server/channel.o \
src/server/client.o \
src/server/server.o \
src/server/message.o \
src/server/request/channel_list.o \
src/server/request/connect.o \
src/server/request/join_channel.o \
src/server/request/leave_channel.o \
src/server/request/set_name.o

all: liblacenet.a test-client test-server

liblacenet.a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) test/client.o liblnet.a test-client

test-client: test/client.o liblacenet.a
	$(CC) -o test-client test/client.o liblacenet.a -llacewing -lc

test-server: test/server.o liblacenet.a
	$(CC) -o test-server test/server.o liblacenet.a -llacewing -lc

.PHONY: all clean
.SUFFIXES: .c .o .h

