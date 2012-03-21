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
  size_t dimx, dimy;
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
  
  usd->dimx = dimx;
  usd->dimy = dimy;
  for (ii = 0; ii < ncells; ++ii) {
    usd->dist[ii] = USDTRANS_INFINITY;
    usd->flags[ii] = USDTRANS_FLAG_UNKNOWN;
  }
  return usd;
  
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
  heap_destroy (usd->queue_negative);
  heap_destroy (usd->queue_positive);
  free (usd->flags);
  free (usd->dist);
  free (usd);
}


void usdtrans_requeue (usdtrans_t * usd, size_t index, double dist)
{
  if (usd->flags[index] & USDTRANS_FLAGS_QUEUE_POSITIVE) {
    heap_change_key (usd->queue_positive, dist, index);
  }
  else {
    usd->flags[index] |= USDTRANS_FLAGS_QUEUE_POSITIVE;
    heap_insert (usd->queue_positive, dist, index);
  }
  
  if (usd->flags[index] & USDTRANS_FLAGS_QUEUE_NEGATIVE) {
    heap_change_key (usd->queue_negative, dist, index);
  }
  else {
    usd->flags[index] |= USDTRANS_FLAGS_QUEUE_NEGATIVE;
    heap_insert (usd->queue_negative, dist, index);
  }
}


void usdtrans_seed (usdtrans_t * usd, size_t index, double dist)
{
  usd->dist[index] = dist;
  usd->flags[index] = USDTRANS_FLAG_FIXED;
  usdtrans_requeue (usd, index, dist);
}


int usdtrans_seed2 (usdtrans_t * usd, size_t ix, size_t iy, double dist)
{
  size_t index;
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
  size_t index;
  if (ix >= usd->dimx) {
    return -1;
  }
  if (iy >= usd->dimy) {
    return -1;
  }
  usdtrans_partition (usd, ix + usd->dimx * iy, dist);
  return 0;
}


#define usdtrans_get (usd, index) ((usd)->dist[(index)])

double usdtrans_get2 (usdtrans_t * usd, size_t ix, size_t iy)
{
  size_t index;
  if (ix >= usd->dimx) {
    return NAN;
  }
  if (iy >= usd->dimy) {
    return NAN;
  }
  return usdtrans_get (usd, ix + usd->dimx * iy);
}


#define usdtrans_fget (usd, index) ((usd)->flags[(index)])

int usdtrans_fget2 (usdtrans_t * usd, size_t ix, size_t iy)
{
  size_t index;
  if (ix >= usd->dimx) {
    return -1;
  }
  if (iy >= usd->dimy) {
    return -1;
  }
  return usdtrans_fget (usd, ix + usd->dimx * iy);
}


size_t usdtrans_compute_positive (usdtrans_t * usd, double maxdist)
{
}


size_t usdtrans_compute_negative (usdtrans_t * usd, double mindist)
{
}


size_t usdtrans_compute (usdtrans_t * usd, double range)
{
  return usdtrans_compute_positive (usd, range) + usdtrans_compute_negative (usd, - range);
}


int usdtrans_propagate_positive (usdtrans_t * usd)
{
}


int usdtrans_propagate_negative (usdtrans_t * usd)
{
}
