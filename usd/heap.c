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

#include "heap.h"
#include <string.h>


double heap_keycmp_more (double lhs, double rhs)
{
  return lhs - rhs;
}


double heap_keycmp_less (double lhs, double rhs)
{
  return rhs - lhs;
}


heap_t * heap_create (size_t capacity, heap_keycmp_t keycmp)
{
  heap_t * heap;
  if ( ! (heap = calloc (capacity + 1, sizeof(heap_t)))) {
    goto fail_heap;
  }
  if ( ! (heap->key = calloc (capacity + 1, sizeof(double)))) {
    goto fail_key;
  }
  if ( ! (heap->value = calloc (capacity + 1, sizeof(void*)))) {
    goto fail_value;
  }
  
  heap->keycmp = keycmp;
  heap->capacity = capacity;
  heap->length = 0;
  return heap;
  
 fail_value:
  free (heap->key);
 fail_key:
  free (heap);
 fail_heap:
  return NULL;
}


heap_t * heap_clone (heap_t * heap)
{
  heap_t * clone;
  if ( ! (clone = heap_create (heap->length, heap->keycmp))) {
    return NULL;
  }
  clone->length = heap->length;
  memcpy (clone->key + 1, heap->key + 1, heap->length * sizeof(double));
  memcpy (clone->value + 1, heap->value + 1, heap->length * sizeof(void*));
  return clone;
}


heap_t * maxheap_create (size_t capacity)
{
  return heap_create (capacity, heap_keycmp_more);
}


heap_t * minheap_create (size_t capacity)
{
  return heap_create (capacity, heap_keycmp_less);
}


void heap_destroy (heap_t * heap)
{
  free (heap->value);
  free (heap->key);
  free (heap);
}


int heap_grow (heap_t * heap)
{
  double * kk;
  void * vv;
  size_t cc;
  cc = heap->capacity * 2 + 1;	/* one extra element at the beginning... */
  if ( ! (kk = realloc (heap->key, cc * sizeof(double)))) {
    return -1;
  }
  if ( ! (vv = realloc (heap->value, cc * sizeof(void*)))) {
    return -1;
  }
  heap->capacity = cc - 1;	/* remember to remove the +1 for the extra element */
  heap->key = kk;
  heap->value = vv;
  return 0;
}


void heap_swap (heap_t * heap, size_t ii, size_t jj)
{
  double kk;
  const void * vv;
  kk = heap->key[ii];
  vv = heap->value[ii];
  heap->key[ii] = heap->key[jj];
  heap->value[ii] = heap->value[jj];
  heap->key[jj] = kk;
  heap->value[jj] = vv;
}


void heap_bubble_up (heap_t * heap, size_t index)
{
  size_t parent;
  parent = index / 2;
  while ((parent > 0)
	 && (heap->keycmp(heap->key[index], heap->key[parent]) > 0.0))
    {
      heap_swap (heap, index, parent);
      index = parent;
      parent = index / 2;
    }
}


int heap_insert (heap_t * heap, double key, const void * value)
{
  if (heap->length == heap->capacity) {
    if (0 != heap_grow (heap)) {
      return -1;
    }
  }
  ++heap->length; /* remember, arrays start at index 1 to simplify arithmetic */
  heap->key[heap->length] = key;
  heap->value[heap->length] = value;
  heap_bubble_up (heap, heap->length);
  return 0;
}


size_t heap_find_element (heap_t * heap, double key, const void * value, size_t root)
{
  size_t subtree, guess;
  
  if (root > heap->length) {
    return 0;			/* out of bounds */
  }
  
  if (heap->keycmp(heap->key[root], key) < 0.0) {
    return 0;			/* key cannot be in this subtree due to heap property */
  }
  
  if ((heap->key[root] == key)
      && (heap->value[root] == value))
    {
      return root;		/* found */
    }
  
  subtree = 2 * root;
  guess = heap_find_element (heap, key, value, subtree);
  if (0 != guess) {
    return guess;		/* found in left subtree */
  }
  
  ++subtree;
  return heap_find_element (heap, key, value, subtree); /* try right subtree */
}


int heap_change_key (heap_t * heap, double old_key, double new_key, const void * value)
{
  size_t index;
  
  index = heap_find_element (heap, old_key, value, 1);
  if (0 == index) {
    return -1;			/* no such element */
  }
  
  if (heap->keycmp == heap_keycmp_more) {
    /* this is a max heap */
    heap->key[index] = new_key;
    if (new_key > old_key) {
      heap_bubble_up (heap, index);
    }
    else if (new_key < old_key) {
      heap_bubble_down (heap, index);
    }
    /* else no change */
    return 0;
  }
  
  else if (heap->keycmp == heap_keycmp_less) {
    /* this is a min heap */
    heap->key[index] = new_key;
    if (new_key < old_key) {
      heap_bubble_up (heap, index);
    }
    else if (new_key > old_key) {
      heap_bubble_down (heap, index);
    }
    /* else no change */
    return 0;
  }
  
  /* else this is an unsopprted kind of heap */
  return -2;
}


void heap_bubble_down (heap_t * heap, size_t index)
{
  size_t left, right, target;
  target = index;
  for (;;) {
    left = 2 * index;
    right = left + 1;
    if ((left <= heap->length)
	&& (heap->keycmp(heap->key[left], heap->key[target]) > 0.0))
      {
	target = left;
      }
    if ((right <= heap->length)
	&& (heap->keycmp(heap->key[right], heap->key[target]) > 0.0))
      {
	target = right;
      }
    
    if (target == index) {
      return;
    }
    
    heap_swap (heap, target, index);
    index = target;
  }
}


void * heap_pop (heap_t * heap)
{
  const void * vv;
  if (0 == heap->length) {
    return NULL;
  }
  vv = heap->value[1];
  heap->key[1] = heap->key[heap->length];
  heap->value[1] = heap->value[heap->length];
  --heap->length;
  heap_bubble_down (heap, 1);
  return (void*) vv;
}
