CC=gcc
CFLAGS= -fPIC -I../include
BAR: bar1.o bar2.o

all: $(BAR)

$(BAR): %.o: %.c ../include/common.h ../include/bar.h
	$(CC) $(CFLAGS) -c $< -o $@
