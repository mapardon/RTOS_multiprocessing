/*
 * list.c
 *
 *  Created on: Dec 2, 2021
 *      Author: mathieu
 */

#include <stdio.h>
#include <string.h>

#include "read.h"


/*
 * Count lines in provided file in order to determine size of buffer for values.
 * Only consider non empty lines and (normally) works for files ending in line
 * feed and those who don't. Returns -1 if file opening failed.
 * */
int count_lines(char filename[]) {
	FILE *f = fopen(filename, "r");
	int lines = 0, found_char = 0, ch;

	if (f == NULL) {
	    printf("(count lines) Error occurred while opening file\n");
		return -1;
	}

	while(!feof(f)) {
	  ch = fgetc(f);

	  if (ch != '\n' && ch != -1 && !found_char) {
	      lines++;
	      found_char = 1;
	  }

	  if (ch == '\n')
		found_char = 0;
	}

	fclose(f);
	return lines;
}


/*
 * Count columns in provided file in order to determine dimension of points of the environment
 * and size of buffer to store them. Returns -1 if file opening failed.
 * */
int count_columns(char filename[]) {
    FILE *f = fopen(filename, "r");
    int columns = 0, spacing = 1, ch;

    if (f == NULL) {
        printf("(count columns) Error occurred while opening file\n");
        return -1;
    }

    do {
        ch = fgetc(f);

        if (ch != ' ' && ch != '\n' && spacing) {
            columns++;
            spacing = 0;
        } else if (ch == ' ' && !spacing) {
            spacing = 1;
        }
    } while(ch != '\n' && !feof(f));

    fclose(f);
	return columns;
}


/*
 * Read file containing points coordinates (and eventually class) and store numbers in
 * given allocated space (consider every entry as float). Returns -1 if failed to open file.
 *
 * The function takes advantage of the preliminary calculation of the number of elements
 * we should retrieve (nb of challengers * dimension of points for example) that fixes
 * problems linked to presence of white characters at then end of the file.
 * */
int read_file(float* storage, char filename[], int points, int dimension) {
    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        printf("(read file) Error occurred while opening file\n");
        return -1;
    }

    int cursor = 0;
    while(cursor < points * dimension) {
        fscanf(f, "%f", storage + cursor);
        cursor++;
    }

    fclose(f);
    return 0;
}


/* Writes results of master and slaves computations in file */
int write_results(char filename[], float* challengers, int* classes, int nb_challengers, int dimension) {
    FILE *f = fopen(filename, "w");

    if (f == NULL) {
        printf("(write results) Error occurred while opening file\n");
        return -1;
    }

    for (int chall = 0 ; chall < nb_challengers ; chall++) {
        // Write class
        fprintf(f, "%d", *(classes + chall));

        // Write coordinates of the challenger
        for (int c = 0 ; c < dimension ; c++)
            fprintf(f, " %f", *(challengers + chall * dimension + c));

        //Line feed to separate next entry (if any)
        fprintf(f, " \n");
    }

    fclose(f);
    return 0;
}









