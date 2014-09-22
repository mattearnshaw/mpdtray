CC=clang
INSTALL=install
PREFIX=/usr

LIBS+=`pkg-config --libs gtk+-3.0 libmpdclient`
LIBS+=-lm
CFLAGS+=`pkg-config --cflags gtk+-3.0 libmpdclient`
CFLAGS+=-Wall

FILES:=$(wildcard *.c)
FILES:=$(FILES:.c=.o)

all: mpdtray
mpdtray: ${FILES}
	$(CC) -o $@ $^ $(LIBS)

install: all
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 755 mpdtray $(DESTDIR)$(PREFIX)/bin/mpdtray

clean:
	rm -f mpdtray ${FILES}
