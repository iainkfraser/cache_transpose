.PHONY: run clean
transpose : main.c
	gcc main.c -std=c99 -g3 -O2 -o transpose -Wall -Wextra

# force CPU affinity so we don't get boned on cache coherence
run : transpose
	taskset 1 ./transpose
clean:
	rm -f transpose
