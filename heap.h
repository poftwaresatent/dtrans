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
#define HEAP_LENGTH(heap) ((heap)->length)

/**
   Utility macro for determining whether a heap is empty.
*/
#define HEAP_EMPTY(heap) ((heap)->length == 0)

/**
   Utility macro for peeking at the key of the element that is at top
   of the heap. Note that the heap wastes the array index zero in
   order to keep arithmetic a bit easier, so this will return the key
   at index one.
*/
#define HEAP_PEEK_KEY(heap) ((heap)->key[1])

/**
   Utility macro for peeking at the value of the element that is at top
   of the heap. Note that the heap wastes the array index zero in
   order to keep arithmetic a bit easier, so this will return the value
   at index one.
*/
#define HEAP_PEEK_VALUE(heap) ((heap)->value[1])


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
   
   \return A freshly created heap, or NULL in case of failure (which
   stems from calloc, so you can use errno to get a system error
   message).
*/
heap_t * heap_create (size_t capacity, heap_keycmp_t keycmp);

/**
   Create a clone of a given heap. The capacity of the clone will be
   exactly the length of the original.
   
   \return A freshly allocated clone with all keys and data properly
   copied, or NULL on failure (in which case you can use errno to see
   what it's about, probably out of memory from calloc).
*/
heap_t * heap_clone (heap_t * heap);

/**
   Create and properly initialize a max heap. It is an error to pass
   capacity=0, that'll cause infinite loops.
   
   \return A freshly created max heap, or NULL in case of failure
   (which stems from calloc, so you can use errno to get a system
   error message).
*/
heap_t * maxheap_create (size_t capacity);

/**
   Create and properly initialize a min heap. It is an error to pass
   capacity=0, that'll cause infinite loops.
   
   \return A freshly created min heap, or NULL in case of failure
   (which stems from calloc, so you can use errno to get a system
   error message).
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

   \return 0 on success, -1 on failure (which stems from realloc, so
   you can use errno to get a system error message).
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
   
   \return 0 on success, or -1 on failure (which stems from a failed
   heap_grow, thus from realloc, and thus you can use errno).
*/
int heap_insert (heap_t * heap, double key, const void * value);

/**
   Internal (recursive) function to find an element with a given key
   and value in the subtree rooted at the given location. Used by
   heap_change_key.
   
   \return The index of the sought element, or zero in case of failure
   (remember, the array storage starts at index one, so a zero can be
   used to signify "not found" in this implementation).
*/
size_t heap_find_element (heap_t * heap, double key, const void * value, size_t root);

/**
   Modify the key of an element already stored in the heap. You need
   to specify the old key under which the value is currently stored,
   in order to allow finding the element. After the element is found
   and the key changed, the appropriate up or down bubble operation is
   performed to restore the heap property.
   
   \note Both the old_key and the value have to match, and the
   implementation uses a straightforward pointer comparison on the
   value. So beware of heaps that store strings or similar possibly
   identical duplicates for the value.
   
   \return 0 on success, -1 if the given key-value pair is not in the
   heap, and -2 if there is some problem detecting whether this is a
   max heap or a min heap. The latter is either a bug, or you have not
   used maxheap_create or minheap_create to create this heap.
*/
int heap_change_key (heap_t * heap, double old_key, double new_key, const void * value);

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
   
   \return A pointer to the value that used to be on top, or NULL in
   case the heap was empty (or maybe you had actually put a zero on
   there somewhere).
*/
void * heap_pop (heap_t * heap);

#endif
