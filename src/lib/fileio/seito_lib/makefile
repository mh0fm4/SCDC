prog: benchmark.o
	gcc -o prog benchmark.o

benchmark.o: benchmark.c
	gcc -c benchmark.c

$(shell ${PWD}/lib/skript.sh)
#LD_PRELOAD=./lib/libseito.so ./prog
