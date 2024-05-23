/*
 * list.h
 *
 *  Created on: Dec 2, 2021
 *      Author: mathieu
 */

#ifndef PROJECT_LIST_H_
#define PROJECT_LIST_H_

int count_lines(char filename[]);

int count_columns(char filename[]);

int read_file(float* storage, char filename[], int points, int dimension);

int write_results(char filename[], float* challengers, int* classes, int nb_challengers, int dimension);

#endif /* PROJECT_LIST_H_ */
