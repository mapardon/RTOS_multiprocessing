/*
 * master.c
 *
 *  Created on: Dec 2, 2021
 *      Author: mathieu
 */

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>

#include "master.h"
#include "read.h"
#include "heap.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ERROR -1



int master(int world_size, char environment_file[], char challengers_file[], int k, char out_file[]) {
    MPI_Status status;

    /* ** Initializations ** */

    // Fetch dimension of points, number of environment points and number of challenger points
    int env_size = count_lines(environment_file),
        nb_challengers = count_lines(challengers_file),
        env_data_size = count_columns(environment_file);  // includes point coordinates AND its class


    // Allocate space for reading environment/challengers points (will be used lower)
    float *env_points_buf = (float*) malloc(env_data_size * env_size * sizeof(float)),
          *chall_points_buf = (float*) malloc((env_data_size - 1) * nb_challengers * sizeof(float));


    // Check if error occurred
    int code = (chall_points_buf == NULL || env_points_buf == NULL || env_size < 1 || nb_challengers < 1 || env_data_size == -1) ? -1 : 0;


    // Store data from files
    if (!code) {
        code = read_file(env_points_buf, environment_file, env_size, env_data_size);
        code += read_file(chall_points_buf, challengers_file, env_size, env_data_size - 1);
    }


    // Notify slaves if operations went well
    MPI_Bcast(&code, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (code) {
        if (!env_size)
            printf("Environment file is empty\n");
        else if (!nb_challengers)
            printf("Challengers file is empty\n");
        else
            printf("An error occurred during master initializations\n");

        if (chall_points_buf != NULL)
            free(chall_points_buf);
        if (env_points_buf != NULL)
            free(env_points_buf);
        return ERROR;
    }


    // Send points dimension to other nodes
    MPI_Bcast(&env_data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Send number of challengers
    MPI_Bcast(&nb_challengers, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Determine how many environment points each node will receive and send them
    int regular_set = env_size / (world_size - 1), bonus_set = regular_set + 1, bonus_nodes = env_size % (world_size - 1);

    // nodes that receive one extra point for computation
    for (int i = 1 ; i <= bonus_nodes ; i++) {
        MPI_Send(&bonus_set, 1, MPI_INT, i, 15, MPI_COMM_WORLD);
    }
    // those who have regular set
    for (int i = (bonus_nodes + 1) ; i < world_size ; i++) {
        MPI_Send(&regular_set, 1, MPI_INT, i, 15, MPI_COMM_WORLD);
    }


    // Check if slaves have successfully allocated space
    int sentinel = 0, check;
    for (int i = 1 ; i < world_size ; i++) {
        MPI_Recv(&check, 1, MPI_INT, i, 15, MPI_COMM_WORLD, &status);
        sentinel += check;
    }
    int can_continue = !sentinel;


    // Notify slaves whether they can continue
    for (int i = 1 ; i < world_size ; i++)
        MPI_Send(&can_continue, 1, MPI_INT, i, 15, MPI_COMM_WORLD);

    if (!can_continue) {  // at least 1 slave had trouble allocating
        free(env_points_buf);
        free(chall_points_buf);
        printf("An error occurred during slaves memory allocation\n");
        return ERROR;
    }


    // Send challengers to slaves
    MPI_Bcast(chall_points_buf, nb_challengers * (env_data_size - 1), MPI_FLOAT, 0, MPI_COMM_WORLD);


    // Send environment points to slaves
    // bonus set
    for (int i = 1 ; i <= bonus_nodes ; i++) {
        MPI_Send(env_points_buf + bonus_set * env_data_size * (i - 1), bonus_set * env_data_size, MPI_INT, i, 15, MPI_COMM_WORLD);
    }
    // regular set
    int shift;
    for (int i = (bonus_nodes + 1) ; i < world_size ; i++) {
        shift = bonus_set * env_data_size * bonus_nodes + regular_set * env_data_size * (i - bonus_nodes - 1);
        MPI_Send(env_points_buf + shift, regular_set * env_data_size, MPI_FLOAT, i, 15, MPI_COMM_WORLD);
    }
    free(env_points_buf);


    // Allocate space for computation results
    double *slaves_computed_res = (double*) malloc(sizeof(double) * 2 * nb_challengers * (MIN(k, regular_set) * (world_size - bonus_nodes - 1) + MIN(k, bonus_set) * bonus_nodes));
    int *best_classes = (int*) malloc(sizeof(int) * nb_challengers);

    code = slaves_computed_res == NULL || best_classes == NULL;
    MPI_Bcast(&code, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (code) {
        printf("An error occurred during master memory allocation\n");
        if (slaves_computed_res != NULL)
            free(slaves_computed_res);
        if (best_classes != NULL)
            free(best_classes);
        free(chall_points_buf);
        return ERROR;
    }


    // Check if slaves managed to create their heap
    int dummy_sendv = 0;
    MPI_Reduce(&dummy_sendv, &code, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    Heap *closest_dists = NULL;

    if (!code) {  // went well, create own heap
        closest_dists = init_heap(MIN(k, env_size));
        code = closest_dists == NULL;
    }

    // Send confirmation
    MPI_Bcast(&code, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (code) {  // something failed
        printf("An error occurred while creating heaps\n");

        if (closest_dists != NULL)
            delete_heap(closest_dists);
        free(chall_points_buf);
        free(slaves_computed_res);
        free(best_classes);
        return ERROR;
    }


    // Receive slaves computations
    int nb_rec;  // how many computed distances each slave will return
    shift = 0;
    for (int cur_chal = 0 ; cur_chal < nb_challengers ; cur_chal++) {
        for (int i = 1 ; i < world_size ; i++) {
            nb_rec = MIN(k, i > bonus_nodes ? regular_set : bonus_set);
            MPI_Recv(slaves_computed_res + shift, 2 * nb_rec, MPI_DOUBLE, i, 15, MPI_COMM_WORLD, &status);
            shift += 2 * nb_rec;
        }
    }


    // Complete number of results (dist, class) received per challenger (from all slaves)
    int tot_rec = MIN(k, regular_set) * (world_size - bonus_nodes - 1) + MIN(k, bonus_set) * bonus_nodes;

    // Computations received, determine k closest classes
    most_frequent_class best;
    for (int chall = 0 ; chall < nb_challengers ; chall++) {
        for (int offset = 0 ; offset < tot_rec ; offset++) {
            shift = 2 * (chall * tot_rec + offset);
            heap_insert(*(slaves_computed_res + shift), *(slaves_computed_res + shift + 1), closest_dists);
        }

        // Determine highest frequency class
        best = compute_most_frequent_class(closest_dists);
        while (!best.is_unique && closest_dists != NULL) {
            closest_dists = remove_root(closest_dists);

            if (closest_dists != NULL)
                best = compute_most_frequent_class(closest_dists);
        }
        *(best_classes + chall) = best.class;

        clear_heap(closest_dists);
    }

    // From here, no need to notify slave if error occurred (they are already finished), just display message on screen
    if (closest_dists == NULL) {
        printf("Error occurred while computing highest frequency class\n");

    } else {  // Ties fixed, write down
        code = write_results(out_file, chall_points_buf, best_classes, nb_challengers, env_data_size - 1);

        if (code)
            printf("Error occurred while trying to write results in file\n");
    }


    // End of main, prepare to exit
    delete_heap(closest_dists);
    free(chall_points_buf);
    free(slaves_computed_res);
    free(best_classes);

    return 0;
}
