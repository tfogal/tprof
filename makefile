CF_GLIB=$(shell pkg-config glib-2.0 --cflags)
LD_GLIB=$(shell pkg-config glib-2.0 --libs)
CFLAGS:=-fPIC -Wall -g $(CF_GLIB)
LDFLAGS:=$(LD_GLIB) -ldl
SRC_LIB=tprof.c
SRC_TEST=test.c
OBJ_LIB=$(SRC_LIB:.c=.o)
OBJ_TEST=$(SRC_TEST:.c=.o)
OBJ:=$(OBJ_LIB) $(OBJ_TEST)
PREFIX=$(HOME)/sw
LIBDIR=$(PREFIX)/lib

all: $(OBJ) libtprof.so program

libtprof.so: $(OBJ_LIB)
	$(CC) -shared $(CFLAGS) $^ -o $@ $(LDFLAGS)

program: $(OBJ_TEST) libtprof.so
	$(CC) $< -o $@ libtprof.so

test.o: test.c
	$(CC) -c $(CFLAGS) -finstrument-functions $^ -o $@

install: libtprof.so
	install -m 644 $^ $(LIBDIR)

clean:
	rm -f $(OBJ) libtprof.so program

.PHONY: all clean install
