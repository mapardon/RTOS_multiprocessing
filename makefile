FLAGS = -I_MPI_WAIT_MODE=0 -I_MPI_THREAD_YIELD=3 -I_MPI_THREAD_SLEEP=10
CFLAGS = -Wall -Wextra -Wpedantic -lm
CC = mpicc
OBJECTS := read.o master.o slave.o heap.o
OUT = knn

all: $(OUT)
	@echo "Compile"

$(OUT): $(OBJECTS)
	@echo "Compile main"
	$(CC) main.c $^ -o $@ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	clear
	@echo "Clean"
	echo > dummy.o
	rm *.o

mrproper: clean
	@echo "MrProper"
	echo >> knn
	echo >> test
	rm knn
	rm test

rebuild: mrproper all
	@echo "Rebuild"

run: all
	mpirun --oversubscribe -n 12 $(OUT) "data/example2_env.txt" "data/example1_chal.txt" 500 "out"
