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


namespace dtrans {
  
  
  class DistanceTransform
  {
  public:
    static double const infinity;
    
    DistanceTransform(size_t dimx, size_t dimy, double scale);
    
    inline bool isValid(size_t ix, size_t iy) const
    { return (ix < m_dimx) && (iy < m_dimy); }
    
    inline size_t dimX() const { return m_dimx; }
    inline size_t dimY() const { return m_dimy; }
    
    bool set(size_t ix, size_t iy, double dist);
    double get(size_t ix, size_t iy) const;
    
    void compute();
    void compute(FILE * dbg_fp, std::string const & dbg_prefix);
    
    /** Perform one cell expansion. If the queue is empty, it does
	nothing.
	\return true if it computed something, false otherwise
	(i.e. when the queue was empty). */
    bool propagate();
    
    void dump(FILE * fp, std::string const & prefix) const;
    void dumpQueue(FILE * fp, std::string const & prefix) const;
    
  protected:
    typedef std::multimap<double, size_t> queue_t;
    typedef queue_t::iterator queue_it;
    
    size_t const m_dimx;
    size_t const m_dimy;
    size_t const m_ncells;
    size_t const m_toprow;
    size_t const m_rightcol;
    double const m_scale;
    double const m_scale2;
    std::vector<double> m_value; // <=0 means "fixed"
    std::vector<double> m_rhs;	 // <=0 means "fixed"
    std::vector<double> m_key;	 // -1 means "not on queue"
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
