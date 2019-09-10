CC=gcc
LIBS=`pkg-config --libs json-c`
CFLAGS=-O2 `pkg-config --cflags json-c` -Wno-discarded-qualifiers

all:
	$(CC) $(LIBS) $(CFLAGS) *.c -o kowalski

clean:
	rm -f kowalski
