#include <mpi.h>

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include "master.h"
#include "slave.h"


int main(int argc, char **argv) {
    /* MPI initializations */
    int rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // in case youre lost: C-style funcall
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    /* Processing parameter k */
    errno = 0;
    char *endPtr;
    int k;
    long tmp = strtol(argv[3], &endPtr, 10);

    // Verify if conversion went well
    if (errno != 0 || *endPtr != '\0' || tmp > INT_MAX || tmp < INT_MIN) {
        printf("Error occurred while processing parameters");
        return 1;
    } else {
        k = tmp;
    }


    /* Launched node-specific functions */
    if (world_size > 1) {
        if (rank == 0) {
            if (master(world_size, argv[1], argv[2], k, argv[4]))
                printf("Master exited abnormally\n");
            else
                printf("Master exited normally\n");
        } else {
            if (slave(k))
                printf("Slave exited abnormally\n");
            else
                printf("Slave %d exited normally\n", rank);
        }
    } else {
        printf("Program should be launched with at least two nodes\n");
    }


    MPI_Finalize();

    return 0;
}
