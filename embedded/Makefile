all: test benchmark

ifeq ($(shell uname -s), Darwin)
DYLIB_SETUP=DYLD_LIBRARY_PATH=`pwd`/../src
else
DYLIB_SETUP=LD_LIBRARY_PATH=`pwd`/../src
EXTRA_LDFLAGS = -Wl,-rpath `pwd`/../src
endif

CC ?= gcc
CFLAGS = -O2 -g

clean:
	-rm -f test test.o benchmark benchmark.o ../src/liberedis.so
	$(MAKE) -C ../src clean

../src/liberedis.so:
	$(MAKE) -C ../src embedded

test: ../src/liberedis.so test.o
	$(CC) test.o -o test -L../src -leredis $(EXTRA_LDFLAGS) $(EXTRA_LIBS)

benchmark: ../src/liberedis.so benchmark.o
	$(CC) benchmark.o -o benchmark -fPIE -L../src -leredis $(EXTRA_LDFLAGS) $(EXTRA_LIBS)

tests: test
	$(DYLIB_SETUP) ./test

benchmarks: benchmark
	$(DYLIB_SETUP) ./benchmark