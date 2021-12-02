//#include <mpi.h>
#include <stdio.h>

/* Template, don't ask too much question, just copy it */

void master(int world_size) {
  printf("Rank is %d/%d, master\n", 0, world_size);
  MPI_Send(0, 1, MPI_INT, 1, 15, MPI_COMM_WORLD);
}

void slave(int rank, int world_size) {
  int prev;
  MPI_Status status;
  if (rank < world_size - 1){
	MPI_Recv(&prev, 1, MPI_INT, rank-1, 15, MPI_COMM_WORLD, &status);
	printf("Rank is %d, previous is %d, slave\n", rank, prev);
	// not sure for the references
	MPI_Send(rank, 1, MPI_INT, rank+1, 15, MPI_COMM_WORLD);
  } else {
	MPI_Recv(rank, 1, MPI_INT, rank-1, 15, MPI_COMM_WORLD, &status);
	printf("Rank is %d, last\n", rank, a);
  }
}

int main(int argc, char **argv) {
  int rank, world_size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // in case youre lost: C-style funcall
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (rank == 0) {
    master(world_size);
  } else {
    slave(rank, world_size);
  }

  MPI_Finalize();
  return 0;
}
