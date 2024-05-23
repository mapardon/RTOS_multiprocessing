/*
 * heap->c
 *
 *  Created on: Dec 7, 2021
 *      Author: mathieu
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "heap.h"

#define ERROR NULL


/* Initialization of a Heap adt under the form of struct and functions. Fill the heap (capacity times) with HeapNode that have infinite
 * distance which will simplify other functions implementation. */
Heap* init_heap(unsigned capacity) {
    HeapNode *heapArray = malloc(sizeof(HeapNode) * capacity);
    Heap* heap = malloc(sizeof(heap));

    if (heapArray == NULL || heap == NULL) {
        printf("Error while allocating in heap initialization\n");
        return NULL;
    }

    for (unsigned i = 0 ; i < capacity ; i++) {
        HeapNode build;
        build.class = -1;
        build.distance = INFINITY;
        *(heapArray + i) = build;
    }

    heap->capacity = capacity;
    heap->size = 0;
    heap->nodesArray = heapArray;

    return heap;
}


/* Re-fill heap with default values */
void clear_heap(Heap *heap) {
    for (unsigned i = 0 ; i < heap->size ; i++) {
        (heap->nodesArray + i)->distance = INFINITY;
        (heap->nodesArray + i)->class = -1;
    }
    heap->size = 0;
}


/* Deallocate nodes */
void delete_heap(Heap* heap) {
    if (heap != NULL) {
        free(heap->nodesArray);
        heap->nodesArray = NULL;
        heap = NULL;
    }
}


/* Not indispensable but might be more visual. */
HeapNode* get_root(Heap* heap) {
    return heap->nodesArray;
}


/* Modify parameter heap by removing root node (reallocate to smaller heap) */
Heap* remove_root(Heap *heap) {
    unsigned new_siz = heap->size;
    Heap* new_heap = init_heap(--new_siz);

    if (new_heap != NULL) {
        for (unsigned i = 1 ; i < heap->size ; i++) {
            heap_insert((heap->nodesArray + i)->distance, (heap->nodesArray + i)->class, new_heap);
        }
    }
    delete_heap(heap);
    return new_heap;
}


/* Insertion of a value in the heap-> Test if new value has smaller distance than the one in the root (longest distance). If yes, root values
 * are updated and sink is called on the root node. if no, that means every value in the heap is already smaller than the new one. */
void heap_insert(double dist, double class, Heap* heap) {

    HeapNode ins;
    ins.distance = dist;
    ins.class = class;

    if (heap->nodesArray->distance > ins.distance) {
        if (heap->nodesArray->distance == INFINITY) ++heap->size;

        heap->nodesArray->distance = ins.distance;
        heap->nodesArray->class = ins.class;

        sink_root(heap);
    }
}


/* Executes sink procedure on the root node (rearrange the tree if a modification of root key has occurred). */
void sink_root(Heap* heap) {
    unsigned next, cur = 0, add = 0; // use indexes instead of pointers to test if we reach out of the nodes array
    double tmp_dist;
    int tmp_class;
    HeapNode *root = heap->nodesArray;

    while (cur * 2 < heap->capacity - 1) { // cur has at least 1 son
        add = 0;
        next = cur * 2;

        // Check if current node has larger son
        add = (root + next + 1)->distance > (root + cur)->distance ? 1 : 0;
        if (next < heap->capacity - 2 && (root + cur)->distance < (root + next + 2)->distance && (root + next + 1)->distance < (root + next + 2)->distance)
            add = 2;

        // If found son with higher value: swap them
        if (add) {
            next += add;

            tmp_dist = (root + cur)->distance;
            tmp_class = (root + cur)->class;

            (root + cur)->distance = (root + next)->distance;
            (root + cur)->class = (root + next)->class;

            (root + next)->distance = tmp_dist;
            (root + next)->class = tmp_class;

            cur = next;
        } else { // new node has found its place
            cur = heap->capacity;
        }
    }
}


/*
 * Search the most frequent class in the heap. Returned struct contains the class number
 * and an int indicating if it is unique (no tie).
 * * */
most_frequent_class compute_most_frequent_class(Heap *heap) {
    int min = (int) heap->nodesArray->class, max = (int) heap->nodesArray->class, tmp;

    // First search range of testable values
    for (unsigned i = 0 ; i < heap->size ; i++) {

        tmp = (heap->nodesArray + i)->class;
        if (tmp > max)
            max = tmp;
        else if (tmp < min)
            min = tmp;
    }

    // Search for the most frequent class through the heap
    most_frequent_class res;
    res.is_unique = 1;
    int cur_f = 0, best_f = 0;
    for (int cur_class = min ; cur_class <= max ; cur_class++) {
        for (unsigned i = 0 ; i < heap->size ; i++) {
            if ((int) (heap->nodesArray + i)->class == cur_class)
                cur_f++;
        }

        if (cur_f > best_f) {
            best_f = cur_f;
            res.class = cur_class;
            res.is_unique = 1;
        } else if (cur_f == best_f) {
            res.is_unique = 0;
        }
        cur_f = 0;
    }
    return res;
}





