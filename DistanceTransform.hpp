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
     A DistanceTransform object computes the distance to some initial
     level set throughout a two-dimensional grid. You can use it
     e.g. for determining the distance from any location to the
     nearest obstacle, or to compute a navigation function that
     encodes cost-optimal paths from any location to some goal set.
   */  
  class DistanceTransform
  {
  public:
    /** A (very large) positive number that will be considered
	equivalent to infinity by the distance transform. */
    static double const infinity;
    
    /** A small positive number that will be considered equivalent to
	zero by the distance transform (only for speeds). */
    static double const epsilon;
    
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
    
    /** Scale of the cells.
	
	\return The length of one side of one cell.
    */
    inline double scale() const { return m_scale;}
    
    /** Set a given cell (given by its X and Y index) to a certain
	distance. Cells whose distance is set in this manner will be
	used to seed the distance transform computation. The
	propagation will not overwrite a cell's distance value if it
	has been set using this method.

	\return True if the cell's distance value has been set
	(i.e. the given coordinates lie within the grid).
    */
    bool setDist(size_t ix, size_t iy, double dist);
    
    /** Set the propagation speed for a cell (given by its X and Y
	index). These speeds are normalized to the range [0, 1], where
	zero speed means that the cell is an obstacle and unit speed
	means that it is fully in freespace. If you pass a speed
	smaller than zero or larger than one, it is ignored and this
	method returns false.
	
	\note You should set speeds before propagating the distance
	transform. Contrary to E*, this code does not support changing
	the speed on the fly.
	
	\return True if the given speed and indices were valid, false
	otherwise.
    */
    bool setSpeed(size_t ix, size_t iy, double speed);
    
    /** Get the distance of a cell.
	
	\return The distance value of a cell (given by its X and Y
	index). If the cell is invalid (i.e. it lies outside the
	grid), then DistanceTransform::infinity is returned. */
    double getDist(size_t ix, size_t iy) const;
    
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
    
    /** Reset all distance and gradient data and purge the queue, but
	keep the speed map. This is useful if you want to use the
	DistanceTransform as a global path planner and reuse a given
	instance for planning to a new goal.
    */
    void resetDist();
    
    /** Reset the speed map, setting all speeds to 1 (one). If you are
	using the DistanceTransform as global planner, this is the
	same as clearing all obstacles from the map. This method does
	not touch the distance or gradient, nor does it touch the
	queue. So it really only makes sense to call it right before
	or right after calling resetDist().
    */
    void resetSpeed();
    
    /** Debugging version of compute(). It does the same propagation,
	and writes information about what it is doing at each
	iteration. */
    void compute(double ceiling, FILE * dbg_fp, std::string const & dbg_prefix);
    
    /** Compute (or look up) the unscaled upwind gradient at a given cell. Unscaled
	means that it is not divided by the scale specified at
	DistanceTransform construction time, and upwind means that
	only neighbors lying below the value of the given cell are
	taken into account. This makes for faster and more robust
	computations.
	
	\note The resulting gradient (gx, gy) is placed in the
	corresponding references passed as method arguments. This
	method caches its results, so calling computeGradient()
	repeatedly for a given index does not repeat the
	computation. If the given (ix, iy) index is invalid, the
	returned gradient is zero.
	
	\return The number of neighboring cells taken into account for
	computing (gx, gx). If this number is zero, then the gradient
	is likewise (0, 0) because the cell is either inside an
	obstacle or a fixed cell that lies below its surrounding. Note
	that obstacle cells with at least one non-obstacle neighbor
	will generally result in a non-zero gradient.
    */
    size_t computeGradient(size_t ix, size_t iy,
			   double & gx, double & gy) const;
    
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
    void stat(/** minimum distance value, or +infinity in case no cell
		  lies below infinity */
	      double & minval,
	      /** maximum distance value, or -infinity in case no cell
		  lies below infinity */
	      double & maxval,
	      /** minimum queue key, or +infinity in case the queue is
		  empty */
	      double & minkey,
	      /** maximum queue key, or -infinity in case the queue is
		  empty */
	      double & maxkey) const;
    
    /** Write the current state of the grid and the queue key map in a
	(more or less) human-readable format to the given FILE
	stream. Each line gets prefixed with a user-defined string. */
    void dump(FILE * fp, std::string const & prefix) const;
    
    /** Write the speed map in a (more or less) human-readable format
	to the given FILE stream. Each line gets prefixed with a
	user-defined string. */
    void dumpSpeed(FILE * fp, std::string const & prefix) const;
    
    /** Write the current queue in a more or less human-readable
	format to the provided FILE stream, prefixing each line with
	the given user-defined string. */
    void dumpQueue(FILE * fp, std::string const & prefix) const;
    
    inline size_t const nCells() { return m_ncells; }
    inline std::vector<double> const & valueArray() { return m_value; }
    inline size_t index(size_t ix, size_t iy) const { return ix + m_dimx * iy; }
    
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
    std::vector<double> m_value; /**< distance map, negative values mean "fixed cell" */
    std::vector<double> m_lsm_radius; /**< scale/speed map, infinity means "obstacle" */
    std::vector<double> m_lsm_r2; /**< square thereof, to speed up computations */
    std::vector<double> m_key;	 /**< map of queue keys, a -1 means "not on queue" */
    queue_t m_queue;
    
    // gradient map and its neighbor count, to support caching
    mutable std::vector<double> m_gx;
    mutable std::vector<double> m_gy;
    mutable std::vector<int> m_gn;

    bool unqueue(size_t index);
    void requeue(size_t index);
    void update(size_t index);
    size_t pop();
  };
  
}

#endif // DTRANS_DISTANCE_TRANSFORM_HPP
