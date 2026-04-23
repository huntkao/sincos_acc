CC = gcc
CFLAGS = -O3 -I./include -I./extern
LDFLAGS = -lm

all: perf_test

perf_test: src/sincos_neon.c benchmarks/perf_test.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f perf_test
