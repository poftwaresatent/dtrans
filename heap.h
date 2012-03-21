/*
 * Copyright (c) 2012 Roland Philippsen <roland DOT philippsen AT gmx DOT net>
 *
 * BSD license:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of
 *    contributors to this software may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR THE CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DTRANS_HEAP_H
#define DTRANS_HEAP_H

#include <stdlib.h>


/**
   Internal key comparison function pointer type. Allows to use the
   heap as min heap or max heap. For convenience, just use the
   maxheap_create and minheap_create functions.
*/
typedef double (*heap_keycmp_t)(double, double);


/**
   Heap data structure object. Keys are double, and the values are
   stored as void* (you are responsible for managing the necessary
   casts and the memory of the values). Uses an array-backed tree with
   the first element stored at index 1. This wastes a bit of storage
   but makes the index arithmetic a tad easier and faster. But it
   means you have to be careful if you want to grab the first element,
   for instance in order to peek at the top of a priority queue. Use
   the HEAP_PEEK_KEY and HEAP_PEEK_VALUE macros for convenience, but
   they do not check whether the heap actually contains elements. Use
   HEAP_LENGTH or HEAP_EMPTY in case of doubt.
*/
typedef struct heap_s {
  heap_keycmp_t keycmp;
  double * key; /* first element at data[1], using length+1 storage size */
  const void ** value;
  size_t capacity;
  size_t length;
} heap_t;


/**
   Utility macro for reading the length (actual number of elements)
   of the heap.
*/
#define HEAP_LENGTH (heap) ((heap)->length)

/**
   Utility macro for determining whether a heap is empty.
*/
#define HEAP_EMPTY (heap) ((heap)->length == 0)

/**
   Utility macro for peeking at the key of the element that is at top
   of the heap. Note that the heap wastes the array index zero in
   order to keep arithmetic a bit easier, so this will return the key
   at index one.
*/
#define HEAP_PEEK_KEY (heap) ((heap)->key[1])

/**
   Utility macro for peeking at the value of the element that is at top
   of the heap. Note that the heap wastes the array index zero in
   order to keep arithmetic a bit easier, so this will return the value
   at index one.
*/
#define HEAP_PEEK_VALUE (heap) ((heap)->value[1])


/**
   Internal function used as key comparison for max heaps.
*/
double heap_keycmp_more (double lhs, double rhs);

/**
   Internal function used as key comparison for min heaps.
*/
double heap_keycmp_less (double lhs, double rhs);

/**
   Internal function for creating a heap, with a given key comparison
   function. You can just use maxheap_create or minheap_create instead
   of worrying about which internal key comparison function you have
   to pick for which kind of heap.
*/
heap_t * heap_create (size_t capacity, heap_keycmp_t keycmp);

/**
   Create and properly initialize a max heap. It is an error to pass
   capacity=0, that'll cause infinite loops.
*/
heap_t * maxheap_create (size_t capacity);

/**
   Create and properly initialize a min heap. It is an error to pass
   capacity=0, that'll cause infinite loops.
*/
heap_t * minheap_create (size_t capacity);

/**
   Destroy a heap. Properly frees any memory allocated by the heap,
   but you are still responsible for freeing up, if applicable, any of
   the values that you had stored in the heap.
*/
void heap_destroy (heap_t * heap);

/**
   Internal function for doubling the capacity of a heap. This is
   triggered when you insert an element into a heap that is full. At
   the time of writing, a heap will never deallocate any of its memory
   even if it shrinks.
*/
int heap_grow (heap_t * heap);

/**
   Internal function for swapping two elements of the heap.
*/
void heap_swap (heap_t * heap, size_t ii, size_t jj);

/**
   Internal function for adjusting the heap property after an insertion.
*/
void heap_bubble_up (heap_t * heap, size_t index);

/**
   Insert a given value into the heap at a position determined by the
   given key. The position depends on whether you have create a min
   heap or a max heap.
*/
int heap_insert (heap_t * heap, double key, const void * value);

/**
   Internal function for adjusting the heap property after a removal.
*/
void heap_bubble_down (heap_t * heap, size_t index);

/**
   Remove the topmost element, and return its value. In case you need
   to get at the key as well, you can use the HEAP_PEEK_KEY macro. If
   you need to read the topmost value without removing it, use
   HEAP_PEEK_VALUE instead. Depending on whether you have create a min
   heap or a max heap, the topmost element is either the one with the
   smallest key (min heap), or with the biggest key (max heap).
*/
void * heap_pop (heap_t * heap);

#endif
