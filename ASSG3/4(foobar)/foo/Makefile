CC=gcc
CFLAGS= -fPIC -I../include
FOO: foo1.o foo2.o foo3.o

all: $(FOO)

$(FOO): %.o: %.c ../include/common.h ../include/foo.h
	$(CC) $(CFLAGS) -c $< -o $@
