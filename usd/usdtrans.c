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
#include <math.h>

#include <stdio.h>
#include <err.h>


#define USDTRANS_INFINITY 1.0e9

#define USDTRANS_FLAG_FIXED          0x01
#define USDTRANS_FLAG_UNKNOWN        0x02
#define USDTRANS_FLAG_QUEUE_POSITIVE 0x04
#define USDTRANS_FLAG_QUEUE_NEGATIVE 0x08


typedef struct usdtrans_s {
  double * dist;
  int * flags;
  heap_t * queue_positive;
  heap_t * queue_negative;
  heap_t * propagators;
  size_t dimx, dimy, ncells, toprow, rightcol;
} usdtrans_t;


usdtrans_t * usdtrans_create (size_t dimx, size_t dimy)
{
  usdtrans_t * usd;
  size_t ncells;
  size_t ii;
  
  if ( ! (usd = calloc (1, sizeof(usdtrans_t)))) {
    goto fail_usd;
  }
  ncells = dimx * dimy;
  if ( ! (usd->dist = calloc (ncells, sizeof(double)))) {
    goto fail_dist;
  }
  if ( ! (usd->flags = calloc (ncells, sizeof(int)))) {
    goto fail_flags;
  }
  if ( ! (usd->queue_positive = maxheap_create (dimx + dimy))) {
    goto fail_qp;
  }
  if ( ! (usd->queue_negative = minheap_create (dimx + dimy))) {
    goto fail_qn;
  }
  if ( ! (usd->propagators = minheap_create (4))) {
    goto fail_props;
  }
  
  usd->dimx = dimx;
  usd->dimy = dimy;
  usd->ncells = ncells;
  usd->toprow = ncells - dimx;
  usd->rightcol = dimx - 1;
  for (ii = 0; ii < ncells; ++ii) {
    usd->dist[ii] = USDTRANS_INFINITY;
    usd->flags[ii] = USDTRANS_FLAG_UNKNOWN;
  }
  return usd;
  
 fail_props:
  heap_destroy (usd->queue_negative);
 fail_qn:
  heap_destroy (usd->queue_positive);
 fail_qp:
  free (usd->flags);
 fail_flags:
  free (usd->dist);
 fail_dist:
  free (usd);
 fail_usd:
  return NULL;
}


void usdtrans_destroy (usdtrans_t * usd)
{
  heap_destroy (usd->propagators);
  heap_destroy (usd->queue_negative);
  heap_destroy (usd->queue_positive);
  free (usd->flags);
  free (usd->dist);
  free (usd);
}


void usdtrans_requeue (usdtrans_t * usd, size_t index, double new_dist)
{
  if (usd->flags[index] & USDTRANS_FLAG_QUEUE_POSITIVE) {
    heap_change_key (usd->queue_positive, usd->dist[index], new_dist, (void*) index);
  }
  else {
    usd->flags[index] &= ~USDTRANS_FLAG_UNKNOWN;
    usd->flags[index] |= USDTRANS_FLAG_QUEUE_POSITIVE;
    heap_insert (usd->queue_positive, new_dist, (void*) index);
  }
  
  if (usd->flags[index] & USDTRANS_FLAG_QUEUE_NEGATIVE) {
    heap_change_key (usd->queue_negative, usd->dist[index], new_dist, (void*) index);
  }
  else {
    usd->flags[index] &= ~USDTRANS_FLAG_UNKNOWN;
    usd->flags[index] |= USDTRANS_FLAG_QUEUE_NEGATIVE;
    heap_insert (usd->queue_negative, new_dist, (void*) index);
  }
  
  usd->dist[index] = new_dist;
}


void usdtrans_seed (usdtrans_t * usd, size_t index, double dist)
{
  usd->flags[index] |= USDTRANS_FLAG_FIXED;
  usdtrans_requeue (usd, index, dist);
}


int usdtrans_seed2 (usdtrans_t * usd, size_t ix, size_t iy, double dist)
{
  if (ix >= usd->dimx) {
    return -1;
  }
  if (iy >= usd->dimy) {
    return -1;
  }
  usdtrans_seed (usd, ix + usd->dimx * iy, dist);
  return 0;
}


void usdtrans_partition (usdtrans_t * usd, size_t index, double dist)
{
  usd->dist[index] = dist;
  usd->flags[index] = USDTRANS_FLAG_UNKNOWN;
}


int usdtrans_partition2 (usdtrans_t * usd, size_t ix, size_t iy, double dist)
{
  if (ix >= usd->dimx) {
    return -1;
  }
  if (iy >= usd->dimy) {
    return -1;
  }
  usdtrans_partition (usd, ix + usd->dimx * iy, dist);
  return 0;
}


#define usdtrans_get(usd, index) ((usd)->dist[(index)])

double usdtrans_get2 (usdtrans_t * usd, size_t ix, size_t iy)
{
  if (ix >= usd->dimx) {
    return NAN;
  }
  if (iy >= usd->dimy) {
    return NAN;
  }
  return usdtrans_get (usd, ix + usd->dimx * iy);
}


#define usdtrans_fget(usd, index) ((usd)->flags[(index)])

int usdtrans_fget2 (usdtrans_t * usd, size_t ix, size_t iy)
{
  if (ix >= usd->dimx) {
    return -1;
  }
  if (iy >= usd->dimy) {
    return -1;
  }
  return usdtrans_fget (usd, ix + usd->dimx * iy);
}


void usdtrans_update_positive (usdtrans_t * usd, size_t index)
{
  size_t nbor, ix, primary_index;
  int northsouth;
  double primary_dist, p2, rhs;
  
  if (usd->flags[index] & USDTRANS_FLAG_FIXED) {
    return;
  }
  
  usd->propagators->length = 0;
  if (index >= usd->dimx) {
    nbor = index - usd->dimx;	/* try south */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, usd->dist[nbor], (void*) nbor);
    }
  }
  if (index < usd->toprow) {
    nbor = index + usd->dimx;	/* try north */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, usd->dist[nbor], (void*) nbor);
    }
  }
  ix = index % usd->dimx;
  if (ix > 0) {
    nbor = index - 1;		/* try west */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, usd->dist[nbor], (void*) nbor);
    }
  }
  if (ix < usd->rightcol) {
    nbor = index + 1;		/* try east */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, usd->dist[nbor], (void*) nbor);
    }
  }
  
  if (0 == usd->propagators->length) {
    return;			/* "never happens" though */
  }
  
  primary_dist = usd->propagators->key[1];
  p2 = pow (primary_dist, 2.0);
  primary_index = (size_t) usd->propagators->value[1];
  northsouth = (ix == (primary_index % usd->dimx));
  heap_pop (usd->propagators);
  
  for (/**/; 0 != usd->propagators->length; heap_pop (usd->propagators)) {
    const size_t secondary_index = (size_t) usd->propagators->value[1];
    if (northsouth ^ (ix == (secondary_index % usd->dimx))) {
      const double secondary_dist = usd->propagators->key[1];
      if (1.0 > secondary_dist - primary_dist) {
	const double bb = primary_dist + secondary_dist;
	const double cc = (p2 + pow (secondary_dist, 2.0) - 1.0) / 2.0;
	const double root = pow (bb, 2.0) - 4.0 * cc;
	rhs = (bb + sqrt (root)) / 2.0;
	if (rhs < usd->dist[index]) {
	  usdtrans_requeue (usd, index, rhs);
	  return;
	}
      }
    }
  }
  
  rhs = primary_dist + 1.0;
  if (rhs < usd->dist[index]) {
    usdtrans_requeue (usd, index, rhs);
  }
}


void usdtrans_update_negative (usdtrans_t * usd, size_t index)
{
  size_t nbor, ix, primary_index;
  int northsouth;
  double primary_dist, p2, rhs;
  
  if (usd->flags[index] & USDTRANS_FLAG_FIXED) {
    return;
  }
  
  usd->propagators->length = 0;
  if (index >= usd->dimx) {
    nbor = index - usd->dimx;	/* try south */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, - usd->dist[nbor], (void*) nbor);
    }
  }
  if (index < usd->toprow) {
    nbor = index + usd->dimx;	/* try north */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, - usd->dist[nbor], (void*) nbor);
    }
  }
  ix = index % usd->dimx;
  if (ix > 0) {
    nbor = index - 1;		/* try west */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, - usd->dist[nbor], (void*) nbor);
    }
  }
  if (ix < usd->rightcol) {
    nbor = index + 1;		/* try east */
    if ( ! (usd->flags[nbor] & USDTRANS_FLAG_UNKNOWN)) {
      heap_insert (usd->propagators, - usd->dist[nbor], (void*) nbor);
    }
  }
  
  if (0 == usd->propagators->length) {
    return;			/* "never happens" though */
  }
  
  primary_dist = usd->propagators->key[1];
  p2 = pow (primary_dist, 2.0);
  primary_index = (size_t) usd->propagators->value[1];
  northsouth = (ix == (primary_index % usd->dimx));
  heap_pop (usd->propagators);
  
  for (/**/; 0 != usd->propagators->length; heap_pop (usd->propagators)) {
    const size_t secondary_index = (size_t) usd->propagators->value[1];
    if (northsouth ^ (ix == (secondary_index % usd->dimx))) {
      const double secondary_dist = usd->propagators->key[1];
      if (1.0 > secondary_dist - primary_dist) {
	const double bb = primary_dist + secondary_dist;
	const double cc = (p2 + pow (secondary_dist, 2.0) - 1.0) / 2.0;
	const double root = pow (bb, 2.0) - 4.0 * cc;
	rhs = - (bb + sqrt (root)) / 2.0;
	if (rhs > usd->dist[index]) {
	  usdtrans_requeue (usd, index, rhs);
	  return;
	}
      }
    }
  }
  
  rhs = - primary_dist - 1.0;
  if (rhs > usd->dist[index]) {
    usdtrans_requeue (usd, index, rhs);
  }
}


void usdtrans_propagate_positive (usdtrans_t * usd)
{
  size_t index, ix;
  index = (size_t) heap_pop (usd->queue_positive);
  if (index >= usd->dimx) {
    usdtrans_update_positive (usd, index - usd->dimx); /* south */
  }
  if (index < usd->toprow) {
    usdtrans_update_positive (usd, index + usd->dimx); /* north */
  }
  ix = index % usd->dimx;
  if (ix > 0) {
    usdtrans_update_positive (usd, index - 1); /* west */
  }
  if (ix < usd->rightcol) {
    usdtrans_update_positive (usd, index + 1); /* east */
  }
}


void usdtrans_propagate_negative (usdtrans_t * usd)
{
  size_t index, ix;
  index = (size_t) heap_pop (usd->queue_negative);
  if (index >= usd->dimx) {
    usdtrans_update_negative (usd, index - usd->dimx); /* south */
  }
  if (index < usd->toprow) {
    usdtrans_update_negative (usd, index + usd->dimx); /* north */
  }
  ix = index % usd->dimx;
  if (ix > 0) {
    usdtrans_update_negative (usd, index - 1); /* west */
  }
  if (ix < usd->rightcol) {
    usdtrans_update_negative (usd, index + 1); /* east */
  }
}


void usdtrans_compute_positive (usdtrans_t * usd, double maxdist)
{
  while (0 != usd->queue_positive->length) {
    if (usd->queue_positive->key[1] > maxdist) {
      break;
    }
    usdtrans_propagate_positive (usd);
  }
}


void usdtrans_compute_negative (usdtrans_t * usd, double mindist)
{
  while (0 != usd->queue_negative->length) {
    if (usd->queue_negative->key[1] < mindist) {
      break;
    }
    usdtrans_propagate_negative (usd);
  }
}


void usdtrans_compute (usdtrans_t * usd, double range)
{
  usdtrans_compute_positive (usd, range);
  usdtrans_compute_negative (usd, -range);
}


static void pnum6 (FILE * fp, double num)
{
  if (isinf(num)) {
    fprintf(fp, "   inf");
  }
  else if (isnan(num)) {
    fprintf(fp, "   nan");
  }
  else if (fabs(fmod(num, 1)) < 1e-6) {
    fprintf(fp, " % 3d  ", (int) rint(num));
  }
  else {
    fprintf(fp, " % 5.1f", num);
  }
}


static void dump (usdtrans_t * usd)
{
  size_t ix, iy;
  for (iy = usd->dimy - 1; iy < usd->dimy; --iy) { /* until oflo */
    for (ix = 0; ix < usd->dimx; ++ix) {
      pnum6 (stdout, usd->dist[ix + usd->dimx * iy]);
    }
    fprintf (stdout, "\n");
  }
}


int main (int argc, char ** argv)
{
  usdtrans_t * usd;
  size_t ix, iy;
  if ( ! (usd = usdtrans_create (5, 10))) {
    err (EXIT_FAILURE, "failed to create usd\n");
  }
  for (ix = 0; ix < usd->dimx; ++ix) {
    for (iy = 0; iy < usd->dimy / 2; ++iy) {
      usdtrans_partition2 (usd, ix, iy, -100.0);
    }
    for (iy = usd->dimy / 2; iy < usd->dimy; ++iy) {
      usdtrans_partition2 (usd, ix, iy,  100.0);
    }
    for (iy = usd->dimy / 2; iy < 2 + usd->dimy / 2; ++iy) {
      usdtrans_seed2 (usd, ix, iy, iy - usd->dimy / 2 - 0.5);
    }
  }
  dump (usd);
  usdtrans_destroy (usd);
  return 0;
}
