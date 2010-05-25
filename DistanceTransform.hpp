/*
 * Copyright (c) 2010 Roland Philippsen <roland DOT philippsen AT gmx DOT net>
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

#ifndef DTRANS_DISTANCE_TRANSFORM_HPP
#define DTRANS_DISTANCE_TRANSFORM_HPP

#include <vector>
#include <map>
#include <string>
#include <stdio.h>


/** \namespace dtrans Contains all distance transformation entities (classes, functions, etc). */

namespace dtrans {
  
  
  /**
     Compute the distance to some initial level set throughout a
     two-dimensional grid.
   */  
  class DistanceTransform
  {
  public:
    /** A (very large) positive number that will be considered
	equivalent to infinity by the distance transform. */
    static double const infinity;
    
    /** A two-dimensional grid of cells, each of which stores its
	distance to some initial level set. Plus some auxiliary data
	and methods to propagate the distance transform out from the
	initial set. */
    DistanceTransform(/** Dimension (number of cells) along the X direction. */
		      size_t dimx,
		      /** Dimension (number of cells) along the Y direction. */
		      size_t dimy,
		      /** Scale of the cells: the length of one side
			  of one cell is considered to be these many
			  units. E.g. if the scale=0.1 then it will
			  take 10 cells for the distance to grow by
			  1. */
		      double scale);
    
    /** Check if a grid index is valid.
	
	\return True if the given grid coordinates are valid
	(i.e. they lie within the grid dimensions specified at
	construction time). */
    inline bool isValid(size_t ix, size_t iy) const
    { return (ix < m_dimx) && (iy < m_dimy); }
    
    /** Dimension along X.
	
	\return The number of cells along the X direction. */
    inline size_t dimX() const { return m_dimx; }
    
    /** Dimension along Y.
	
	\return The number of cells along the Y direction. */
    inline size_t dimY() const { return m_dimy; }
    
    /** Set a given cell (given by its X and Y index) to a certain
	distance. Cells whose distance is set in this manner will be
	used to seed the distance transform computation. The
	propagation will not overwrite a cell's distance value if it
	has been set using this method.

	\return True if the cell's distance value has been set
	(i.e. the given coordinates lie within the grid).
    */
    bool set(size_t ix, size_t iy, double dist);
    
    /** Get the distance of a cell.
	
	\return The distance value of a cell (given by its X and Y
	index). If the cell is invalid (i.e. it lies outside the
	grid), then DistanceTransform::infinity is returned. */
    double get(size_t ix, size_t iy) const;
    
    /** Propagate the distance transform until a maximum distance has
	been reached or the entire grid has been updated. Repeatedly
	calls propagate() until the top of the queue lies above the
	given ceiling, or the queue is empty.
	
	\note You can call compute() again with a higher ceiling and
	it will keep on propagating where it left off. That way you
	can compute the distance transform in several steps, or
	adaptively change the ceiling depending on whether a certain
	cell you are interested in has been reached yet.
	
	\note Simply pass dtrans::DistanceTransform::infinity as
	ceiling if you want to make sure that the entire grid gets
	computed.
    */
    void compute(/** distance up to which the transform should be
		     propagated (say DistanceTransform::infinity for
		     no limit) */
		 double ceiling);
    
    /** Debugging version of compute(). It does the same propagation,
	and writes information about what it is doing at each
	iteration. */
    void compute(double ceiling, FILE * dbg_fp, std::string const & dbg_prefix);
    
    /** Perform one cell expansion. If the queue is empty, it does
	nothing.
	
	\return true if it computed something, false otherwise
	(i.e. when the queue was empty). */
    bool propagate();
    
    /** Peek at the next-to-be propagated value.
	
	\return The key of the cell at the top of the queue, or
	+infinity in case the queue is empty. */
    double getTopKey() const;
    
    /** Compute some (simple) statistics over the current state of the
	grid and its associated queue.
	
	\note You can detect whether any valid stats are available by
	checking that the returned max is higher than the returned
	min. */
    void stat(/** minimum distance value (including one-step
		  lookahead) or +infinity in case no cell lies below
		  infinity */
	      double & minval,
	      /** maximum distance value (including one-step
		  lookahead) or -infinity in case no cell lies below
		  infinity */
	      double & maxval,
	      /** minimum queue key, or +infinity in case the queue is
		  empty */
	      double & minkey,
	      /** maximum queue key, or -infinity in case the queue is
		  empty */
	      double & maxkey) const;
    
    /** Write the current state of the grid, the one-step-lookahead,
	as well as the queue key map in a (more or less)
	human-readable format to the given FILE stream. Each line gets
	prefixed with a user-defined string. */
    void dump(FILE * fp, std::string const & prefix) const;
    
    /** Write the current queue in a more or less human-readable
	format to the provided FILE stream, prefixing each line with
	the given user-defined string. */
    void dumpQueue(FILE * fp, std::string const & prefix) const;
    
  protected:
    typedef std::multimap<double, size_t> queue_t;
    typedef queue_t::iterator queue_it;
    typedef queue_t::const_iterator queue_cit;
    
    size_t const m_dimx;
    size_t const m_dimy;
    size_t const m_ncells;
    size_t const m_toprow;
    size_t const m_rightcol;
    double const m_scale;
    double const m_scale2;
    std::vector<double> m_value; /**< distance map, negative values mean "fixed cell" */
    std::vector<double> m_rhs;	 /**< one-step look-ahead map, negative values mean "fixed cell" */
    std::vector<double> m_key;	 /**< map of queue keys, a -1 means "not on queue" */
    queue_t m_queue;
    
    inline size_t index(size_t ix, size_t iy) const
    { return ix + m_dimx * iy; }
    
    bool unqueue(size_t index);
    void requeue(size_t index);
    void update(size_t index);
    size_t pop();
  };
  
}

#endif // DTRANS_DISTANCE_TRANSFORM_HPP
