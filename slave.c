/*
 * salve.c
 *
 *  Created on: Dec 2, 2021
 *      Author: mathieu
 */

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "heap.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MASTER 0
#define ERROR -1


/*
 * Receives pointers to an environment point and a challenger and computes the distance between them
 */
double distance(float* environment, float* challenger, int dimension) {
    double dist = 0;
    for (int i = 0 ; i < dimension ; i++)
        dist += pow(*(environment + i) - *(challenger + i), 2.0);
    return sqrt(dist);
}


int slave(int k) {

    MPI_Status status;
    int dimension, subenv_size, nb_challengers;

    // If master fails allocation: exit the program
    int warning;
    MPI_Bcast(&warning, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (warning)
        return ERROR;


    // Receive dimension of points
    MPI_Bcast(&dimension, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Receive number of challengers
    MPI_Bcast(&nb_challengers, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Receive number of environment points
    MPI_Recv(&subenv_size, 1, MPI_INT, MASTER, 15, MPI_COMM_WORLD, &status);


    // Allocate space for values to receive
    float *subenv_points = (float*) malloc(dimension * subenv_size * sizeof(float)),
          *challengers = (float*) malloc((dimension - 1) * nb_challengers * sizeof(float));
    double *results = (double*) malloc(MIN(k, subenv_size) * 2 * sizeof(double));  // used later


    // Indicate to master if allocation went right
    int feedback = (subenv_points == NULL) || (challengers == NULL);  // sends 1 if trouble
    MPI_Send(&feedback, 1, MPI_INT, MASTER, 15, MPI_COMM_WORLD);


    // Wait confirmation to continue (indirect confirmation of others)
    int can_continue;
    MPI_Recv(&can_continue, 1, MPI_INT, MASTER, 15, MPI_COMM_WORLD, &status);

    if (!can_continue) {
        if (subenv_points != NULL)
            free(subenv_points);
        if (challengers != NULL)
            free(challengers);

        printf("An error occurred during execution\n");
        return ERROR;
    }


    // Receive challengers
    MPI_Bcast(challengers, nb_challengers * (dimension - 1), MPI_FLOAT, 0, MPI_COMM_WORLD);


    // Receive the environment points
    MPI_Recv(subenv_points, subenv_size * dimension, MPI_FLOAT, MASTER, 15, MPI_COMM_WORLD, &status);


    // Wait for master confirmation
    MPI_Bcast(&warning, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (warning) {
        free(subenv_points);
        free(challengers);
        return ERROR;
    }


    // Distance computation
    double dist;
    unsigned n_selec = MIN(k, subenv_size);  // In case there would be less environment points than k
    Heap *closest_dists;
    closest_dists = init_heap(n_selec);

    // Test heap creation
    warning = closest_dists == NULL;
    MPI_Reduce(&warning, NULL, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Wait for confirmation (other nodes failed ?)
    MPI_Bcast(&warning, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (warning) {
        printf("Heap initialization in slave failed\n");

        if (closest_dists != NULL)
            delete_heap(closest_dists);
        free(subenv_points);
        free(challengers);
        return ERROR;
    }


    // Computed distances and keep k closest
    for (int cur_chal = 0 ; cur_chal < nb_challengers ; cur_chal++) {
        for (int cur_env = 0 ; cur_env < subenv_size ; cur_env++) {
            dist = distance(subenv_points + (cur_env * dimension + 1), challengers + cur_chal * (dimension - 1), dimension - 1);
            heap_insert(dist, *(subenv_points + cur_env * dimension), closest_dists);
        }

        // Finished computation for a challenger, set results in buffer and send to master
        for (unsigned i = 0 ; i < n_selec ; i++) {
            *(results + 2 * i) = (closest_dists->nodesArray + i)->distance;
            *(results + 2 * i + 1) = (closest_dists->nodesArray + i)->class;
        }
        MPI_Send(results, 2 * n_selec, MPI_DOUBLE, MASTER, 15, MPI_COMM_WORLD);

        clear_heap(closest_dists);
    }


    // End of slave, prepare to exit
    delete_heap(closest_dists);
    free(subenv_points);
    free(challengers);

    return 0;
}
