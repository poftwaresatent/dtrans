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
#include <stdio.h>
#include <err.h>


typedef struct test_input_s {
  double key;
  const char * value;
} test_input_t;


static void test_dump (heap_t * heap, const char * prefix)
{
  size_t ii;
  fprintf (stderr, "%sheap with %zu of %zu items:\n", prefix, heap->length, heap->capacity);
  for (ii = 1; ii <= heap->length; ++ii) {
    fprintf (stderr, "%s  % 5.2f\t%s\n", prefix, heap->key[ii], (char*) heap->value[ii]);
  }
}


static heap_t * test_create (test_input_t * data, heap_t * (*creator)(size_t))
{
  size_t ii, capacity;
  heap_t * heap;
  
  fprintf (stderr, "scanning input data:\n");
  for (ii = 0; data[ii].value; ++ii) {
    fprintf (stderr, "  % 5.2f\t%s\n", data[ii].key, data[ii].value);
  }
  fprintf (stderr, "there seem to be %zu data items\n", ii);
  
  capacity = ii / 2 + 1;
  fprintf (stderr, "creating heap with capacity %zu\n", capacity);
  heap = creator (capacity);
  if ( ! heap) {
    err (EXIT_FAILURE, "maxheap_create");
  }
  
  fprintf (stderr, "inserting data into heap\n");
  for (ii = 0; data[ii].value; ++ii) {
    fprintf (stderr, "  inserting item %zu (% 5.2f\t%s)\n", ii, data[ii].key, data[ii].value);
    heap_insert (heap, data[ii].key, data[ii].value);
    test_dump (heap, "    ");
  }
  
  return heap;
}


static void test_enumerate (heap_t * heap)
{
  heap_t * clone;
  clone = heap_clone (heap);
  while (clone->length > 0) {
    fprintf (stderr, "% 5.2f\t%s\n",
	     HEAP_PEEK_KEY (clone), (char*) HEAP_PEEK_VALUE (clone));
    heap_pop (clone);
  }
}


static int suite (heap_t * (*creator)(size_t))
{
  static test_input_t data[] = {
    {  12.0, "hello world" },
    { -13.0, "too much negativity" },
    {  42.9, "byebye universe" },
    {   0.0, NULL }};
  
  heap_t * heap;
  
  if (NULL == (heap = test_create (data, creator))) {
    fprintf (stderr, "OOPS: creation failed\n");
    return -1;
  }
  fprintf (stderr, "\nafter creation:\n");
  test_enumerate (heap);
  
  fprintf (stderr, "\nlet's modify some existing and bogus elements...\n");
  if (0 != heap_change_key (heap, data[0].key, -22.0, data[0].value)) {
    fprintf (stderr, "OOPS: that should have succeeded!\n");
    return -2;
  }
  if (0 == heap_change_key (heap, 888.999, -1.0, data[0].value)) {
    fprintf (stderr, "OOPS: invalid old_key should have failed!\n");
    return -3;
  }
  if (0 == heap_change_key (heap, data[1].key, 22000.3, "blah")) {
    fprintf (stderr, "OOPS: invalid value have failed!\n");
    return -4;
  }
  fprintf (stderr, "\nafter changing a key:\n");
  test_enumerate (heap);
  
  heap_destroy (heap);
  return 0;
}


int main (int argc, char ** argv)
{
  size_t nfail;
  nfail = 0;
  
  fprintf (stderr, "\nlet's try max heap first...\n\n");
  if (0 != suite (maxheap_create)) {
    ++nfail;
  }
  
  fprintf (stderr, "\nand how about min heap?\n\n");
  if (0 != suite (minheap_create)) {
    ++nfail;
  }
  
  if (nfail == 1) {
    fprintf (stderr, "\nOOPS there was a failure\n");
    return 1;
  }
  else if (nfail != 0) {
    fprintf (stderr, "\nOOPS there were %zu failures\n", nfail);
    return 1;
  }
  
  fprintf (stderr, "\nall tests passed\n");
  return 0;
}
