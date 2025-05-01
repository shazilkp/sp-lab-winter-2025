CC=gcc
FOOOBJ=foo/foo1.o foo/foo2.o foo/foo3.o
BAROBJ=bar/bar1.o bar/bar2.o

all:
	cd foo; make
	cd bar; make
	$(CC) -shared -o libfoobar.so $(FOOOBJ) $(BAROBJS)
