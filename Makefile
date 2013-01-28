transpose : main.c
	gcc main.c -std=c99 -g3 -O0 -o transpose

# force CPU affinity so we don't get boned on cache coherence
run : transpose
	taskset 1 ./transpose
