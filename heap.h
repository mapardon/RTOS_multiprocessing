/*
 * heap.h
 *
 *  Created on: Dec 7, 2021
 *      Author: mathieu
 */

#ifndef PROJECT_HEAP_H_
#define PROJECT_HEAP_H_

/* Those structs are meant to store the result of the computation of a distance between a challenger node and a node of the environment. Storing
 * distance and class will allow to determine which are the classes of the closest nodes of the challenger. The heap will always keep the node
 * with the greatest distance in the root. */

typedef struct {
    double distance, class;
} HeapNode;

typedef struct {
    unsigned size, capacity;
    HeapNode* nodesArray;
} Heap;

Heap* init_heap(unsigned capacity);

void clear_heap(Heap *heap);

void delete_heap(Heap* heap);

Heap* remove_root(Heap *heap);

HeapNode* get_root();

void heap_insert(double dist, double class, Heap* heap);

void sink_root(Heap* heap);

typedef struct {
    int class, is_unique;
} most_frequent_class;

most_frequent_class compute_most_frequent_class(Heap *heap);

#endif /* PROJECT_HEAP_H_ */
